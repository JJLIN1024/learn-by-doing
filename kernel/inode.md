# struct inode

## Reference

- [Linux 核心原始程式碼巨集: __randomize_layout](https://hackmd.io/@sysprog/linux-macro-randstruct)
- [linux/fs.h: struct inode](https://elixir.bootlin.com/linux/v6.18.2/source/include/linux/fs.h#L793)

## Analysis

```c
/*
 * Keep mostly read-only and often accessed (especially for
 * the RCU path lookup and 'stat' data) fields at the beginning
 * of the 'struct inode'
 */
struct inode {
	umode_t			i_mode;
	unsigned short		i_opflags;
	kuid_t			i_uid;
	kgid_t			i_gid;
	unsigned int		i_flags;
// ...
	const struct inode_operations	*i_op;
	struct super_block	*i_sb;
	struct address_space	*i_mapping;
// ...
} __randomize_layout;
```

### Cache and __randomize_layout

首先看到開頭的 comments：

```
Keep mostly read-only and often accessed (especially for the RCU path lookup and 'stat' data) fields at the beginning of the 'struct inode'
```
> 在現代的主流處理器（x86_64, ARM64）上，Cache Line 的大小通常是 64 Bytes

代表 kernel 認為 `stat` 以及各種 `inode_operations` 會最常被存取。所以將它們都放在相鄰的位置以避免 cache miss。但是在整個 struct 的結尾又放了一個 `__randomize_layout`，代表 `struct inode` 的各個 member 的位置是會被隨機擺放的，這樣不就沒辦法避免 cache miss 了嗎？

其實不然，主流的 linux ditribution 預設是都不會在編譯時開啟 `__randomize_layout` 的，這是以資安風險為代價，換取了維護的方便性，以及避免 ABI break。參考 Jserv 的說明：

> 由於維護核心模組的額外成本 (發行者要公開自己使用的 seed，允許第三方核心模組在開啟的 RANDSTRUCT 核心上運行)、性能和相容性的議題，若干 Linux 系統發行商 (distribution) 不傾向啟用。實際上能夠 RANDSTRUCT 受益的，主要是 Google, Meta, Amazon 這類雲端服務提供商。

`Google` 這些巨頭因為伺服器核心、驅動、應用程式全是自己編譯的，他們不需要考慮「第三方驅動相容性」，所以可以放心的開啟 `RANDSTRUCT` 這個編譯選項。

### mode, permission, flags
```c
typedef struct {
	uid_t val;
} kuid_t;

typedef struct {
	gid_t val;
} kgid_t;

```

```c
// ...
	umode_t			i_mode;
	unsigned short		i_opflags;
	kuid_t			i_uid;
	kgid_t			i_gid;
	unsigned int		i_flags;
// ...
```

- `i_mode`: Permissions(`rwxrwxrwx`) + Special Bits(`SUID`, `SGID`, `Sticky Bit`) + File Type (e.g. `S_IFREG`, `S_IFDIR`, `S_IFLNK`)
    - `chmod 4755 test.txt` 改動的就是 `SUID` + Permissions，`chmod 1777 /tmp` 改動的就是 `Sticky Bit` + Permissions
    - `Sticky Bit` 使得只有檔案、目錄的擁有者以及 root 可以刪除或更名檔案
    - `SUID`, `SGID` 最高只能提權到 `i_uid`, `i_gid`
- `i_opflags`: e.g. `IOP_LOOKUP`, `IOP_NOFOLLOW`
    - 由 kernel 根據 `inode_operations` 去設定
- `i_uid`, `i_gid`: In-memory User ID & Group ID
    - `kuid_t`, `kgid_t`: k means "kernel"，迫使開發者顯示轉型
- `i_flags`: `APPEND`, `IMMUTABLE`, `SYNC`
    - `i_mode` 是 Unix 標準權限，而 `i_flags` 是檔案系統層級的進階行為控制
    - 透過 `ioctl` 或是 `chattr` 來設定


## inode_operations

## super_block 


## address_space


