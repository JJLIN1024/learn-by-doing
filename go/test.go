package main

import (
	"fmt"
)

type Vertex struct {
	X, Y int
}

func (v Vertex) Abs() int {
	return v.X * v.Y
}

func (v Vertex) randon() int {
	return 111
}
func main() {
	fmt.Println("HellO")
	v := Vertex{1, 2}
	fmt.Println(v.Abs())
	fmt.Println(v.randon())
}
