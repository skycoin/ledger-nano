from adpu_call import send_to_ledger
import binascii

address = send_to_ledger(ins=0x2,le=20)

print "\n"
print "address [" + str(len(binascii.b2a_qp(address))) + "] = " + binascii.b2a_qp(address)