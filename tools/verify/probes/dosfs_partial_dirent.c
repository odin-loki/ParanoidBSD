/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/libsa/dosfs.c: dos_readdir() tested only for a WHOLE unread
 * directory entry, so a PARTIAL one was parsed out of stack garbage.
 *
 *   cbmc -DOLD tools/verify/probes/dosfs_partial_dirent.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       tools/verify/probes/dosfs_partial_dirent.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	DOS_DIR dd;
 *	size_t res;
 *	...
 *	err = dos_read(fd, &dd, sizeof(dd), &res);
 *	if (err)
 *		return (err);
 *	if (res == sizeof(dd))
 *		return (ENOENT);
 *	if (dd.de.name[0] == 0)
 *		return (ENOENT);
 *
 * res is the RESIDUAL - dos_read()'s `*resid = nbyte - nb + cnt', the
 * bytes it could NOT read.  dos_read() clamps its transfer to
 * `size - f->offset', and size is fsize(), whose first line is
 *
 *	if (!(size = cv4(de->size)) && de->attr & FA_DIR) {
 *
 * so the cluster-chain length is used ONLY when the entry's own size
 * field is zero.  A directory entry with FA_DIR set and a NON-zero
 * size takes that field verbatim - four arbitrary bytes off the
 * medium, under no constraint to be a multiple of 32.  When
 * `size - f->offset' lands between 1 and 31, res is neither 0 nor
 * sizeof(dd), both of the old tests pass, and the tail of dd is
 * whatever the stack held.
 *
 * dd is a union: the code goes on to branch on dd.de.attr, dd.xde.seq
 * and dd.xde.chk, and to bcopy dd.xde.name1/2/3 into the dirent it
 * returns.  So the length field of a directory on a USB stick picks
 * how many bytes of the loader's stack are parsed as a filename and
 * handed back to whatever listed the directory.
 *
 * The assertion is that every byte of the entry the parser reads is a
 * byte the read filled.
 */

typedef unsigned long size_t;
#define	ENOENT	2
#define	DIRSZ	32

int nondet_int(void);
size_t nondet_size(void);

int
main(void)
{
	size_t res, filled;

	/*
	 * dos_read() with err == 0: the residual is anything from 0
	 * (a whole entry) to sizeof(dd) (nothing left to read).
	 */
	res = nondet_size();
	__CPROVER_assume(res <= DIRSZ);
	filled = DIRSZ - res;

#ifdef OLD
	if (res == DIRSZ)
		return (ENOENT);
#else
	if (res != 0)
		return (ENOENT);
#endif

	/* Past here the code reads all DIRSZ bytes of the entry. */
	__CPROVER_assert(filled == DIRSZ,
	    "every byte of the entry the parser reads is one the read filled");
	return (0);
}
