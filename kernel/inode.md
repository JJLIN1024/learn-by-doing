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

各個檔案系統（e.g. ext4, btrfs, ...）各自實作。

```c
struct inode_operations {
	struct dentry * (*lookup) (struct inode *,struct dentry *, unsigned int);
	const char * (*get_link) (struct dentry *, struct inode *, struct delayed_call *);
	int (*permission) (struct mnt_idmap *, struct inode *, int);
	struct posix_acl * (*get_inode_acl)(struct inode *, int, bool);

	int (*readlink) (struct dentry *, char __user *,int);

	int (*create) (struct mnt_idmap *, struct inode *,struct dentry *,
		       umode_t, bool);
	int (*link) (struct dentry *,struct inode *,struct dentry *);
	int (*unlink) (struct inode *,struct dentry *);
	int (*symlink) (struct mnt_idmap *, struct inode *,struct dentry *,
			const char *);
	struct dentry *(*mkdir) (struct mnt_idmap *, struct inode *,
				 struct dentry *, umode_t);
	int (*rmdir) (struct inode *,struct dentry *);
	int (*mknod) (struct mnt_idmap *, struct inode *,struct dentry *,
		      umode_t,dev_t);
	int (*rename) (struct mnt_idmap *, struct inode *, struct dentry *,
			struct inode *, struct dentry *, unsigned int);
	int (*setattr) (struct mnt_idmap *, struct dentry *, struct iattr *);
	int (*getattr) (struct mnt_idmap *, const struct path *,
			struct kstat *, u32, unsigned int);
// ...
}
```

## super_block 



## address_space


```c
/**
 * struct address_space - Contents of a cacheable, mappable object.
 * @host: Owner, either the inode or the block_device.
 * @i_pages: Cached pages.
 * @invalidate_lock: Guards coherency between page cache contents and
 *   file offset->disk block mappings in the filesystem during invalidates.
 *   It is also used to block modification of page cache contents through
 *   memory mappings.
 * @gfp_mask: Memory allocation flags to use for allocating pages.
 * @i_mmap_writable: Number of VM_SHARED, VM_MAYWRITE mappings.
 * @nr_thps: Number of THPs in the pagecache (non-shmem only).
 * @i_mmap: Tree of private and shared mappings.
 * @i_mmap_rwsem: Protects @i_mmap and @i_mmap_writable.
 * @nrpages: Number of page entries, protected by the i_pages lock.
 * @writeback_index: Writeback starts here.
 * @a_ops: Methods.
 * @flags: Error bits and flags (AS_*).
 * @wb_err: The most recent error which has occurred.
 * @i_private_lock: For use by the owner of the address_space.
 * @i_private_list: For use by the owner of the address_space.
 * @i_private_data: For use by the owner of the address_space.
 */
struct address_space {
	struct inode		*host;
	struct xarray		i_pages;
	struct rw_semaphore	invalidate_lock;
	gfp_t			gfp_mask;
	atomic_t		i_mmap_writable;
#ifdef CONFIG_READ_ONLY_THP_FOR_FS
	/* number of thp, only for non-shmem files */
	atomic_t		nr_thps;
#endif
	struct rb_root_cached	i_mmap;
	unsigned long		nrpages;
	pgoff_t			writeback_index;
	const struct address_space_operations *a_ops;
	unsigned long		flags;
	errseq_t		wb_err;
	spinlock_t		i_private_lock;
	struct list_head	i_private_list;
	struct rw_semaphore	i_mmap_rwsem;
	void *			i_private_data;
} __attribute__((aligned(sizeof(long)))) __randomize_layout;
```
