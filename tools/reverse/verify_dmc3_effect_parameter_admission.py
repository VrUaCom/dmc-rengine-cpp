"""v22 canonical EXE fixtures: defaults, raw parameter admission and cache shape.

Usage: verify_dmc3_effect_parameter_admission.py CANONICAL_EXE OUTPUT_JSON
Requires adjacent verify_dmc3_render_producer_boundaries.py (v21).
No retail-game or GPU execution. All additional external stubs are recorded.
"""
from pathlib import Path
import importlib.util
import json
import struct
import sys
from unicorn import UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn.x86_const import *

helper = Path(__file__).with_name('verify_dmc3_render_producer_boundaries.py')
if not helper.exists():
    helper = Path(__file__).with_name('verify_machine_code.py')
spec = importlib.util.spec_from_file_location('v21', helper)
v21 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(v21)
PARAM, CTX, LOOKUP, SOURCE = 0x10001000, 0x10020000, 0x10004000, 0x10008000
CACHE, PACKET, DISPLAY, EXIT = 0x10040000, 0x10050000, 0x10003000, 0x1007F000


def r32(u, address):
    return struct.unpack('<I', u.mem_read(address, 4))[0]


def run(u, start, count=2_000_000):
    u.reg_write(UC_X86_REG_RSP, 0x10060008)
    v21.w64(u, 0x10060008, EXIT)
    u.emu_start(start, EXIT, count=count)
    assert u.reg_read(UC_X86_REG_RIP) == EXIT, hex(u.reg_read(UC_X86_REG_RIP))


def initialize(kind):
    u = v21.make()
    stubs = []

    def code(u, ip, size, unused):
        if ip == 0x140346BEA:
            dst = u.reg_read(UC_X86_REG_RCX)
            n = u.reg_read(UC_X86_REG_R8)
            value = u.reg_read(UC_X86_REG_RDX) & 0xFF
            u.mem_write(dst, bytes([value])*n)
            u.reg_write(UC_X86_REG_RAX, dst)
            stubs.append({'target': hex(ip), 'operation': 'memset', 'bytes': n, 'value': value})
            v21.ret(u)

    hook = u.hook_add(UC_HOOK_CODE, code)
    u.reg_write(UC_X86_REG_RCX, CTX)
    u.reg_write(UC_X86_REG_RDX, PARAM)
    u.reg_write(UC_X86_REG_R8, kind)
    run(u, 0x140316E10)
    u.hook_del(hook)
    result = {'type': u.mem_read(PARAM, 1)[0],
        'checksumLengthBytes': struct.unpack('<H', u.mem_read(PARAM+6, 2))[0],
        'field2C': r32(u, PARAM+0x2C), 'field30': r32(u, PARAM+0x30),
        'field58': r32(u, PARAM+0x58), 'field5C': r32(u, PARAM+0x5C),
        'field60': r32(u, PARAM+0x60), 'stubs': stubs}
    assert result['type'] == kind
    assert stubs == [{'target': '0x140346bea', 'operation': 'memset', 'bytes': 640, 'value': 0}]
    if kind == 11:
        assert [result['field58'], result['field5C'], result['field60']] == [4, 3, 0]
        assert result['checksumLengthBytes'] == 80
    else:
        assert [result['field2C'], result['field30']] == [100, 100]
        assert result['checksumLengthBytes'] == 120
    return u, result


def viewport(index, alternate):
    u = v21.make()
    v21.w64(u, 0x140D6D300, DISPLAY)
    calls = []

    def code(u, ip, size, unused):
        if ip in [0x140332E30, 0x1403326C0]:
            calls.append(hex(ip))
            v21.ret(u)

    hook = u.hook_add(UC_HOOK_CODE, code)
    u.reg_write(UC_X86_REG_RCX, index)
    u.reg_write(UC_X86_REG_RDX, alternate)
    run(u, 0x140337CD0)
    u.hook_del(hook)
    dims = list(struct.unpack('<3H', u.mem_read(DISPLAY+0x20, 6)))
    assert dims == ([512, 256, 224] if index < 2 else [512, 512, 512 if alternate else 448])
    return {'index': index, 'alternateTable': bool(alternate),
        'width': dims[0], 'height': dims[1], 'thirdDimension': dims[2], 'stubs': calls}


def admit(kind, a, b):
    u, initial = initialize(kind)
    u.mem_write(LOOKUP+4, struct.pack('<20h', kind, *([-1]*19)))
    v21.w64(u, LOOKUP+0x58, PARAM)
    offsets = [0x1C, 0x20] if kind == 11 else [0, 4]
    for offset, value in zip(offsets, [a, b]):
        v21.w32(u, SOURCE+offset, value)
    u.reg_write(UC_X86_REG_RCX, LOOKUP)
    u.reg_write(UC_X86_REG_RDX, kind)
    u.reg_write(UC_X86_REG_R8, SOURCE)
    u.reg_write(UC_X86_REG_R9, 0)
    run(u, 0x14031F050)
    dst = [0x58, 0x5C] if kind == 11 else [0x2C, 0x30]
    observed = [r32(u, PARAM+x) for x in dst]
    assert observed == [a, b]
    assert u.reg_read(UC_X86_REG_RAX) == 0
    return {'type': kind, 'sourceOffsets': offsets, 'runtimeOffsets': dst,
        'input': [a, b], 'admitted': observed, 'importerStubs': []}


def checksum(u):
    u.reg_write(UC_X86_REG_RCX, CTX)
    u.reg_write(UC_X86_REG_RDX, PARAM)
    run(u, 0x1403162E0)
    return u.reg_read(UC_X86_REG_RAX) & 0xFFFFFFFF


def cache_gate(kind, changed, lane):
    u, initial = initialize(kind)
    before = checksum(u)
    if changed:
        v21.w32(u, PARAM+(0x58 if kind == 11 else 0x2C), 5 if kind == 11 else 60)
    after = checksum(u)
    v21.w32(u, PARAM+8, before)
    v21.w64(u, PARAM+0x10, CACHE)
    v21.w64(u, CACHE+0x40+lane*8, PACKET)
    v21.w64(u, CACHE+0x40+(1-lane)*8, PACKET+0x100)
    freed = []
    reached = []

    def code(u, ip, size, unused):
        if ip == 0x1402C6260:
            freed.append(u.reg_read(UC_X86_REG_RCX))
            v21.ret(u)
        elif ip in [0x1403154FC, 0x140315605]:
            reached.append(hex(ip))
            u.emu_stop()

    hook = u.hook_add(UC_HOOK_CODE, code)
    u.reg_write(UC_X86_REG_RBX, PARAM)
    u.reg_write(UC_X86_REG_RDI, CTX)
    u.reg_write(UC_X86_REG_R14, lane)
    u.reg_write(UC_X86_REG_RAX, after)
    u.reg_write(UC_X86_REG_RSP, 0x10060008)
    u.emu_start(0x1403154A9, 0x140315606, count=10000)
    u.hook_del(hook)
    mask = struct.unpack('<H', u.mem_read(PARAM+0xC, 2))[0]
    selected = struct.unpack('<Q', u.mem_read(CACHE+0x40+lane*8, 8))[0]
    other = struct.unpack('<Q', u.mem_read(CACHE+0x40+(1-lane)*8, 8))[0]
    assert before != after if changed else before == after
    assert reached == ['0x1403154fc' if changed else '0x140315605']
    assert selected == (0 if changed else PACKET)
    assert other == PACKET+0x100
    assert mask == ((3 ^ (1 << lane)) if changed else 0)
    assert freed == ([CACHE+lane*32] if changed else [])
    return {'type': kind, 'shapeParameterChanged': changed, 'frameLane': lane,
        'beforeChecksum': before, 'afterChecksum': after, 'dirtyMaskAfter': mask,
        'reached': reached[0], 'cacheClearObserved': selected == 0,
        'otherLanePreserved': True, 'freeHelperStubAddresses': [hex(x) for x in freed]}


def checksum_collision():
    u, initial = initialize(11)
    before = checksum(u)
    amplitude_before = r32(u, PARAM+0x18)
    v21.w32(u, PARAM+0x58, 3)
    v21.w32(u, PARAM+0x18, amplitude_before+0x10000)
    after = checksum(u)
    assert before == after
    v21.w32(u, PARAM+8, before)
    v21.w64(u, PARAM+0x10, CACHE)
    v21.w64(u, CACHE+0x40, PACKET)
    reached = []

    def code(u, ip, size, unused):
        if ip in [0x1403154FC, 0x140315605]:
            reached.append(hex(ip))
            u.emu_stop()

    hook = u.hook_add(UC_HOOK_CODE, code)
    u.reg_write(UC_X86_REG_RBX, PARAM)
    u.reg_write(UC_X86_REG_RDI, CTX)
    u.reg_write(UC_X86_REG_R14, 0)
    u.reg_write(UC_X86_REG_RAX, after)
    u.reg_write(UC_X86_REG_RSP, 0x10060008)
    u.emu_start(0x1403154A9, 0x140315606, count=10000)
    u.hook_del(hook)
    assert reached == ['0x140315605']
    assert struct.unpack('<Q', u.mem_read(CACHE+0x40, 8))[0] == PACKET
    return {'type': 11, 'beforeChecksum': before, 'afterChecksum': after,
        'field58Change': [4, 3], 'field18Change': [amplitude_before, amplitude_before+0x10000],
        'reached': reached[0], 'cacheInvalidated': False,
        'claimBoundary': 'Deliberately chosen raw parameter collision; no claim it occurs in retail assets'}


def cached11(width, height, xs, ys, initial_cache_bytes, phase=(0, 0), drift=(0, 0)):
    u, initial = initialize(11)
    v21.w32(u, PARAM+0x58, xs)
    v21.w32(u, PARAM+0x5C, ys)
    for offset, value in zip([0x70, 0x74, 0x20, 0x24], [*phase, *drift]):
        v21.w32(u, PARAM+offset, value & 0xFFFFFFFF)
    v21.w32(u, CTX+0x15D58, width)
    v21.w32(u, CTX+0x15D5C, height)
    v21.w64(u, CTX+0x15CB8, CACHE)
    v21.w64(u, CACHE+0x50, PACKET)
    u.mem_write(CACHE+0x5C, struct.pack('<H', 1))
    # Execute the producer's actual registry/length command writer slice.
    # Registry ID is synthetic; the byte->dword count encoding is unmodified EXE.
    v21.w64(u, CTX+0x15CB0, PACKET)
    u.reg_write(UC_X86_REG_RDI, CTX)
    u.reg_write(UC_X86_REG_R12, 0x140CC1AD0)
    u.reg_write(UC_X86_REG_RBX, 0x140CC1AD0+initial_cache_bytes)
    u.reg_write(UC_X86_REG_RAX, 0x1234)
    u.reg_write(UC_X86_REG_RSP, 0x10060008)
    u.emu_start(0x1403194E0, 0x140319511, count=100)
    assert r32(u, PACKET) == 0x1234
    assert r32(u, PACKET+4) == initial_cache_bytes//4
    assert struct.unpack('<Q', u.mem_read(PACKET+8, 8))[0] == 0x64
    v21.w64(u, 0x140D6D300, DISPLAY)
    v21.w64(u, 0x140CF2330, 0)  # Actual depth helper's no-camera route.
    before = checksum(u)
    conflict = None
    completed = False
    max_end = 0x140CC1AD0
    external = []
    packet_writes = []

    def code(u, ip, size, unused):
        nonlocal completed
        if ip == 0x1403455F0:
            external.append(hex(ip))
            v21.ret(u)
        elif ip == EXIT:
            completed = True

    def write(u, access, address, size, value, unused):
        nonlocal conflict, max_end
        if PACKET <= address < PACKET+0x100:
            packet_writes.append({'offset': address-PACKET, 'size': size})
        if 0x140CC1AD0 <= address < 0x140CD1AD0+0x100:
            max_end = max(max_end, address+size)
        if address < 0x140CD1AD4 and address+size > 0x140CD1AD0:
            conflict = {'ip': hex(u.reg_read(UC_X86_REG_RIP)), 'address': hex(address),
                'size': size, 'recordOrdinal': (u.reg_read(UC_X86_REG_R10)-0x140CC1AD0)//40+1}
            u.emu_stop()

    hook1 = u.hook_add(UC_HOOK_CODE, code)
    hook2 = u.hook_add(UC_HOOK_MEM_WRITE, write)
    u.reg_write(UC_X86_REG_RCX, CTX)
    u.reg_write(UC_X86_REG_RDX, PARAM)
    u.reg_write(UC_X86_REG_RSP, 0x10060008)
    v21.w64(u, 0x10060008, EXIT)
    u.emu_start(0x14031A830, EXIT, count=2_000_000)
    completed = u.reg_read(UC_X86_REG_RIP) == EXIT
    end = u.reg_read(UC_X86_REG_R10)-0x140CC1AD0
    u.hook_del(hook1)
    u.hook_del(hook2)
    after = checksum(u)
    expected = 40*(height//(1 << ys)+2)*(width//(1 << xs)+3)
    assert before == after  # Phase accumulation is outside this record's checksum span.
    assert r32(u, PACKET) == 0x1234
    assert r32(u, PACKET+4) == initial_cache_bytes//4
    assert packet_writes == [{'offset': 20, 'size': 4}, {'offset': 16, 'size': 4}, {'offset': 24, 'size': 8}]
    assert struct.unpack('<H', u.mem_read(CACHE+0x5A, 2))[0] == 1
    assert struct.unpack('<Q', u.mem_read(PACKET+0x18, 8))[0] == 0x64
    if completed:
        assert conflict is None and end == expected and max_end-0x140CC1AD0 == expected
    else:
        assert conflict == {'ip': '0x14031ab97', 'address': '0x140cd1ad0', 'size': 4, 'recordOrdinal': 1639}
    return {'display': [width, height], 'shifts': [xs, ys], 'initialPhase': list(phase),
        'drift': list(drift), 'formulaBytes': expected, 'completed': completed,
        'generatedEndDelta': end, 'knownScalarWrite': conflict,
        'inputChecksumUnchanged': True, 'cachedEncodedDwordLength': r32(u, PACKET+4),
        'cachedSourceBytesUnchanged': initial_cache_bytes, 'packetWrites': packet_writes,
        'patchCommand': '0x64', 'externalStubs': external,
        'claimBoundary': 'Synthetic cache and display; actual count-writer slice followed by updater, not full allocator/packet construction or actual GPU draw'}


if __name__ == '__main__':
    result = {'schema': 'dmc-rengine.dmc3-effect-parameter-admission-verification.v1',
        'canonicalExeSha256': v21.SHA,
        'scope': 'Unmodified EXE instruction fixtures; no original-game frame or retail asset execution',
        'defaults': [initialize(t)[1] for t in [11, 12]],
        'viewport': [viewport(i, alternate) for alternate in [0, 1] for i in range(3)],
        'admission': [admit(t, a, b) for t, a, b in [(11, 4, 3), (11, 3, 3), (11, 0, 31),
            (11, 0xFFFFFFFF, 32), (12, 100, 100), (12, 50, 50), (12, 0, 0), (12, 0xFFFFFFFF, 201)]],
        'cacheInvalidation': [cache_gate(t, changed, lane) for t in [11, 12]
            for changed in [False, True] for lane in [0, 1]],
        'checksumCollision': [checksum_collision()],
        'producer11': [v21.sub11(*x) for x in [(512, 256, 4, 3), (512, 512, 4, 3),
            (640, 360, 4, 3), (512, 256, 3, 3)]],
        'producer12': [v21.sub12(*x) for x in [(100, 100), (50, 50), (0, 0)]],
        'cached11': [cached11(*x) for x in [(512, 256, 4, 3, 47600),
            (512, 256, 4, 3, 47600, (15, 7), (16, 8)),
            (640, 360, 5, 5, 11960), (640, 360, 4, 4, 41280),
            (512, 512, 4, 3, 47600), (512, 256, 3, 3, 47600)]]}
    for case in result['producer11']:
        if case['completed']:
            assert case['endDelta'] == case['formulaBytes']
        else:
            assert case['knownScalarWrite']['recordOrdinal'] == 1639
    for case in result['producer12']:
        if case['completed']:
            assert case['endDelta'] == 9600
        else:
            assert case['knownNeighborWrite']['recordOrdinal'] == 410
    case_keys = ['defaults', 'viewport', 'admission', 'cacheInvalidation', 'checksumCollision', 'producer11', 'producer12', 'cached11']
    result['verification'] = {'cases': sum(len(result[k]) for k in case_keys), 'passed': True}
    Path(sys.argv[2]).write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps(result['verification']))
