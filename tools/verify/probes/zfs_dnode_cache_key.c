/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/libsa/zfs/zfsimpl.c: dnode_read()'s one-block cache is keyed
 * on the ADDRESS of the caller's dnode_phys_t, and callers put that on
 * their stack.
 *
 *   cbmc -DOLD tools/verify/probes/zfs_dnode_cache_key.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       tools/verify/probes/zfs_dnode_cache_key.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	static const dnode_phys_t *dnode_cache_obj;
 *	static uint64_t dnode_cache_bn;
 *	static char *dnode_cache_buf;
 *	...
 *	if (dnode == dnode_cache_obj && bn == dnode_cache_bn)
 *		goto cached;
 *	...
 *	dnode_cache_obj = dnode;
 *	dnode_cache_bn = bn;
 *
 * zfs.c's zfs_lookup() and efi/boot1/zfs_module.c's load() both
 * declare `dnode_phys_t dn;' and pass &dn.  The pointer outlives the
 * frame, so the next caller at the same depth gets the same address
 * for a DIFFERENT object and the cache hits: dnode_cache_buf still
 * holds the previous file's block, and that is what comes back.
 * Nothing crashes and nothing is reported - the loader reads the
 * wrong bytes, which on this path is the kernel or a module.
 *
 * Modelled as a shadow identity: two objects that reuse one address
 * are distinguished by the id the cache does not look at.
 *
 * The assertion is that a cache hit returns the block of the object
 * that was cached.
 */

int nondet_int(void);

static long cache_addr;		/* dnode_cache_obj */
static long cache_bn;		/* dnode_cache_bn */
static int  cache_bp0;		/* dnode_cache_bp0 - the fix */
static int  cache_id;		/* shadow: whose block dnode_cache_buf holds */
static int  cache_valid;

static int
dnode_read(long addr, int id, int bp0, long bn)
{
	if (cache_valid && addr == cache_addr && bn == cache_bn
#ifndef OLD
	    && bp0 == cache_bp0
#endif
	    ) {
		__CPROVER_assert(cache_id == id,
		    "a cache hit returns the block of the object that was cached");
		return (cache_id);
	}
	cache_addr = addr;
	cache_bn = bn;
	cache_bp0 = bp0;
	cache_id = id;
	cache_valid = 1;
	return (id);
}

int
main(void)
{
	/*
	 * Two files read in turn from the same call depth: one stack
	 * address, two objects, two first block pointers.
	 */
	long addr = 0x7fff0000;

	dnode_read(addr, 1, 11, 0);
	dnode_read(addr, 2, 22, 0);
	return (0);
}
