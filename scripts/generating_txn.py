import adpu_call
from sign_transaction import send_txn

def to_le(num, byte_number):
    hx = hex(num)[2:]
    hx = ('0' if len(hx) % 2 == 1 else '') + hx
    hx = [hx[i:i+2] for i in range(0, len(hx), 2)]
    hx = "".join(hx[::-1])
    hx = hx + (byte_number*2 - len(hx))*'0'
    return hx

class Input:
    def __init__(self, uxId, address_index):
        self.uxId = uxId
        self.address_index = address_index

    def serialized(self):
        return self.uxId + to_le(self.address_index, 4)

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
            sky = to_le(sky, 8)
            ret = ret + sky
        return ret

def createRawTransaction(inputs, outputs):
    type = "00"
    input_num = to_le(len(inputs), 4)
    output_num = to_le(len(outputs), 4)

    txn_in_out = input_num + "".join(map(lambda x: x.serialized(), inputs)) + output_num + "".join(map(lambda x: x.serialized(), outputs))
    inner_hash, sigs = send_txn(txn_in_out)
    print(inner_hash, len(sigs))
    txn_in_out = input_num + "".join(map(lambda x: x.uxId, inputs)) + output_num + "".join(map(lambda x: x.serialized(), outputs))
    txn = type + inner_hash + input_num + "".join(sigs) + txn_in_out
    length = to_le(len(txn)//2+4, 4)
    txn = length + txn
    return txn

outputs = [Output("LtdGkoRPyvBASvZZ7WJWUBSgosfCUjmkYo", 0.01, 1),
           Output("hbWbgT97s7RoGEfqoiiJgV5pkdaAsBow3o", 0.01, 1),
           Output("2cPurVYpW1MCqk7cjNjqDG531BoRCgJ8iTD", 0.01, 1),
           Output("qGJNvH2LM1jrawo1ZggreLYaTpwUXCkzWh", 0.01, 1),
           Output("HiDEhbBdNu2pSb3pbTLKKSbjWaqJTQWsAi", 0.01, 1),
           Output("2x5dAJzZQzKHQNKwrGCYz8cvehxDWMp7i1", 0.01, 1),
           Output("2Fxv5KoVxyTDHdSHFFMxoQrfUQZPs9DM9XR", 0.01, 1),
           Output("2NQAW9WJSDxosKVxosrC8RkWHtHTveM9oDr", 0.01, 1),
           Output("2ZsCqmqhZ8t7yAerdYLbCfTj4xtdPB97W6k", 0.01, 1)
           ]

inputs = [Input("fef9f78f541034e05388c55a63ba2d5a48a9af4f2a98d0dae94929619d306319", 0),
          Input("80b17dfdac22f09e200a9f2a1604603952b2e2467d65929403f54ea9c0e13b0f", 1),
          Input("f933e5fad352fc7b703cee1483373b597024b8064c7f4614cdb911f58c3ae83b", 2),
          Input("f53122e7687eaab62d57f2c63fa29d8b23b8416769e168f3ce4a1a07eb930601", 3),
          Input("b0b61b32775449657e3bdfc232174cc14dbcfa822494f812313df0a651bebde1", 4),
          Input("f83b6c8572c796703367f081a3e635be32d531e08e909f9bb6f5e7c1d2de21ac", 5),
          Input("64532b664d5d89373021dd37565a678ff489191becf1a78091eeebc24f37548d", 6),
          Input("6352b7f806ba23f1f3581e3dbc80ca7893e123cdfa64d2d0bfe4c68ec3fc4eba", 7),
          Input("58910a2ec90d92945abd711a8530731eea819b74f66fe90770e25e082852d157", 8)
          ]

# input = "b1b1499b5bddff2bd6c1b40f0744bed32c1eb2dfd909a5aaa928c1c1b641fd84"
txn = createRawTransaction(inputs, outputs)
print(txn)
