package main

import (
	"fmt"
	"log"

	"github.com/RomanMilishchuk/hardware-wallet-go/src/ledger"
)

func main() {
	ledger.DEBUG = true

	var nanos *ledger.NanoS
	nanos, err := ledger.OpenNanoS()
	if err != nil {
		log.Fatalln("Couldn't open device:", err)
	}

	// fmt.Println(nanos.GetPublicKey(0x80000000, 0x00000000))
	fmt.Println(nanos.GetVersion())
}
