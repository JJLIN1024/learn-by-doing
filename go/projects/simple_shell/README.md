# Go-Shell

A lightweight, experimental shell implemented in Go.

---

## Roadmap

### Phase 1: Foundations (Core Infrastructure)
- [x] Basic REPL (Read-Eval-Print Loop)
- [x] Command execution using `os/exec`
- [ ] Implement Built-in commands:
    - `cd`: Support directory changing (current implementation spawns a subshell which can't change parent's PWD).
    - `exit`: Graceful shutdown.
    - `export`: Environment variable management.
- [ ] Proper argument parsing (handling quotes like `echo "hello world"`).

### Phase 2: Interactive Terminal (The "Vim" Milestone)
- [ ] Integrate `github.com/creack/pty` to support interactive TUI applications.
- [ ] Terminal Raw Mode: Transfer terminal control to the child process.
- [ ] Window Resizing: Listen for `SIGWINCH` and propagate size changes to the PTY.

### Phase 3: Data Plumbing (I/O Redirection)
- [ ] Implement Pipelines (`|`) using `io.Pipe`.
- [ ] Input/Output Redirection:
    - `>` : Truncate and write to file.
    - `>>`: Append to file.
    - `<` : Read from file.

### Phase 4: Job Control & Signals
- [ ] Signal Handling: Trap `Ctrl+C` (SIGINT) to interrupt child processes without killing the shell.
- [ ] Background Execution: Support the `&` operator to run commands in the background.
- [ ] Process Groups: Assign PGIDs to manage command groups effectively.
---

## Getting Started

### Prerequisites
- Go 1.21+
- Unix-like environment (Linux / macOS)

### Running the Shell
```bash
go run main.go