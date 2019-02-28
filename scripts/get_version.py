from adpu_call import send_to_ledger
import binascii

version = send_to_ledger(ins=0x01, le=3)

print "\n"
print "version [" + str(len(binascii.hexlify(version))) + "] = " + binascii.hexlify(version)
