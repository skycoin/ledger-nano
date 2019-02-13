import binascii

from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException


bipp32_path = ( # example of bip32 path
      "8000002C"
    + "80000378"
    + "80000000"
    + "00000000"
    )


dongle = getDongle(True)

publicKey = dongle.exchange(
    bytes(bytearray.fromhex("80040000FF" + bipp32_path)))

print "\n"
print "publicKey [" + str(len(binascii.hexlify(publicKey))) + "] = " + binascii.hexlify(publicKey)
