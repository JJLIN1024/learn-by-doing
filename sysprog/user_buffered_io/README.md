## Introduction

Why is user-buffered I/O? It is implemented by standard library such as stdio.h or iostream, it will buffured the read or write request until the size of it is a multiple of block size, which is commonly 1024, 2048, ..., and so on. Because user's program typically will not read or write in block size, but string, or paragraph with varied size. So user-buffered I/O will bring huge performance gain by saving useless read or write request that has size not equal to block size.
