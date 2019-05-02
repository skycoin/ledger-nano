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

input = "b1b1499b5bddff2bd6c1b40f0744bed32c1eb2dfd909a5aaa928c1c1b641fd84"
txn = createRawTransaction(inputs, outputs)
# print txn
a = "e7040000004d42c58b31491b20e0c9244e5f229c1580b03ee75f590300d92e754d97b6dc4109000000314ae1cb365eaf90a1d70ae6aefe9b286c1bd57b4c24bb5a03eb5bf47092802b248e6737572e92b2c87810db86a62e705e61e0c10e3919b0f7eb07d63d0dd4aa012ae4401e1987a793e6dc71900452da9998fd3c4169f305c6bc5a5fdedd37ff1b4b2029d16f0cb7f65781bb4a1697a809e9f85891b9aa70a4495713810a36cd8b00b693f0c601f28cd55aa95eb0d300073b6dfbe1bd4d5722fa4f3a32bc90e139fd524d73b423cb17d68f30587e109689d4ff187cf766dce91078b8545011b53c1f013754033d27459c89090470e924f9debfc9bf5bbd8c150c4c77cedacad617529d0df661dc3a76f1c0da579dfaaad5868ae747ed438498e47f765016732bc695bb00bd3b2285295d18ecfb9c7ea993f0750487034dd50e76668e590d8c6f5e78cc470b58a491a24572fd4f9594df6826faf2c5fb85285687fc6cf53d0bace59a820a013404af83d1e9a077ee6799796b6734bc85f62e365a7a8c951c57e8eb052230605741e0a4f916c191f3a0869a90cf64ad3fcb2019db63ac67abb449b1ad971464009f2a9594d7d54a428ada746385747c96d0e4193d78e464d82553ee1ce1ff2b545fae2ce838fd34a6532076755492d9963ab8df625fa0b3a81960df885a567cb801474d748760e136402d3ca73a56a60c95c222a464ca49591b0486e343332d2b024893e1256aef68a16f2b4e5fe158ed3697f308bae30db33fe0f60b0f59e2d0e600ee0426739e2c0b818d3bc244d4ce5fa12ca84ca50c62520cd587c9192a62eaec34d08d220b6f0e91eebb76713797fd78085d470188a0fa25e99d05749b2516a30009000000fef9f78f541034e05388c55a63ba2d5a48a9af4f2a98d0dae94929619d30631980b17dfdac22f09e200a9f2a1604603952b2e2467d65929403f54ea9c0e13b0ff933e5fad352fc7b703cee1483373b597024b8064c7f4614cdb911f58c3ae83bf53122e7687eaab62d57f2c63fa29d8b23b8416769e168f3ce4a1a07eb930601b0b61b32775449657e3bdfc232174cc14dbcfa822494f812313df0a651bebde1f83b6c8572c796703367f081a3e635be32d531e08e909f9bb6f5e7c1d2de21ac64532b664d5d89373021dd37565a678ff489191becf1a78091eeebc24f37548d6352b7f806ba23f1f3581e3dbc80ca7893e123cdfa64d2d0bfe4c68ec3fc4eba58910a2ec90d92945abd711a8530731eea819b74f66fe90770e25e082852d1570900000000316e6527b35f105ab161f648c7cb7415d72feba8102700000000000001000000000000000064e3311b82c8da9d8f04bf9ae29db7a661d4c75e1027000000000000010000000000000000e81b683060ab6bb2428965e0d274a5f4dd85a745102700000000000001000000000000000077f2316fd44dec49d126ad6906ed4abf8ba4db4f10270000000000000100000000000000002987824fe2b4ce03d23b09dc7f65c888ef38a510102700000000000001000000000000000e4d866100ce55d211b08caea0044bf064d4380a901027000000000000010000000000000000b554e5d8c5feca74c6bf39e71ef9ef515cdcf66f1027000000000000010000000000000000c553274eef8cd33e7d8f5ffdefb6c93f0c5a7d7b1027000000000000010000000000000000e1d2242b79421ab0f2ca858ed46e27d01ae0ffc310270000000000000100000000000000"
print a == txn
