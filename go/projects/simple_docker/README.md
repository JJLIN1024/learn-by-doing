# Simple Container

A minimalist container runtime implementation in Go. This project demystifies the "magic" behind Docker by interacting directly with Linux Kernel features: **Namespaces**, **Cgroups**, and **Layered Filesystems**.

---

## Roadmap

### Phase 1: The Skeleton (Process Isolation)
- [x] Implement the "Re-execution" pattern (`/proc/self/exe`) to fork the Go process.
- [x] Enable **Namespaces** via `syscall.SysProcAttr`:
    - `CLONE_NEWUTS`: Hostname isolation.
    - `CLONE_NEWPID`: Process ID isolation (Process becomes PID 1).
    - `CLONE_NEWNS`: Mount namespace isolation.
- [x] Verify isolation by running `ps` inside the container (should see only a few processes).

### Phase 2: The Filesystem (RootFS)
- [x] Prepare a minimal RootFS (e.g., Alpine Linux export).
- [ ] Implement `pivot_root` syscall to swap the OS root directory.
- [ ] **TODO:** Mount critical virtual filesystems:
    - `mount -t proc proc /proc` (Required for `ps` to work).
    - `mount -t tmpfs tmpfs /dev`

### Phase 3: Resource Control (Cgroups)
- [ ] **TODO:** Mount the cgroup V2 filesystem.
- [ ] **TODO:** Create a cgroup hierarchy for the container (`/sys/fs/cgroup/my-container`).
- [ ] **TODO:** Implement hardware limits:
    - **Memory:** Limit max RAM usage to prevent OOM on host.
    - **CPU:** Limit CPU shares/quota.
    - **PIDs:** Limit maximum number of processes to prevent fork bombs.

### Phase 4: Networking (The "Plumbing")
- [ ] **TODO:** Create a `veth` pair (Virtual Ethernet) connecting Host and Container.
- [ ] **TODO:** Move one end into the Container's Network Namespace (`CLONE_NEWNET`).
- [ ] **TODO:** Assign IP addresses and set up `iptables` NAT for external access.

### Phase 5: Observability (System Metrics)
- [ ] **TODO:** **Metrics Exporter:** Read from `/sys/fs/cgroup/...` to show real-time CPU/Mem usage.
- [ ] **TODO:** **Syscall Tracking:** Use `ptrace` or eBPF to monitor container activity.
---

## Getting Started

### Prerequisites
* **OS:** Linux (Kernel 4.0+) **Required**. (Relies on Linux-specific syscalls).
* **Go:** 1.21+
* **Privilege:** Root access is required for namespace manipulation.

### Installation & Run

1. **Prepare RootFS**
   ```bash
   mkdir rootfs
   docker export $(docker create alpine) | tar -C rootfs -xvf -