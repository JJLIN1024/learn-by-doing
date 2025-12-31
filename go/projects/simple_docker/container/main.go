package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"syscall"
)

type Config struct {
	Rootfs     string
	OverlayDir string
	Hostname   string
}

var containerCfg = Config{
	Rootfs:     "./rootfs",
	OverlayDir: "./overlay",
	Hostname:   "mini-docker",
}

func main() {
	if len(os.Args) < 2 {
		printUsage()
		os.Exit(1)
	}

	switch os.Args[1] {
	case "run":
		if len(os.Args) < 3 {
			fmt.Fprintf(os.Stderr, "Error: No command specified for container to run.\n\n")
			printUsage()
			os.Exit(1)
		}
		parent()
	case "child":
		child()
	case "help", "-h", "--help":
		printUsage()
	default:
		fmt.Fprintf(os.Stderr, "Error: Unknown command '%s'\n\n", os.Args[1])
		printUsage()
		os.Exit(1)
	}
}

func printUsage() {
	fmt.Printf(`
Mini-Docker: A lightweight container implementation in Go.

Usage:
  mini-docker run <command> [arguments...]
  mini-docker help

Arguments:
  <command>    The command to execute inside the isolated environment (e.g., /bin/sh)
  [arguments]  Optional arguments for the command

Example:
  sudo ./mini-docker run /bin/bash

Note: 
  This tool requires root privileges to manage namespaces, cgroups, and mounting.
`)
}

func parent() {
	fmt.Printf("[*] Spawning child process with namespaces...\n")
	cmd := exec.Command("/proc/self/exe", append([]string{"child"}, os.Args[2:]...)...)
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	cmd.SysProcAttr = &syscall.SysProcAttr{
		Cloneflags: syscall.CLONE_NEWNS | syscall.CLONE_NEWUTS | syscall.CLONE_NEWPID,
	}

	if err := cmd.Run(); err != nil {
		fmt.Fprintf(os.Stderr, "[-] Parent failed: %v\n", err)
		os.Exit(1)
	}
}

func child() {
	must(syscall.Sethostname([]byte(containerCfg.Hostname)))
	must(syscall.Mount("", "/", "", syscall.MS_REC|syscall.MS_SLAVE, "")) // MS_SLAVE => Unidirectional, MS_PRIVATE => Isolate

	fmt.Printf("[+] Container internal setup started (PID: %d)\n", os.Getpid())
	fmt.Printf("[+] Configuring Cgroup v2...\n")
	setupCgroup()

	fmt.Printf("[+] Setting up OverlayFS at %s...\n", containerCfg.OverlayDir)
	mergedDir := setupOverlay()

	fmt.Printf("[+] Executing PivotRoot...\n")
	pivotRoot(mergedDir)
	must(syscall.Mount("proc", "/proc", "proc", syscall.MS_RDONLY|syscall.MS_NOSUID|syscall.MS_NOEXEC, ""))

	fmt.Printf("[*] --- Container Running ---\n\n")
	cmd := exec.Command(os.Args[2], os.Args[3:]...)
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	if err := cmd.Run(); err != nil {
		fmt.Fprintf(os.Stderr, "[-] Command failed: %v\n", err)
		os.Exit(1)
	}
}

func setupOverlay() string {
	upper := filepath.Join(containerCfg.OverlayDir, "upper")
	work := filepath.Join(containerCfg.OverlayDir, "work")
	merged := filepath.Join(containerCfg.OverlayDir, "merged")

	must(os.MkdirAll(upper, 0755))
	must(os.MkdirAll(work, 0755))
	must(os.MkdirAll(merged, 0755))

	opts := fmt.Sprintf("lowerdir=%s,upperdir=%s,workdir=%s", containerCfg.Rootfs, upper, work)
	must(syscall.Mount("overlay", merged, "overlay", 0, opts))
	return merged
}

func setupCgroup() {
	path := "/sys/fs/cgroup/mini-docker"
	must(os.MkdirAll(path, 0755))
	must(os.WriteFile(filepath.Join(path, "cpu.max"), []byte("10000 100000"), 0644))
	must(os.WriteFile(filepath.Join(path, "cgroup.procs"), []byte("0"), 0644))
}

func pivotRoot(newRoot string) {
	must(syscall.Mount(newRoot, newRoot, "", syscall.MS_BIND|syscall.MS_REC, ""))
	oldRootFd, err := syscall.Open("/", syscall.O_DIRECTORY|syscall.O_RDONLY, 0)
	must(err)
	defer syscall.Close(oldRootFd)

	must(os.Chdir(newRoot))
	must(syscall.PivotRoot(".", "."))
	must(syscall.Fchdir(oldRootFd))
	must(syscall.Mount("", ".", "", syscall.MS_SLAVE|syscall.MS_REC, ""))
	must(syscall.Unmount(".", syscall.MNT_DETACH))
	must(os.Chdir("/"))
}

func must(err error) {
	if err != nil {
		fmt.Fprintf(os.Stderr, "\n[FATAL ERROR] %v\n", err)
		panic(err)
	}
}
