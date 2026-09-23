#!/usr/bin/env python3
"""Differentially verify the PTX temporary backing arena and scoped allocator."""
import argparse
import ctypes
import hashlib
import json
import random
import struct
import subprocess
import tempfile
from pathlib import Path

import unicorn
from unicorn import x86_const as R

from canonical_image import CANONICAL, CanonicalImage
from verify_crt_lifecycle import STOP, STACK, emulator
from verify_ptx_payload import check

ARENA = 0x65000000
LEASE_A = 0x65100000
LEASE_B = 0x65101000
BASE = 0x64000000
WINDOWS = {
    0x140337920: 0x1403379AF,
    0x1403379B0: 0x1403379D8,
    0x1403379E0: 0x140337A18,
    0x140337A30: 0x140337A78,
    0x140337E90: 0x140337EB5,
    0x1402C6010: 0x1402C60D3,
}


def native(image, entry, arena_state, lease_a, lease_b=b'', args=()):
    u = emulator(image)
    u.mem_map(ARENA, 0x1000)
    u.mem_map(LEASE_A, 0x1000)
    u.mem_map(LEASE_B, 0x1000)
    u.mem_write(ARENA, arena_state)
    u.mem_write(LEASE_A, lease_a)
    if lease_b:
        u.mem_write(LEASE_B, lease_b)
    rsp = STACK + 0x8008
    u.reg_write(R.UC_X86_REG_RSP, rsp)
    u.mem_write(rsp, struct.pack('<Q', STOP))
    regs = (R.UC_X86_REG_RCX, R.UC_X86_REG_RDX, R.UC_X86_REG_R8, R.UC_X86_REG_R9)
    for reg, value in zip(regs, args):
        u.reg_write(reg, value)
    u.emu_start(entry, STOP, count=100000)
    check(u.reg_read(R.UC_X86_REG_RIP) == STOP, f'{entry:#x} returned')
    return (u.reg_read(R.UC_X86_REG_RAX), bytes(u.mem_read(ARENA, 0x28)),
            bytes(u.mem_read(LEASE_A, 0x18)), bytes(u.mem_read(LEASE_B, 0x18)))


def evidence(image):
    functions = []
    for start, end in WINDOWS.items():
        code = image.raw(start, end - start)
        ins = list(image.md.disasm(code, start))
        check(ins and ins[-1].address + ins[-1].size == end,
              f'complete instruction window {start:#x}')
        functions.append({
            'root': hex(start), 'end': hex(end),
            'file_offset': hex(image.offset(start, end - start)),
            'code_sha256': hashlib.sha256(code).hexdigest(),
            'instructions': [{'address': hex(i.address), 'bytes': i.bytes.hex(),
                              'mnemonic': i.mnemonic, 'operands': i.op_str} for i in ins],
        })
    callsite = image.instruction(0x14004F49F)
    check(callsite.mnemonic == 'call' and callsite.operands[0].imm == 0x1402C6010,
          'constructor path calls one-time arena setup')
    context_calls = []
    for site, target in ((0x1402C0594, 0x140331180),
                         (0x1402C035B, 0x140331460),
                         (0x140245D67, 0x140331460)):
        instruction = image.instruction(site)
        check(instruction.mnemonic == 'call' and instruction.operands[0].imm == target,
              f'context lifecycle direct call {site:#x}')
        context_calls.append({'site': hex(site), 'target': hex(target),
                              'bytes': instruction.bytes.hex()})
    embedded_receiver = image.instruction(0x140245D60)
    check(embedded_receiver.mnemonic == 'lea' and embedded_receiver.op_str == 'rcx, [rbx + 0x5e0]',
          'embedded context receiver is this+0x5e0')
    setup_calls = []
    for site, target in ((0x1402C6022, 0x1403379B0),
                         (0x1402C603C, 0x140337920),
                         (0x1402C605F, 0x140337780),
                         (0x1402C6070, 0x140337920),
                         (0x1402C6093, 0x140337780),
                         (0x1402C60A4, 0x140337920),
                         (0x1402C60C7, 0x140337780)):
        instruction = image.instruction(site)
        check(instruction.mnemonic == 'call' and instruction.operands[0].imm == target,
              f'one-time setup call {site:#x}')
        setup_calls.append({'site': hex(site), 'target': hex(target),
                            'bytes': instruction.bytes.hex()})
    return {
        'sha256': CANONICAL,
        'functions': functions,
        'one_time_setup': {
            'function': '0x1402c6010', 'direct_caller_site': '0x14004f49f',
            'allocation_context': '0x140ca8988', 'backing_arena': '0x140ca89a0',
            'lease_acquire_site': '0x1402c6022',
            'calls': setup_calls,
            'allocations': [
                {'site': '0x1402c603c', 'bytes': '0x04000000', 'state': '0x140ca8910', 'block_bytes': '0x800', 'shift': 6},
                {'site': '0x1402c6070', 'bytes': '0x00500000', 'state': '0x140ca8938', 'block_bytes': '0x400', 'shift': 4},
                {'site': '0x1402c60a4', 'bytes': '0x00400000', 'state': '0x140ca8960', 'block_bytes': '0x200', 'shift': 6},
            ],
        },
        'context_ordering': {
            'global_context': {'initialize_site': '0x1402c0594', 'address': '0x140cf1030',
                               'status': 'static callsite evidence'},
            'embedded_context': {'destroy_site': '0x140245d67', 'offset': 'this+0x5e0',
                                 'guard': 'this+0x638', 'status': 'static cleanup evidence'},
            'direct_calls': context_calls,
            'ordering_conclusion': 'No common caller or runtime trace establishes relative order between these paths.',
        },
        'status': 'EXE_CONFIRMED',
    }


def run(exe, repo, out):
    image = CanonicalImage(exe)
    facts = evidence(image)
    rng = random.Random(20260923)
    cases = []
    with tempfile.TemporaryDirectory(prefix='ptx-temporary-backing-') as temp:
        library = Path(temp) / 'temporary_backing.so'
        compiler = subprocess.check_output(['g++', '--version'], text=True).splitlines()[0]
        subprocess.run(['g++', '-std=c++20', '-O2', '-shared', '-fPIC', '-Wall', '-Wextra',
                        '-Wconversion', '-Werror', '-I' + str(repo / 'include'),
                        str(repo / 'src/reverse/temporary_backing_allocator.cpp'),
                        str(repo / 'tests/reverse/ptx_temporary_backing_bridge.cpp'),
                        '-o', str(library)], check=True)
        lib = ctypes.CDLL(str(library))
        init = lib.temporary_backing_init_step
        init.argtypes = [ctypes.c_void_p, ctypes.c_uint64, ctypes.c_uint32, ctypes.c_uint32]
        init.restype = None
        acquire = lib.temporary_backing_acquire_step
        acquire.argtypes = [ctypes.c_void_p, ctypes.c_uint64, ctypes.c_void_p]
        acquire.restype = ctypes.c_uint8
        alloc = lib.temporary_backing_alloc_step
        alloc.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint32]
        alloc.restype = ctypes.c_uint64
        release = lib.temporary_backing_release_step
        release.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
        release.restype = ctypes.c_uint8

        for shift, size, base in ((6, 0x1000, BASE), (4, 0x400, BASE + 3),
                                  (0, 0x100, BASE + 0x800)):
            arena = ctypes.create_string_buffer(rng.randbytes(0x28), 0x28)
            native_init = native(image, 0x140337A30, bytes(0x28), bytes(0x18),
                                 args=(ARENA, base, size, shift))
            init(arena, base, size, shift)
            check(arena.raw == native_init[1], f'arena init shift={shift}')
            cases.append({'name': f'arena-init-shift-{shift}', 'status': 'PASS',
                          'base': hex(base), 'bytes': size, 'alignment_shift': shift})

            owner = ctypes.create_string_buffer(bytes(0x18), 0x18)
            got = native(image, 0x1403379B0, arena.raw, bytes(0x18),
                         args=(LEASE_A, ARENA))
            c_result = acquire(owner, ARENA, arena)
            check((got[0] & 0xff) == c_result == 1, 'lease acquire result')
            check(got[1] == arena.raw and got[2] == owner.raw, 'lease acquire bytes')
            cases.append({'name': f'lease-acquire-shift-{shift}', 'status': 'PASS',
                          'lease_depth': 1})

            # Each native call starts from the state produced by the previous one.
            for n, request in enumerate((1, (1 << shift), (1 << shift) + 1, size, 0xFFFFFFFF)):
                state_before, lease_before = arena.raw, owner.raw
                native_alloc = native(image, 0x140337920, state_before, lease_before,
                                      args=(LEASE_A, request))
                c_address = alloc(owner, arena, request)
                check(native_alloc[0] == c_address, f'allocation return case {shift}/{n}: {native_alloc[0]:#x}/{c_address:#x}')
                check(native_alloc[1] == arena.raw, f'allocation arena bytes {shift}/{n}')
                cases.append({'name': f'alloc-shift-{shift}-{n}', 'status': 'PASS',
                              'requested': request, 'returned': hex(c_address)})

            native_release = native(image, 0x1403379E0, arena.raw, owner.raw,
                                    args=(LEASE_A,))
            c_released = release(owner, arena)
            check((native_release[0] & 0xff) == c_released == 1, 'lease release result')
            check(native_release[1] == arena.raw and native_release[2] == owner.raw,
                  'lease release bytes and cursor rollback')
            cases.append({'name': f'lease-release-shift-{shift}', 'status': 'PASS',
                          'cursor_restored': True})

        # Nested leases behave as a stack: only the latest lease may release.
        arena = ctypes.create_string_buffer(bytes(0x28), 0x28)
        init(arena, BASE, 0x1000, 6)
        native_init = native(image, 0x140337A30, bytes(0x28), bytes(0x18),
                             args=(ARENA, BASE, 0x1000, 6))
        check(arena.raw == native_init[1], 'nested fixture arena init')
        owner_a = ctypes.create_string_buffer(bytes(0x18), 0x18)
        owner_b = ctypes.create_string_buffer(bytes(0x18), 0x18)
        native_a = native(image, 0x1403379B0, arena.raw, bytes(0x18),
                          args=(LEASE_A, ARENA))
        c_outer_acquire = acquire(owner_a, ARENA, arena)
        check((native_a[0] & 0xff) == c_outer_acquire == 1, 'outer lease acquire')
        check(native_a[2] == owner_a.raw, f'outer lease state: {native_a[2].hex()} / {owner_a.raw.hex()}')
        alloc(owner_a, arena, 0x81)
        native_nested = native(image, 0x1403379B0, arena.raw, owner_a.raw,
                               args=(LEASE_B, ARENA))
        acquired_b = acquire(owner_b, ARENA, arena)
        check((native_nested[0] & 0xff) == acquired_b == 1, 'nested lease acquire result')
        check(native_nested[1] == arena.raw and native_nested[3] == owner_b.raw,
              'nested lease acquire state')
        failed_release = native(image, 0x1403379E0, arena.raw, owner_a.raw,
                                owner_b.raw, args=(LEASE_A,))
        c_failed = release(owner_a, arena)
        check((failed_release[0] & 0xff) == c_failed == 0, 'outer lease cannot release before inner')
        check(failed_release[1] == arena.raw and failed_release[2] == owner_a.raw,
              'failed outer release preserves state')
        inner_release = native(image, 0x1403379E0, arena.raw, owner_a.raw,
                               owner_b.raw, args=(LEASE_B,))
        c_inner = release(owner_b, arena)
        check((inner_release[0] & 0xff) == c_inner == 1, 'inner lease release')
        check(inner_release[1] == arena.raw and inner_release[3] == owner_b.raw,
              'inner release restores its cursor snapshot')
        outer_release = native(image, 0x1403379E0, arena.raw, owner_a.raw,
                               owner_b.raw, args=(LEASE_A,))
        c_outer = release(owner_a, arena)
        check((outer_release[0] & 0xff) == c_outer == 1, 'outer lease release')
        check(outer_release[1] == arena.raw and outer_release[2] == owner_a.raw,
              'outer release restores its cursor snapshot')
        cases.append({'name': 'nested-leases-lifo', 'status': 'PASS',
                      'outer_release_blocked_while_nested': True,
                      'inner_release_restores_snapshot': True})

    out.mkdir(parents=True, exist_ok=True)
    result = {
        'status': 'PASS', 'sha256': CANONICAL, 'cases': len(cases),
        'case_results': cases, 'compiler': compiler,
        'limitations': [
            'Host spans are not dereferenced by the allocator fixture; returned virtual addresses and descriptor bytes are compared.',
            'Concurrent calls, reentrant acquisition and non-LIFO lease recovery are outside the model.',
            'Constructor reachability is established by a direct callsite; process runtime ordering is not traced.',
            'Global and embedded PTX context paths are each statically identified, but their relative lifetime order remains unproven.',
        ],
    }
    (out / 'evidence.json').write_text(json.dumps(facts, indent=2) + '\n')
    (out / 'verification.json').write_text(json.dumps(result, indent=2) + '\n')
    return {'status': result['status'], 'sha256': result['sha256'], 'cases': result['cases'],
            'compiler': compiler}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('exe', 'repo', 'out'):
        parser.add_argument(name, type=Path)
    args = parser.parse_args()
    print(json.dumps(run(args.exe, args.repo.resolve(), args.out), indent=2))
