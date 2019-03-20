from sign_transaction import send_txn

class Input:
    def __init__(self, uxId, address_index):
        self.uxId = uxId
        self.address_index = address_index

class Output:
    @staticmethod
    def base58decode(s):
        b58 = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'
        result = 0
        for c in s:
            result = result*58 + b58.index(c)
        return result

    def __init__(self, base58addr, amount, fee):
        addr = hex(self.base58decode(base58addr))[2:]
        self.addr = addr[20*2:21*2] + addr[:20*2]
        self.amount = int(amount * 10**6)
        self.fee = fee

    def serialized(self):
        ret = self.addr
        for sky in [self.amount, self.fee]:
            sky = hex(sky)[2:]
            sky = ('0' if len(sky) % 2 == 1 else '') + sky
            sky = [sky[i:i+2] for i in range(0, len(sky), 2)]
            sky = "".join(sky[::-1])
            sky = sky + (16 - len(sky))*'0'
            ret = ret + sky
        return ret

def createRawTransaction(inputs, outputs):
    type = "00"

    input_num = hex(len(inputs))[2:]
    input_num = ('0' if len(input_num) % 2 == 1 else '') + input_num
    input_num = input_num + (8 - len(input_num))*'0'

    output_num = hex(len(outputs))[2:]
    output_num = ('0' if len(output_num) % 2 == 1 else '') + output_num
    output_num = output_num + (8 - len(output_num))*'0'
    txn_in_out = input_num + "".join(inputs) + output_num + "".join(map(lambda x: x.serialized(), outputs))
    inner_hash, sigs = send_txn(txn_in_out)

    txn = type + inner_hash + input_num + "".join(sigs) + txn_in_out
    length = hex(len(txn)//2+4)[2:]
    length = ('0' if len(length) % 2 == 1 else '') + length
    length = [length[i:i+2] for i in range(0, len(length), 2)]
    length = "".join(length[::-1])
    length = length + (8 - len(length))*'0'
    txn = length + txn
    return txn

outputs = [Output("LtdGkoRPyvBASvZZ7WJWUBSgosfCUjmkYo", 0.01, 1)]
           # Output("hbWbgT97s7RoGEfqoiiJgV5pkdaAsBow3o", 0.01, 1),
           # Output("2cPurVYpW1MCqk7cjNjqDG531BoRCgJ8iTD", 0.01, 1)]
           # Output("qGJNvH2LM1jrawo1ZggreLYaTpwUXCkzWh", 0.01, 1),
           # Output("HiDEhbBdNu2pSb3pbTLKKSbjWaqJTQWsAi", 0.01, 1)]

# inputs = ["abf988e09d2e6e17f464b37739f939c2203489c459491a04875c2612f1e308fe"]
inputs = ["be6e678ad3edb872a199fd710c8e3da2c51a926303affd14b5009fda8bd90572"]

# input = "b1b1499b5bddff2bd6c1b40f0744bed32c1eb2dfd909a5aaa928c1c1b641fd84"
txn = createRawTransaction(inputs, outputs)
print(txn)
