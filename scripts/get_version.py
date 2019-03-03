from adpu_call import send_to_ledger_with_bip44
import binascii

version = send_to_ledger_with_bip44(ins=0x01, le=3)

print "\n"
print "version [" + str(len(binascii.hexlify(version))) + "] = " + binascii.hexlify(version)
