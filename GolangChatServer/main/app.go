package main

import (
	"fmt"

	"github.com/Hughshine/learngo/calc"
	calcNew "github.com/Hughshine/learngo/v2/calc"
)

func main() {
	result := calc.Add(1, 2, 3)
	fmt.Println("calc.Add(1, 2, 3) => ", result)

	// v2
	err, v2Result := calcNew.Add()
	if err != nil {
		fmt.Println("Error: => ", err)
	} else {
		fmt.Println("v2Result: =>", v2Result)
	}
}