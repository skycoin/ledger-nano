import binascii

from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException


bip44_path = ( # example of bip44 path
      "8000002C"
    + "80000378"
    + "00000000"
    + "00000000"
    + "00000000"
    )


dongle = getDongle(True)

address = dongle.exchange(
    bytes(bytearray.fromhex("80020000FF" + bip44_path))) # the main part here is 02 -> instruction code for get_address

print "\n"
print "address [" + str(len(binascii.hexlify(address))) + "] = " + binascii.hexlify(address)
