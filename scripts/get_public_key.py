from adpu_call import send_to_ledger
import binascii

public_key = send_to_ledger("04")

print "\n"
print "public_key [" + str(len(binascii.hexlify(public_key))) + "] = " + binascii.hexlify(public_key)
