package main

import (
	"fmt"
	"testing"

	"github.com/RomanMilishchuk/hardware-wallet-go/src/ledger"
	"github.com/stretchr/testify/require"
)

func TestGetVersion(t *testing.T) {
	var nanos *ledger.NanoS

	nanos, errOpen := ledger.OpenNanoS()
	require.NoError(t, errOpen)

	version, errGetVersion := nanos.GetVersion()
	require.NoError(t, errGetVersion)

	fmt.Println("Skycoin app version: ", version)
}
