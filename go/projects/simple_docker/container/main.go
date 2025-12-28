package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"syscall"
)

func main() {
	switch os.Args[1] {
	case "run":
		parent()
	case "child":
		child()
	default:
		panic("Panic!!")
	}
}

func parent() {
	cmd := exec.Command("/proc/self/exe", append([]string{"child"}, os.Args[2:]...)...)
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	// CLONE_NEWNS : name space, for file mounting
	// CLONE_NEWUTS: Unix Timesharing System, for hostname
	// CLONE_NEWPID: for process
	cmd.SysProcAttr = &syscall.SysProcAttr{
		Cloneflags: syscall.CLONE_NEWNS | syscall.CLONE_NEWUTS | syscall.CLONE_NEWPID,
	}

	if err := cmd.Run(); err != nil {
		fmt.Println("Error: ", err)
		os.Exit(1)
	}
}

func child() {
	must(syscall.Sethostname([]byte("mini-docker")))
	must(syscall.Mount("", "/", "", syscall.MS_REC|syscall.MS_SLAVE, "")) // MS_SLAVE => Unidirectional, MS_PRIVATE => Isolate

	rootfs := "./rootfs"
	must(syscall.Mount(rootfs, rootfs, "", syscall.MS_BIND|syscall.MS_REC, ""))

	oldRoot := filepath.Join(rootfs, ".old_root")
	must(os.MkdirAll(oldRoot, 0700))

	must(syscall.PivotRoot(rootfs, oldRoot))

	must(os.Chdir("/"))

	must(syscall.Unmount("/.old_root", syscall.MNT_DETACH))
	must(os.Remove("/.old_root"))

	must(syscall.Mount("proc", "/proc", "proc", syscall.MS_RDONLY|syscall.MS_NOSUID|syscall.MS_NOEXEC, "")) // source, target, fstype, flags, args

	cmd := exec.Command(os.Args[2], os.Args[3:]...)
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	if err := cmd.Run(); err != nil {
		fmt.Println("ERROR", err)
		os.Exit(1)
	}
}

func must(err error) {
	if err != nil {
		panic(err)
	}
}
