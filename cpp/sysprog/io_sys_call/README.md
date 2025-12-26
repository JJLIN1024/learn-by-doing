## Basic I/O System Call

- `write()` & `read()` is just `writev()` and `readv()` but with only one segment, in other words, they are both vectored I/O.
