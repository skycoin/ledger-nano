package main

import (
	"fmt"
	"testing"
)

func Sum(a int, b int) int {
	return a + b
}

func TestSum(t *testing.T) {
	total := Sum(5, 5)
	if total != 10 {
		t.Errorf("Sum was incorrect, got: %d, want: %d.", total, 10)
	}
}

func main() {
	fmt.Println("Testing...")
}
