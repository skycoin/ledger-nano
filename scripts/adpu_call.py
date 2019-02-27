from ledgerblue.commException import CommException
from ledgerblue.comm import getDongle

bip44_path = ( # example of bip44 path
    "8000002C"
    + "80000378"
    + "00000000"
    + "00000000"
    + "00000000"
)

def send_to_ledger(ins):
    dongle = getDongle(True)

    return dongle.exchange(bytes(bytearray.fromhex("80" + ins + "0000FF" + bip44_path)))
