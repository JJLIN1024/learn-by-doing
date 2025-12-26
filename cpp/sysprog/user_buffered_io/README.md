## Introduction

Why is user-buffered I/O? It is implemented by standard library such as stdio.h or iostream, it will buffured the read or write request until the size of it is a multiple of block size, which is commonly 1024, 2048, ..., and so on. Because user's program typically will not read or write in block size, but string, or paragraph with varied size. So user-buffered I/O will bring huge performance gain by saving useless read or write request that has size not equal to block size.

User-buffered I/O are good, but with one flaw, that is, double copy of the data. When using say `fgets()`, the data is first copied from kernel buffer into standard I/O buffer(that's what `read()` system call does), and then `fgets()` will copy the data again from the standard I/O buffer into the user-defined buffer, such as `char buf[size]`.

So, it is the programmer's responsiblity to weight in these trade off between the overhead of potentially many system call, or the overhead of double copy of data.

Other take aways are:

1. standard I/O functions are thread-safe, each of which will obtain a lock internally, associated with the FILE stream it works on.
2. programmer can still use function like `fgets_unlocked()`, which is the no lock version of  `fgets()`, and of course, is not thread-safe by default.
3. If you need atomicity that spans over multiple std I/O functions, use `flockfile` and `funlockfile`. BTW, `ftrylockfile` is the non-blocking version of `flockfile`.
