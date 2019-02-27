import binascii

from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException


bip44_path = ( # example of bip44 path
      "8000002C"
    + "80000378"
    + "80000000"
    + "00000000"
    + "00000000"
    )


dongle = getDongle(True)

publicKey = dongle.exchange(
    bytes(bytearray.fromhex("80040000FF" + bip44_path)))

print "\n"
print "publicKey [" + str(len(binascii.hexlify(publicKey))) + "] = " + binascii.hexlify(publicKey)
