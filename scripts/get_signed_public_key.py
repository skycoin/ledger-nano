from adpu_call import send_to_ledger
import binascii

signed_pk = send_to_ledger(ins=0x08, le=32)

print "\n"
print "signed public key [" + str(len(binascii.hexlify(signed_pk))) + "] = " + binascii.hexlify(signed_pk)
