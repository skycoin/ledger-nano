from adpu_call import send_to_ledger_with_bip44
import binascii

public_key = send_to_ledger_with_bip44(ins=0x04, le=32)

print "\n"
print "public_key [" + str(len(binascii.hexlify(public_key))) + "] = " + binascii.hexlify(public_key)
