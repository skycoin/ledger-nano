from adpu_call import send_to_ledger_with_bip44
import binascii

address = send_to_ledger_with_bip44(ins=0x2,le=20)

print "\n"
print "address [" + str(len(binascii.b2a_qp(address))) + "] = " + binascii.b2a_qp(address)
