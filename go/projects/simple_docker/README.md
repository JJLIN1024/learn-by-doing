# Simple Container

A minimalist container runtime implementation in Go. This project demystifies the "magic" behind Docker by interacting directly with Linux Kernel features: **Namespaces**, **Cgroups**, and **Layered Filesystems**.

---

## Roadmap

- [x] Implement the "Re-execution" pattern (`/proc/self/exe`) to fork the Go process.
- [x] Enable **Namespaces** via `syscall.SysProcAttr`:
    - `CLONE_NEWUTS`: Hostname isolation.
    - `CLONE_NEWPID`: Process ID isolation (Process becomes PID 1).
    - `CLONE_NEWNS`: Mount namespace isolation.
- [x] Verify isolation by running `ps` inside the container (should see only a few processes).
- [x] Prepare a minimal RootFS (e.g., Alpine Linux export).
- [x] Implement `pivot_root` syscall to swap the OS root directory.
- [x] Create a cgroup hierarchy for the container (`/sys/fs/cgroup/my-container`)
    - [ ] explore linux kernel cgroup struct and its mechanism
- [x] Implement hardware limits:
    - **CPU:** Limit CPU shares/quota.
- [ ] Create a `veth` pair (Virtual Ethernet) connecting Host and Container.
- [ ] Move one end into the Container's Network Namespace (`CLONE_NEWNET`).
- [ ] Assign IP addresses and set up `iptables` NAT for external access.
- [x] 建立 lowerdir (唯讀的 Base Image), upperdir (可寫層), workdir (OverlayFS 運作所需)。將它們合併掛載到 ./rootfs。
    - [ ] explore linux kernel overlay module and its mechanism
