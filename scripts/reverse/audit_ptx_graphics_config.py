#!/usr/bin/env python3
"""Audit canonical PTX graphics profile storage, preset data and pool callers."""
import argparse
import hashlib
import json
from pathlib import Path

from canonical_image import CANONICAL, CanonicalImage
from verify_ptx_payload import check

CONFIG_SLOT = 0x140D6D300
CONFIG_STORAGE = 0x140D6D310
PRESET_TABLE = 0x1405D1B08
PRESET_DEFAULT = 0x1405D1AE8
WINDOWS = {
    0x140025BF0: 0x140025C2F,
    0x140331710: 0x1403317CF,
    0x140331910: 0x14033196C,
    0x140331D90: 0x140331DE2,
    0x140332F00: 0x1403332D2,
    0x140337BF0: 0x140337CB2,
    0x140337CD0: 0x140337DE4,
}


def instruction(image, site, mnemonic, op=None):
    ins = image.instruction(site)
    check(ins.mnemonic == mnemonic, f'{site:#x} is {mnemonic}')
    if op is not None:
        check(ins.op_str == op, f'{site:#x} operand {op}')
    return ins


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

    storage_lea = instruction(image, 0x140025BF4, 'lea')
    storage = storage_lea.address + storage_lea.size + storage_lea.operands[1].mem.disp
    check(storage == CONFIG_STORAGE, 'static profile backing address')
    initializer = instruction(image, 0x140025C07, 'call')
    check(initializer.operands[0].imm == 0x14032D3C0,
          'static backing is zeroed through the dword-count clear helper')
    count = instruction(image, 0x140025BFB, 'mov')
    check(count.operands[1].imm == 0x2C, 'profile clear count is 44 dwords')

    profile_bytes = image.raw(PRESET_DEFAULT, 16)
    check(profile_bytes.hex() == '010002000100020000020001e0000200',
          'canonical default profile bytes')
    table_entry = image.u64(PRESET_TABLE + 8)
    check(table_entry == PRESET_DEFAULT, 'default profile is table index 1')

    instruction(image, 0x140337C49, 'lea', 'ecx, [rdx + 1]')
    instruction(image, 0x140337C47, 'xor', 'edx, edx')
    profile_call = instruction(image, 0x140337C4C, 'call')
    check(profile_call.operands[0].imm == 0x140337CD0,
          'graphics startup applies preset 1 in standard mode')
    update_call = instruction(image, 0x140337C5A, 'call')
    check(update_call.operands[0].imm == 0x140332F00,
          'graphics startup derives active profile fields')
    instruction(image, 0x140337C58, 'mov', 'cl, 1')

    constant = instruction(image, 0x140333292, 'mov')
    check(constant.operands[1].imm == 0x1900, 'profile bound constant is 0x1900')
    instruction(image, 0x1403332B8, 'mov', 'word ptr [rax + 0x4c], cx')
    instruction(image, 0x1403332C3, 'add', 'cx, word ptr [rax + 0x4a]')
    instruction(image, 0x1403332C7, 'mov', 'word ptr [rax + 0x4e], cx')
    config_ref = instruction(image, 0x14033192D, 'mov')
    config_target = config_ref.address + config_ref.size + config_ref.operands[1].mem.disp
    check(config_target == CONFIG_SLOT,
          'pool initializer reads the profile object through the global slot')

    init_sites = (0x14004F191, 0x14004F438, 0x140238376, 0x140238498)
    configure_sites = (0x14004F1A2, 0x14004F449, 0x140238387, 0x1402384A9,
                       0x140238C8B, 0x14023C71F, 0x1402409DD, 0x140240A77,
                       0x140242C71, 0x140243166)
    calls = []
    for site, target in [*((s, 0x140331910) for s in init_sites),
                         *((s, 0x140331D90) for s in configure_sites)]:
        ins = instruction(image, site, 'call')
        check(ins.operands[0].imm == target, f'pool lifecycle call {site:#x}')
        calls.append({'site': hex(site), 'target': hex(target), 'bytes': ins.bytes.hex()})

    return {
        'sha256': CANONICAL,
        'profile_global': {
            'pointer_slot': hex(CONFIG_SLOT), 'static_storage': hex(CONFIG_STORAGE),
            'zeroed_bytes': 0xB0, 'known_bytes': 'selected fields through +0x4e',
            'known_word_offsets': {'profile_x': '0x20', 'profile_y': '0x22',
                                   'profile_width': '0x24', 'derived_height': '0x4a',
                                   'pool_base': '0x4c', 'pool_limit': '0x4e'},
        },
        'default_profile': {
            'table': hex(PRESET_TABLE), 'index': 1, 'record': hex(PRESET_DEFAULT),
            'bytes': profile_bytes.hex(),
            'fields': {'byte_00': 1, 'byte_02': 2, 'byte_04': 1, 'byte_06': 2,
                       'word_08': 512, 'word_0a': 256, 'word_0c': 224, 'byte_0e': 2},
        },
        'derived_profile_bounds': {
            'pool_base_word_4c': '0x1900',
            'pool_limit_word_4e': 'wrap16(0x1900 + word_4a)',
            'word_4a_formula': 'wrap16(factor*word_40 - word_46 - word_3c + 0x4000)',
            'factor': '0xfffe if byte_06 != 0 else 0xffff',
            'setup_caller': '0x140337c20 -> 0x140337c4c -> 0x140337cd0 -> 0x140337c5a -> 0x140332f00',
        },
        'pool_lifecycle_calls': calls,
        'relative_runtime_order': 'Static direct calls are recovered; scene-wide initialization/shutdown reachability is not traced.',
        'status': 'STATIC_EXE_CONFIRMED_WITH_LIVE_DEFAULT_PRESET',
    }


def run(exe, out):
    image = CanonicalImage(exe)
    evidence = build_evidence(image)
    out.mkdir(parents=True, exist_ok=True)
    (out / 'evidence.json').write_text(json.dumps(evidence, indent=2) + '\n')
    return {'status': evidence['status'], 'sha256': CANONICAL,
            'preset': evidence['default_profile']['fields'],
            'pool_lifecycle_calls': len(evidence['pool_lifecycle_calls'])}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exe', type=Path)
    parser.add_argument('out', type=Path)
    args = parser.parse_args()
    print(json.dumps(run(args.exe, args.out), indent=2))
