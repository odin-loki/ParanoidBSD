/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/efi/boot1: BlockSize is an EFI_BLOCK_IO_MEDIA field boot1
 * divides by and validates nowhere.
 *
 *   cbmc -DOLD tools/verify/probes/boot1_media_blocksize.c
 *       -> 2 of 2 failed, FAILED
 *   cbmc       tools/verify/probes/boot1_media_blocksize.c
 *       -> 0 of 2, SUCCESSFUL
 *
 *	ufs_module.c:55
 *	    lba = lba / (devinfo->dev->Media->BlockSize / DEV_BSIZE);
 *	zfs_module.c:62
 *	    lba = off / devinfo->dev->Media->BlockSize;
 *	    remainder = off % devinfo->dev->Media->BlockSize;
 *
 * The loader does validate it - efipart.c:250-252 requires >= 512,
 * <= 65536 and a power of two - but boot1 is the program that runs
 * BEFORE the loader, finds a root filesystem and loads it, and it
 * takes the number as given.  Under DEV_BSIZE the UFS reader's inner
 * division is zero; at zero the ZFS reader divides by it directly.
 * On x86 that is #DE, in an environment with no handler for it.
 *
 * The assertion is that neither divisor is zero.
 */

#define	DEV_BSIZE	512

unsigned nondet_uint(void);

int
main(void)
{
	unsigned bs = nondet_uint();	/* Media->BlockSize, as given */
	unsigned long lba = 1, off = 4096;

#ifndef OLD
	if (bs < DEV_BSIZE)
		return (-1);
#endif
	__CPROVER_assert(bs / DEV_BSIZE != 0,
	    "the UFS reader's divisor is not zero");
	lba = lba / (bs / DEV_BSIZE);

#ifndef OLD
	if (bs == 0)
		return (-1);
#endif
	__CPROVER_assert(bs != 0, "the ZFS reader's divisor is not zero");
	off = off / bs;

	return ((int)(lba + off));
}
