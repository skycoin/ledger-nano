package main

import (
	"fmt"
	"log"

	"github.com/RomanMilishchuk/hardware-wallet-go/src/ledger"
)

func main() {
	fmt.Println("Hello!")

	var nanos *ledger.NanoS
	nanos, err := ledger.OpenNanoS()
	if err != nil {
		log.Fatalln("Couldn't open device:", err)
	}

	fmt.Println(nanos.GetVersion())
}
