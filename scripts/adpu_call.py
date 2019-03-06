from ledgerblue.commException import CommException
from ledgerblue.comm import getDongle

dongle = getDongle(True)

bip44_path = ( # example of bip44 path
    "8000002C"      # purpose
    + "80000000"    # coin_type (e.g. "80000378")
    + "80000000"    # account
    + "00000000"    # change
    + "00000000"    # address_index
)

def send_to_ledger_with_bip44(ins, p1=0x0, p2=0x0, le=0x0):
    return send_to_ledger(ins, p1, p2,  bip44_path, le)

def send_to_ledger(ins, p1=0x0, p2=0x0, data="", le=0x0):
    req = '80' # CLA
    for a in [ins, p1, p2]:
        req = req + ('0' if a < 16 else '') + hex(a)[2:]
    lc = hex(len(data)//2)[2:]
    if len(lc) > 2:
        raise ValueError("Data is too big (max 256 bytes)")
    le = ('0' if le < 16 else '') + hex(le)[2:]
    req = req + lc + data + le
    ret = dongle.exchange(bytes(bytearray.fromhex(req)))
    return ret
