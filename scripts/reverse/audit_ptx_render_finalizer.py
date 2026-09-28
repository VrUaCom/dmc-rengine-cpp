#!/usr/bin/env python3
"""Audit the PTX render-descriptor finalizer and its visible publication path."""
import argparse
import hashlib
import json
from pathlib import Path

from canonical_image import CANONICAL, CanonicalImage
from verify_ptx_payload import check

WINDOWS = {
    0x140331A80: 0x140331BCD,
    0x140336B68: 0x140336B75,
    0x140336CA8: 0x140336CB0,
    0x14032D6B0: 0x14032D846,
    0x140033370: 0x140033385,
}


def build_evidence(image):
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

    check(image.instruction(0x140331A98).op_str == 'eax, dword ptr [rcx + 0x200]',
          'finalizer first count field')
    check(image.instruction(0x140331A8F).op_str == 'eax, dword ptr [rcx + 0x204]',
          'finalizer second count field')
    call_builder = image.instruction(0x140331B71)
    call_registry = image.instruction(0x140331B7B)
    check(call_builder.mnemonic == 'call' and call_builder.operands[0].imm == 0x14032D6B0,
          'finalizer builds a render descriptor')
    check(call_registry.mnemonic == 'call' and call_registry.operands[0].imm == 0x140033370,
          'finalizer publishes resource mapping')
    direct_callers = []
    for site, target in ((0x140336B6B, 0x140331A80),
                         (0x140336CAB, 0x140331A80)):
        ins = image.instruction(site)
        check(ins.mnemonic == 'call' and ins.operands[0].imm == target,
              f'materializer invokes render finalizer at {site:#x}')
        direct_callers.append({'site': hex(site), 'target': hex(target),
                               'bytes': ins.bytes.hex()})

    registry_lea = image.instruction(0x140033372)
    registry = registry_lea.address + registry_lea.size + registry_lea.operands[1].mem.disp
    check(image.instruction(0x140033379).mnemonic == 'shr',
          'registry indexes entries by signed identifier divided by four')
    check(image.instruction(0x14003337D).op_str == 'qword ptr [r8 + rax*8], rdx',
          'registry stores resource pointer')
    check(image.instruction(0x140033381).op_str == 'dword ptr [rdx + 0x50], ecx',
          'resource record stores its identifier')

    helper = image.md.disasm(image.raw(0x14032D6B0, 0x14032D846 - 0x14032D6B0),
                             0x14032D6B0)
    calls = [{'site': hex(i.address), 'target': hex(i.operands[0].imm)}
             for i in helper if i.mnemonic == 'call' and i.operands and i.operands[0].type == 2]
    allocation_clear = image.instruction(0x14032D6D6)
    check(allocation_clear.mnemonic == 'call' and allocation_clear.operands[0].imm == 0x14032D3C0,
          'descriptor builder zeroes its 0xA0-byte descriptor')

    return {
        'sha256': CANONICAL,
        'finalizer': {
            'address': '0x140331a80',
            'record_count': 'dword[context+0x200] * dword[context+0x204]',
            'record_table': 'qword pointers at context + index*8',
            'per_record_reads': {'word_04': 'identifier metadata', 'word_06': 'signed registry key',
                                 'word_08': 'descriptor input', 'word_0a': 'descriptor input',
                                 'word_0c': 'descriptor input', 'qword_10': 'resource pointer'},
            'per_record_writes': {'dword_3c': 0, 'dword_40': 1,
                                  'qword_30': 'built descriptor pointer', 'dword_38': 1},
            'direct_callers': direct_callers,
        },
        'descriptor_builder': {
            'address': '0x14032d6b0', 'zeroed_bytes': 0xA0,
            'direct_internal_calls': calls,
        },
        'resource_registry': {
            'writer': '0x140033370', 'base': hex(registry),
            'index_expression': 'identifier >> 2',
            'entry': 'qword registry[index] = resource; dword[resource+0x50] = identifier',
            'draw_or_shutdown_reader': 'NOT_ESTABLISHED_BY_THIS_STATIC_PASS',
        },
        'status': 'STATIC_EXE_CONFIRMED_BOUNDED_FINALIZER_PATH',
    }


def run(exe, out):
    image = CanonicalImage(exe)
    evidence = build_evidence(image)
    out.mkdir(parents=True, exist_ok=True)
    (out / 'evidence.json').write_text(json.dumps(evidence, indent=2) + '\n')
    return {'status': evidence['status'], 'sha256': CANONICAL,
            'direct_callers': len(evidence['finalizer']['direct_callers']),
            'registry': evidence['resource_registry']['base']}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exe', type=Path)
    parser.add_argument('out', type=Path)
    args = parser.parse_args()
    print(json.dumps(run(args.exe, args.out), indent=2))
