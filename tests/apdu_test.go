package main

import (
	"fmt"
	"testing"
	"os"

	"github.com/RomanMilishchuk/hardware-wallet-go/src/ledger"
	"github.com/stretchr/testify/require"
)

var (
	nanos *ledger.NanoS
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

	// fmt.Println("Skycoin app version: ", version)
}

func TestGetPublicKey(t *testing.T) {
	var account, index uint32
	account, index = 0x80000000, 0x00000000

	_, errGetPublicKey := nanos.GetPublicKey(account, index)
	require.NoError(t, errGetPublicKey, "Error while getting public key")

	// fmt.Printf("Skycoin app version (account=%d, index=%d): %s\n", account, index, publicKey)
}


func TestMain(m *testing.M) {	
	openConnection()
    code := m.Run() 
    os.Exit(code)
}