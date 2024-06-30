## I/O Model

- [Jserv - 事件驅動伺服器：原理和實例](https://hackmd.io/@sysprog/linux-io-model/https%3A%2F%2Fhackmd.io%2F%40sysprog%2Fevent-driven-server)

## select v.s. poll v.s. epoll
- poll is better than select but less portable
    - does not have to reconstruct timeout structure
    - more efficient when waiting on sparse fds
- epoll
    

## Usage

1. `make`
2. `./select`
3. `./poll` and `./poll < /dev/null`
