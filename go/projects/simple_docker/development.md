# Docker Dev

## Simple Docker

### CLONE_NEWNS, CLONE_NEWUTS, CLONE_NEWPID

```go
package main

import (
	"fmt"
	"os"
	"os/exec"
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
	syscall.Sethostname([]byte("mini-docker"))

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
```

實驗看看：
```shell
jjlin@ubuntu:~/learn-by-doing/go/projects/simple_docker$ sudo go run main.go run /bin/bash
root@mini-docker:/home/jjlin/learn-by-doing/go/projects/simple_docker# uname -a
Linux mini-docker 6.17.8-orbstack-00308-g8f9c941121b1 #1 SMP PREEMPT Thu Nov 20 09:34:02 UTC 2025 x86_64 GNU/Linux
root@mini-docker:/home/jjlin/learn-by-doing/go/projects/simple_docker# hostname
mini-docker
root@mini-docker:/home/jjlin/learn-by-doing/go/projects/simple_docker# ps -ef
UID          PID    PPID  C STIME TTY          TIME CMD
root           1       0  0 12:39 ?        00:00:01 [rosetta] /sbin/init /sbin/init
root           8       1  0 12:39 ?        00:00:00 orbstack-agent: ubuntu
root         122       1  0 12:39 ?        00:00:00 [rosetta] /usr/lib/systemd/systemd-journald /usr/lib/systemd/systemd-journald
root         173       1  0 12:39 ?        00:00:00 [rosetta] /usr/lib/systemd/systemd-udevd /usr/lib/systemd/systemd-udevd
systemd+     181       1  0 12:39 ?        00:00:00 [rosetta] /usr/lib/systemd/systemd-networkd /usr/lib/systemd/systemd-networkd
root         254       1  0 12:39 ?        00:00:00 [rosetta] /usr/sbin/cron /usr/sbin/cron -f -P
message+     255       1  0 12:39 ?        00:00:00 [rosetta] /usr/bin/dbus-daemon @dbus-daemon --system --address=systemd: --nofork --nopi
root         259       1  0 12:39 ?        00:00:00 [rosetta] /usr/lib/systemd/systemd-logind /usr/lib/systemd/systemd-logind
syslog       464       1  0 12:39 ?        00:00:00 [rosetta] /usr/sbin/rsyslogd /usr/sbin/rsyslogd -n -iNONE
root         470       1  0 12:39 pts/0    00:00:00 [rosetta] /sbin/agetty /sbin/agetty -o -- \u --noreset --noclear --keep-baud 115200,576
jjlin       1999       8  0 12:40 ?        00:00:00 [rosetta] /bin/bash -bash
jjlin       2005       1  0 12:40 ?        00:00:00 [rosetta] /usr/lib/systemd/systemd /usr/lib/systemd/systemd --user
jjlin       2006    1999  0 12:40 ?        00:00:00 [rosetta] /usr/bin/sh sh
jjlin       2016    2005  0 12:40 ?        00:00:00 (sd-pam)
jjlin       2066    2006  0 12:40 ?        00:00:15 [rosetta] /home/jjlin/.vscode-server/code-994fd12f8d3a5aa16f17d42c041e5809167e845a /hom
jjlin       2116    2066  0 12:40 ?        00:00:00 [rosetta] /usr/bin/sh sh /home/jjlin/.vscode-server/cli/servers/Stable-994fd12f8d3a5aa1
jjlin       2120    2116  0 12:40 ?        00:00:25 [rosetta] /home/jjlin/.vscode-server/cli/servers/Stable-994fd12f8d3a5aa16f17d42c041e580
jjlin       2146    2120  0 12:40 ?        00:00:35 [rosetta] /home/jjlin/.vscode-server/cli/servers/Stable-994fd12f8d3a5aa16f17d42c041e580
jjlin       5381    2120  1 12:54 ?        00:00:50 [rosetta] /home/jjlin/.vscode-server/cli/servers/Stable-994fd12f8d3a5aa16f17d42c041e580
jjlin       5382    2120  0 12:54 ?        00:00:02 [rosetta] /home/jjlin/.vscode-server/cli/servers/Stable-994fd12f8d3a5aa16f17d42c041e580
jjlin       5410    5381  0 12:54 ?        00:00:01 [rosetta] /home/jjlin/.vscode-server/cli/servers/Stable-994fd12f8d3a5aa16f17d42c041e580
jjlin       6288    2146  0 12:58 pts/1    00:00:00 [rosetta] /bin/bash /bin/bash --init-file /home/jjlin/.vscode-server/cli/servers/Stable
jjlin      14243    5381  0 12:59 ?        00:00:26 [rosetta] /home/jjlin/go/bin/gopls /home/jjlin/go/bin/gopls
jjlin      14253   14243  0 12:59 ?        00:00:00 [rosetta] /home/jjlin/go/bin/gopls /home/jjlin/go/bin/gopls ** telemetry **
jjlin      15878    2146  0 13:08 pts/2    00:00:00 [rosetta] /bin/bash /bin/bash --init-file /home/jjlin/.vscode-server/cli/servers/Stable
jjlin      16119    5381  0 13:11 ?        00:00:01 [rosetta] /home/jjlin/.vscode-server/cli/servers/Stable-994fd12f8d3a5aa16f17d42c041e580
jjlin      18438    2146  0 13:19 pts/3    00:00:00 [rosetta] /bin/bash /bin/bash --init-file /home/jjlin/.vscode-server/cli/servers/Stable
jjlin      19516    2006  0 13:46 ?        00:00:00 [rosetta] /usr/bin/sleep sleep 180
root       19762    6288  0 13:48 pts/1    00:00:00 [rosetta] /usr/bin/sudo sudo go run main.go run /bin/bash
root       19764   19762  0 13:48 pts/4    00:00:00 [rosetta] /usr/bin/sudo sudo go run main.go run /bin/bash
root       19765   19764  1 13:48 pts/4    00:00:00 [rosetta] /usr/bin/go go run main.go run /bin/bash
root       19878   19765  0 13:48 pts/4    00:00:00 [rosetta] /root/.cache/go-build/7a/7a3df3fb1b47b56d2012b982c4c4684f02208931b3619fcd904d
root       19883   19878  0 13:48 pts/4    00:00:00 [rosetta] /root/.cache/go-build/7a/7a3df3fb1b47b56d2012b982c4c4684f02208931b3619fcd904d
root       19888   19883  0 13:48 pts/4    00:00:00 [rosetta] /bin/bash /bin/bash
root       20016   19888 83 13:48 pts/4    00:00:00 [rosetta] /usr/bin/ps ps -ef
root@mini-docker:/home/jjlin/learn-by-doing/go/projects/simple_docker# cat /proc/1/comm
systemd
```

hostname 雖然被隔離了，但是為何還是看到這麼多 process？ pid 1 竟然是 systemd ？

## chroot & rootfs

雖然我們隔離了 namespace(`CLONE_NEWNS `)，但就像 fork 後 child process 會繼承 parent process 的 fd 一樣，這裡的 child 也繼承了 parent `/proc` 當中的內容。

要做到真正的隔離，讓 child 的 `/proc` 有全新的內容，我們需要 `chroot` 以及 `rootfs`。

首先，下載 rootfs：

```shell
mkdir -p rootfs
curl -Lo alpine-rootfs.tar.gz https://dl-cdn.alpinelinux.org/alpine/v3.19/releases/x86_64/alpine-minirootfs-3.19.1-x86_64.tar.gz
tar -xvf alpine-rootfs.tar.gz -C ./rootfs
```

總大小才 8.6MB：
```shell
jjlin@ubuntu:~/learn-by-doing/go/projects/simple_docker$ du -sh ./rootfs/
8.6M    ./rootfs/
```

可觀察到它就是一個 User Space 檔案與目錄的集合：
```shell
jjlin@ubuntu:~/learn-by-doing/go/projects/simple_docker$ ll ./rootfs/
total 0
drwxr-xr-x 1 jjlin jjlin 114 Jan 27  2024 ./
drwxr-xr-x 1 jjlin jjlin  44 Dec 27 14:43 ../
drwxr-xr-x 1 jjlin jjlin 858 Jan 27  2024 bin/
drwxr-xr-x 1 jjlin jjlin   0 Jan 27  2024 dev/
drwxr-xr-x 1 jjlin jjlin 586 Jan 27  2024 etc/
drwxr-xr-x 1 jjlin jjlin   0 Jan 27  2024 home/
drwxr-xr-x 1 jjlin jjlin 280 Jan 27  2024 lib/
drwxr-xr-x 1 jjlin jjlin  28 Jan 27  2024 media/
drwxr-xr-x 1 jjlin jjlin   0 Jan 27  2024 mnt/
drwxr-xr-x 1 jjlin jjlin   0 Jan 27  2024 opt/
dr-xr-xr-x 1 jjlin jjlin   0 Jan 27  2024 proc/
drwx------ 1 jjlin jjlin   0 Jan 27  2024 root/
drwxr-xr-x 1 jjlin jjlin   0 Jan 27  2024 run/
drwxr-xr-x 1 jjlin jjlin 790 Jan 27  2024 sbin/
drwxr-xr-x 1 jjlin jjlin   0 Jan 27  2024 srv/
drwxr-xr-x 1 jjlin jjlin   0 Jan 27  2024 sys/
drwxr-xr-x 1 jjlin jjlin   0 Jan 27  2024 tmp/
drwxr-xr-x 1 jjlin jjlin  40 Jan 27  2024 usr/
drwxr-xr-x 1 jjlin jjlin  86 Jan 27  2024 var/
```

可觀察到裡面使用了 busybox(所以才會 size 才會只有幾 MB)：
```shell
jjlin@ubuntu:~/learn-by-doing/go/projects/simple_docker$ tree -L 2 ./rootfs/
./rootfs/
├── bin
│   ├── arch -> /bin/busybox
│   ├── ash -> /bin/busybox
│   ├── base64 -> /bin/busybox
│   ├── bbconfig -> /bin/busybox
│   ├── busybox
│   ├── cat -> /bin/busybox
│   ├── chattr -> /bin/busybox
│   ├── chgrp -> /bin/busybox
│   ├── chmod -> /bin/busybox
```

rootfs 準備好之後，我們來改 child 的邏輯：chroot 到 `rootfs`，然後 cd 到 `/`，接著 mount child 的 `proc` 到 `/`（rootfs） 的 `/proc`

```go
func child() {
	syscall.Sethostname([]byte("mini-docker"))

	must(syscall.Chroot("./rootfs"))
	must(os.Chdir("/"))

	must(syscall.Mount("proc", "/proc", "proc", syscall.MS_RDONLY | syscall.MS_NOSUID | syscall.MS_NOEXEC, "")) // source, target, fstype, flags, args

	cmd := exec.Command(os.Args[2], os.Args[3:]...)
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	if err := cmd.Run(); err != nil {
		fmt.Println("ERROR", err)
		os.Exit(1)
	}
}
```

測試看看，注意，啟動的 shell 改成了 `/bin/sh`，因為 `rootfs` 裡面沒有 `/bin/bash`。

```shell
jjlin@ubuntu:~/learn-by-doing/go/projects/simple_docker$ sudo go run main.go run /bin/sh
/ # whoami
root
/ # hostname
mini-docker
/ # ps -ef
PID   USER     TIME  COMMAND
    1 root      0:00 {main} /proc/self/exe child /bin/sh
    6 root      0:00 /bin/sh
    9 root      0:00 ps -ef
/ # ls
bin    dev    etc    home   lib    media  mnt    opt    proc   root   run    sbin   srv    sys    tmp    usr    var
```




