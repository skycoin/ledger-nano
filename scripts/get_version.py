from adpu_call import send_to_ledger
import binascii

version = send_to_ledger("01")

print "\n"
print "address [" + str(len(binascii.hexlify(version))) + "] = " + binascii.hexlify(version)
