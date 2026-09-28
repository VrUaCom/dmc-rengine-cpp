#!/usr/bin/env python3
"""Differentially verify callback-record recycling and canonical arena setup."""
import argparse
import ctypes
import hashlib
import json
import random
import struct
import subprocess
import tempfile
from pathlib import Path

import capstone
import unicorn
from unicorn import x86_const as R

from canonical_image import CANONICAL, CanonicalImage
from verify_crt_lifecycle import STOP, emulator
from verify_ptx_payload import check

CALLBACK_POOL = 0x62000000
QUEUE_HEAD = 0x61000000
CALLBACK_STUB = 0x63000000
ARENA_STATE = 0x140CA8910
ARENA_BACKING = 0x64000000
CALLBACK_FREE_HEAD = 0x140CF3240
CALLBACK_COUNT = 0x1405D1130
CALLBACK_COUNT_BYTES = 0x1000

WINDOWS = {
    0x140329240: 0x140329291,
    0x1403292A0: 0x1403292F9,
    0x140329300: 0x140329379,
    0x140337780: 0x1403378CF,
    0x140337E90: 0x140337EB5,
}


def native_callback_step(image, storage, free_head, queue_head, operation,
                         function, callback_context, payload):
    u = emulator(image)
    u.mem_map(QUEUE_HEAD, 0x1000)
    u.mem_map(CALLBACK_POOL, 0x10000)
    u.mem_map(CALLBACK_STUB, 0x1000)
    u.mem_write(CALLBACK_POOL, bytes(storage))
    u.mem_write(QUEUE_HEAD, struct.pack('<Q', queue_head))
    u.mem_write(CALLBACK_FREE_HEAD, struct.pack('<Q', free_head))
    u.mem_write(CALLBACK_STUB, b'\xc3')
    check(struct.unpack('<I', image.raw(CALLBACK_COUNT, 4))[0] == 32,
          'canonical callback pool count is 32')
    next_allocation = CALLBACK_POOL
    invocations = []

    def return_from_hook(uc, value):
        rsp = uc.reg_read(R.UC_X86_REG_RSP)
        target = struct.unpack('<Q', uc.mem_read(rsp, 8))[0]
        uc.reg_write(R.UC_X86_REG_RAX, value)
        uc.reg_write(R.UC_X86_REG_RSP, rsp + 8)
        uc.reg_write(R.UC_X86_REG_RIP, target)

    def hook(uc, address, _size, _):
        nonlocal next_allocation
        if address == 0x140345510:
            result = next_allocation
            next_allocation += 0x20
            return_from_hook(uc, result)
        elif address == CALLBACK_STUB:
            invocations.append((uc.reg_read(R.UC_X86_REG_RCX),
                                uc.reg_read(R.UC_X86_REG_RDX)))

    u.hook_add(unicorn.UC_HOOK_CODE, hook)
    if operation == 0:
        entry = 0x140329240
        for reg, value in ((R.UC_X86_REG_RCX, QUEUE_HEAD),
                           (R.UC_X86_REG_RDX, function),
                           (R.UC_X86_REG_R8, callback_context),
                           (R.UC_X86_REG_R9, payload)):
            u.reg_write(reg, value)
    else:
        entry = 0x1403292A0
        u.reg_write(R.UC_X86_REG_RCX, QUEUE_HEAD)
    u.emu_start(entry, STOP, count=100000)
    check(u.reg_read(R.UC_X86_REG_RIP) == STOP, 'callback function returned')
    return (bytes(u.mem_read(CALLBACK_POOL, len(storage))),
            struct.unpack('<Q', u.mem_read(CALLBACK_FREE_HEAD, 8))[0],
            struct.unpack('<Q', u.mem_read(QUEUE_HEAD, 8))[0], invocations)


def native_arena(image, initial_state, initial_backing, block_bytes, backing_bytes, shift):
    u = emulator(image)
    u.mem_map(ARENA_BACKING, 0x10000)
    u.mem_write(ARENA_STATE, initial_state)
    u.mem_write(ARENA_BACKING, initial_backing)
    rsp = 0x70008008
    u.reg_write(R.UC_X86_REG_RSP, rsp)
    u.mem_write(rsp, struct.pack('<Q', STOP))
    u.mem_write(rsp + 0x28, struct.pack('<I', shift))
    u.reg_write(R.UC_X86_REG_RCX, ARENA_STATE)
    u.reg_write(R.UC_X86_REG_RDX, ARENA_BACKING)
    u.reg_write(R.UC_X86_REG_R8, block_bytes)
    u.reg_write(R.UC_X86_REG_R9, backing_bytes)
    u.emu_start(0x140337780, STOP, count=200000)
    check(u.reg_read(R.UC_X86_REG_RIP) == STOP, 'arena initializer returned')
    return (bytes(u.mem_read(ARENA_STATE, 0x28)),
            bytes(u.mem_read(ARENA_BACKING, len(initial_backing))),
            u.reg_read(R.UC_X86_REG_RAX) & 0xFF)


def evidence(image):
    functions = []
    branches = {}
    for start, end in WINDOWS.items():
        code = image.raw(start, end - start)
        instructions = list(image.md.disasm(code, start))
        check(instructions and instructions[-1].address + instructions[-1].size == end,
              f'complete instruction window {start:#x}')
        for instruction in instructions:
            if instruction.group(capstone.CS_GRP_JUMP) and instruction.mnemonic != 'jmp':
                branches[instruction.address] = (instruction.address + instruction.size,
                                                 instruction.operands[0].imm)
        functions.append({
            'root': hex(start), 'end': hex(end),
            'file_offset': hex(image.offset(start, end - start)),
            'code_sha256': hashlib.sha256(code).hexdigest(),
            'pdata_fragments': [[hex(value) for value in row]
                                for row in image.ranges if row[0] < end and start < row[1]],
            'instructions': [{'address': hex(i.address), 'bytes': i.bytes.hex(),
                              'mnemonic': i.mnemonic, 'operands': i.op_str}
                             for i in instructions],
        })
    callback_count = struct.unpack('<I', image.raw(CALLBACK_COUNT, 4))[0]
    check(callback_count == 32, 'read callback preallocation count from canonical image')
    return {
        'sha256': CANONICAL,
        'functions': functions,
        'callback_record': {'stride': 0x20, 'function': 0, 'context': 8,
                            'payload': 0x10, 'next': 0x18,
                            'global_free_head': hex(CALLBACK_FREE_HEAD),
                            'preallocation_count_address': hex(CALLBACK_COUNT),
                            'preallocation_count': callback_count},
        'arena_table': {'base': hex(ARENA_STATE), 'stride': 0x28,
                        'initializer': '0x140337780'},
        'status': 'EXE_CONFIRMED',
    }, branches


def run(exe, repo, out):
    image = CanonicalImage(exe)
    facts, branches = evidence(image)
    cases = []
    rng = random.Random(20260923)
    with tempfile.TemporaryDirectory(prefix='ptx-callback-arena-') as temp:
        library = Path(temp) / 'callback_arena.so'
        sources = ['src/reverse/runtime_block_allocator.cpp',
                   'src/reverse/deferred_callback_queue.cpp',
                   'tests/reverse/ptx_callback_arena_bridge.cpp']
        compiler = subprocess.check_output(['g++', '--version'], text=True).splitlines()[0]
        subprocess.run(['g++', '-std=c++20', '-O2', '-shared', '-fPIC', '-Wall', '-Wextra',
                        '-Wconversion', '-Werror', '-I' + str(repo / 'include'),
                        *[str(repo / path) for path in sources], '-o', str(library)], check=True)
        lib = ctypes.CDLL(str(library))
        arena_fn = lib.initialize_runtime_arena_step
        arena_fn.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t,
                             ctypes.c_uint64, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32]
        arena_fn.restype = ctypes.c_int
        callback_fn = lib.deferred_callback_queue_step
        callback_fn.argtypes = [ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t,
                                ctypes.c_uint64, ctypes.POINTER(ctypes.c_uint64),
                                ctypes.POINTER(ctypes.c_uint64), ctypes.c_uint64,
                                ctypes.c_uint64, ctypes.c_uint64, ctypes.c_uint64,
                                ctypes.POINTER(ctypes.c_uint64), ctypes.c_size_t,
                                ctypes.POINTER(ctypes.c_size_t)]
        callback_fn.restype = ctypes.c_uint64

        storage = bytearray(0x1000)
        free_head = 0
        queue_head = 0
        for index, context, payload in ((0, 0x111, 0xaaa), (1, 0x222, 0xbbb),
                                        (2, 0x333, 0xccc)):
            c_storage = ctypes.create_string_buffer(bytes(storage), len(storage))
            c_free = ctypes.c_uint64(free_head)
            c_queue = ctypes.c_uint64(queue_head)
            events = (ctypes.c_uint64 * 24)()
            event_count = ctypes.c_size_t()
            node = callback_fn(ctypes.cast(c_storage, ctypes.POINTER(ctypes.c_ubyte)),
                len(storage), CALLBACK_POOL, ctypes.byref(c_free), ctypes.byref(c_queue), 0,
                CALLBACK_STUB, context, payload, events, 8, ctypes.byref(event_count))
            cpp = (c_storage.raw, c_free.value, c_queue.value, node,
                   [tuple(events[3*i:3*i+3]) for i in range(event_count.value)])
            native = native_callback_step(image, storage, free_head, queue_head, 0,
                                          CALLBACK_STUB, context, payload)
            check(cpp[0] == native[0] and cpp[1] == native[1] and cpp[2] == native[2],
                  f'callback enqueue state {index}')
            check(node == c_queue.value, 'callback node address')
            cases.append({'name': f'enqueue-{index}', 'status': 'PASS',
                          'node': hex(node), 'free_head': hex(c_free.value),
                          'queue_head': hex(c_queue.value)})
            storage, free_head, queue_head = bytearray(c_storage.raw), c_free.value, c_queue.value

        c_storage = ctypes.create_string_buffer(bytes(storage), len(storage))
        c_free = ctypes.c_uint64(free_head)
        c_queue = ctypes.c_uint64(queue_head)
        events = (ctypes.c_uint64 * 24)()
        event_count = ctypes.c_size_t()
        callback_fn(ctypes.cast(c_storage, ctypes.POINTER(ctypes.c_ubyte)), len(storage),
            CALLBACK_POOL, ctypes.byref(c_free), ctypes.byref(c_queue), 1, 0, 0, 0,
            events, 8, ctypes.byref(event_count))
        cpp_events = [tuple(events[3*i:3*i+3]) for i in range(event_count.value)]
        cpp_state = (c_storage.raw, c_free.value, c_queue.value)
        native_state = native_callback_step(image, storage, free_head, queue_head, 1, 0, 0, 0)
        native_events = [(CALLBACK_STUB, callback_context, payload)
                         for callback_context, payload in native_state[3]]
        check(cpp_state == native_state[:3], 'callback drain state and recycled list')
        check(cpp_events == native_events, 'callback dispatch order and arguments')
        cases.append({'name': 'drain-three-records', 'status': 'PASS',
                      'callback_order': [hex(row[1]) for row in cpp_events],
                      'free_head': hex(c_free.value)})

        arena_cases = [(0x200, 0x1000, 6), (0x201, 0x1000, 6),
                       (0x800, 0x1000, 6), (0x200, 0x1000, 4),
                       (0x800, 0x83F, 6), (0x200, 0x0F, 6)]
        for index, (block_bytes, backing_bytes, shift) in enumerate(arena_cases):
            initial_state = rng.randbytes(0x28)
            initial_backing = rng.randbytes(0x1000)
            c_state = ctypes.create_string_buffer(initial_state, 0x28)
            c_backing = ctypes.create_string_buffer(initial_backing, len(initial_backing))
            c_result = arena_fn(c_state, ctypes.cast(c_backing, ctypes.POINTER(ctypes.c_ubyte)),
                len(initial_backing), ARENA_BACKING, block_bytes, backing_bytes, shift)
            native = native_arena(image, initial_state, initial_backing,
                                  block_bytes, backing_bytes, shift)
            check(c_result == native[2], f'arena initializer return {index}')
            check(c_state.raw == native[0], f'arena initializer metadata {index}')
            check(c_backing.raw == native[1], f'arena initializer backing bytes {index}')
            cases.append({'name': f'arena-init-{index}', 'status': 'PASS',
                          'block_bytes': block_bytes, 'backing_bytes': backing_bytes,
                          'alignment_shift': shift, 'return': c_result})

    out.mkdir(parents=True, exist_ok=True)
    result = {
        'status': 'PASS', 'sha256': CANONICAL, 'cases': len(cases), 'case_results': cases,
        'compiler': compiler, 'conditional_sites': len(branches),
        'limitations': [
            'Callback target bodies are intercepted at a synthetic RET stub; their own semantics are outside this pass.',
            'Operator-new allocation failure and callback reentrancy/concurrency are not modeled.',
            'Global backing ownership is checked separately by verify_ptx_temporary_backing.py; cross-owner runtime ordering is not established.',
            'Arena overlap/invalid-pointer CPU-fault behavior is not modeled; the fixtures use disjoint bounded spans.',
        ],
    }
    (out / 'evidence.json').write_text(json.dumps(facts, indent=2) + '\n')
    (out / 'verification.json').write_text(json.dumps(result, indent=2) + '\n')
    return {key: value for key, value in result.items() if key != 'case_results'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('exe', 'repo', 'out'):
        parser.add_argument(name, type=Path)
    args = parser.parse_args()
    print(json.dumps(run(args.exe, args.repo.resolve(), args.out), indent=2))
