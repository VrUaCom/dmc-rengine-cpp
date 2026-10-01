"""Write every record of an FXBANK slot as <out>/<pac>_slot<N>_<K><id>.bin (local only, never commit).

python3 dump_bank_records.py em000.pac 41 records/
FXBANK slots: em000 / em006 / em007 41, em028 9, em034 28, plwp_sword / plwp_2sword 2,
st00x_effect.pac 0 (the whole file is the bank: pass slot -1).
"""
import os, sys
from records import pac_slots, bank_from_bytes

pac, slot, out = sys.argv[1], int(sys.argv[2]), sys.argv[3]
os.makedirs(out, exist_ok=True)
b = open(pac, 'rb').read()
bank = bank_from_bytes(b if slot < 0 else pac_slots(b)[slot])
base = os.path.basename(pac)
for (kind, id_), data in bank.items():
    open(os.path.join(out, f'{base}_slot{max(slot, 0)}_{kind}{id_}.bin'), 'wb').write(data)
print(len(bank), 'records')
