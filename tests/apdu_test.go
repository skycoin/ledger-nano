package main

import (
	"fmt"
	"os"
	"testing"

	"github.com/RomanMilishchuk/hardware-wallet-go/src/ledger"
	"github.com/skycoin/skycoin/src/cipher"
	"github.com/skycoin/skycoin/src/coin"
	"github.com/stretchr/testify/require"
)

var (
	nanos   *ledger.NanoS
	errOpen error
)

func openConnection() {
	nanos, errOpen = ledger.OpenNanoS()

	if errOpen != nil {
		fmt.Println("Can not open Skycoin Ledger app")
	}
}

func TestGetVersion(t *testing.T) {
	version, errGetVersion := nanos.GetVersion()
	require.NoError(t, errGetVersion, "Error while getting Skycoin app version")

	fmt.Println("Skycoin app version: ", version)
}

func TestGetPublicKey(t *testing.T) {
	var account, index uint32
	account, index = 0x80000000, 0x00000000

	publicKey, errGetPublicKey := nanos.GetPublicKey(account, index)
	require.NoError(t, errGetPublicKey, "Error while getting public key")

	fmt.Printf("Skycoin app version (account=%d, index=%d): %s\n", account, index, publicKey.Hex())
}

func TestGetAddress(t *testing.T) {
	var account, index uint32
	account, index = 0x80000000, 0x00000000

	pk, errGetPublicKey := nanos.GetPublicKey(account, index)
	require.NoError(t, errGetPublicKey, "Error while getting public key")

	address, errGetAddress := nanos.GetAddress(account, index)
	require.NoError(t, errGetAddress, "Error while getting address")

	require.NoError(t, address.Verify(pk), "Address is not valid for the public key which was returned by ledger")

	fmt.Printf("Address (account=%d, index=%d): %s\n", account, index, address.String())
}

func TestTransactionSigning(t *testing.T) {
	var account, index uint32
	account, index = 0x80000000, 0x00000000

	inputs := [9]cipher.SHA256{
		cipher.MustSHA256FromHex("fef9f78f541034e05388c55a63ba2d5a48a9af4f2a98d0dae94929619d306319"),
		cipher.MustSHA256FromHex("80b17dfdac22f09e200a9f2a1604603952b2e2467d65929403f54ea9c0e13b0f"),
		cipher.MustSHA256FromHex("f933e5fad352fc7b703cee1483373b597024b8064c7f4614cdb911f58c3ae83b"),
		cipher.MustSHA256FromHex("f53122e7687eaab62d57f2c63fa29d8b23b8416769e168f3ce4a1a07eb930601"),
		cipher.MustSHA256FromHex("b0b61b32775449657e3bdfc232174cc14dbcfa822494f812313df0a651bebde1"),
		cipher.MustSHA256FromHex("f83b6c8572c796703367f081a3e635be32d531e08e909f9bb6f5e7c1d2de21ac"),
		cipher.MustSHA256FromHex("64532b664d5d89373021dd37565a678ff489191becf1a78091eeebc24f37548d"),
		cipher.MustSHA256FromHex("6352b7f806ba23f1f3581e3dbc80ca7893e123cdfa64d2d0bfe4c68ec3fc4eba"),
		cipher.MustSHA256FromHex("58910a2ec90d92945abd711a8530731eea819b74f66fe90770e25e082852d157"),
	}
	inputs_indexes := [9]uint32{0, 1, 2, 3, 4, 5, 6, 7, 8}
	outputs := [9]coin.TransactionOutput{
		coin.TransactionOutput{cipher.MustDecodeBase58Address("LtdGkoRPyvBASvZZ7WJWUBSgosfCUjmkYo"), 10000, 1},
		coin.TransactionOutput{cipher.MustDecodeBase58Address("hbWbgT97s7RoGEfqoiiJgV5pkdaAsBow3o"), 10000, 1},
		coin.TransactionOutput{cipher.MustDecodeBase58Address("2cPurVYpW1MCqk7cjNjqDG531BoRCgJ8iTD"), 10000, 1},
		coin.TransactionOutput{cipher.MustDecodeBase58Address("qGJNvH2LM1jrawo1ZggreLYaTpwUXCkzWh"), 10000, 1},
		coin.TransactionOutput{cipher.MustDecodeBase58Address("HiDEhbBdNu2pSb3pbTLKKSbjWaqJTQWsAi"), 10000, 1},
		coin.TransactionOutput{cipher.MustDecodeBase58Address("2x5dAJzZQzKHQNKwrGCYz8cvehxDWMp7i1"), 10000, 1},
		coin.TransactionOutput{cipher.MustDecodeBase58Address("2Fxv5KoVxyTDHdSHFFMxoQrfUQZPs9DM9XR"), 10000, 1},
		coin.TransactionOutput{cipher.MustDecodeBase58Address("2NQAW9WJSDxosKVxosrC8RkWHtHTveM9oDr"), 10000, 1},
		coin.TransactionOutput{cipher.MustDecodeBase58Address("2ZsCqmqhZ8t7yAerdYLbCfTj4xtdPB97W6k"), 10000, 1},
	}

	_, errGetAddress := nanos.GetAddress(account, index)
	require.NoError(t, errGetAddress, "Error while getting address before signing the tx")

	signedTxn, errSignTxn := nanos.SignTxn(inputs[:], inputs_indexes[:], outputs[:])
	require.NoError(t, errSignTxn, "Error while signing the transaction")

	require.NoError(t, signedTxn.Verify(), "Signed transaction verifying has been failed")
	require.Equal(t, signedTxn.HashInner, signedTxn.HashInner(), "Inner hashes do not match")
}

func TestMain(m *testing.M) {
	openConnection()
	code := m.Run()
	os.Exit(code)
}
