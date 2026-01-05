
- [source](https://elixir.bootlin.com/linux/v6.18.2/source/include/linux/dcache.h#L92)

```c
struct dentry {
	/* RCU lookup touched fields */
	unsigned int d_flags;		/* protected by d_lock */
	seqcount_spinlock_t d_seq;	/* per dentry seqlock */
	struct hlist_bl_node d_hash;	/* lookup hash list */
	struct dentry *d_parent;	/* parent directory */
	union {
	struct qstr __d_name;		/* for use ONLY in fs/dcache.
	const struct qstr d_name;
	};
	struct inode *d_inode;		/* Where the name belongs to - NULL is
					 * negative */
	union shortname_store d_shortname;
	/* --- cacheline 1 boundary (64 bytes) was 32 bytes ago --- */

	/* Ref lookup also touches following */
	const struct dentry_operations *d_op;
	struct super_block *d_sb;	/* The root of the dentry tree */
	unsigned long d_time;		/* used by d_revalidate */
	void *d_fsdata;			/* fs-specific data */
	/* --- cacheline 2 boundary (128 bytes) --- */
	struct lockref d_lockref;	/* per-dentry lock and refcount
					 * keep separate from RCU lookup area if
					 * possible!
					 */

	union {
		struct list_head d_lru;		/* LRU list */
		wait_queue_head_t *d_wait;	/* in-lookup ones only */
	};
	struct hlist_node d_sib;	/* child of parent list */
	struct hlist_head d_children;	/* our children */
	/*
	 * d_alias and d_rcu can share memory
	 */
	union {
		struct hlist_node d_alias;	/* inode alias list */
		struct hlist_bl_node d_in_lookup_hash;	/* only for in-lookup ones */
	 	struct rcu_head d_rcu;
	} d_u;
};
```