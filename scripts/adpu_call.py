from ledgerblue.commException import CommException
from ledgerblue.comm import getDongle

bip44_path = ( # example of bip44 path
    "8000002C"      # purpose
    + "80000378"    # coin_type
    + "00000000"    # account
    + "00000000"    # change
    + "00000000"    # address_index
)

def send_to_ledger(ins, p1=0x0, p2=0x0, le=0x0):
    dongle = getDongle(True)
    req = '80' # CLA
    for a in [ins, p1, p2]:
        req = req + ('0' if a < 16 else '') + hex(a)[2:]
    req = req + "14" + bip44_path + ('0' if le < 16 else '') + hex(le)[2:]
    return dongle.exchange(bytes(bytearray.fromhex(req)))
