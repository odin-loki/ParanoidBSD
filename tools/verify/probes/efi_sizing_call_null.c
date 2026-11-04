/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/efi: five places call an EFI service with a NULL buffer to
 * learn the size, and then use the buffer without checking that the
 * second call happened.
 *
 *   cbmc -DOLD tools/verify/probes/efi_sizing_call_null.c
 *       -> 2 of 2 failed, FAILED
 *   cbmc       tools/verify/probes/efi_sizing_call_null.c
 *       -> 0 of 2, SUCCESSFUL
 *
 *	hlist = NULL;
 *	status = BS->LocateHandle(ByProtocol, &gop_guid, NULL, &hsize, hlist);
 *	if (status == EFI_BUFFER_TOO_SMALL) {
 *		hlist = malloc(hsize);
 *		...
 *	}
 *	...
 *	nhandles = hsize / sizeof(*hlist);
 *	for (i = 0; i < nhandles; i++)
 *		... hlist[i] ...
 *
 * libefi/efinet.c:388, libefi/efipart.c:385, loader/framebuffer.c:625,
 * loader/copy.c:129 and loader/bootinfo.c:320 are the same shape, the
 * last two around GetMemoryMap() rather than LocateHandle().
 *
 * The UEFI specification says a NULL buffer cannot come back
 * EFI_SUCCESS with a non-zero size, so the second call always
 * happens.  But the status and the size are BOTH the firmware's, and
 * the code checks neither against the other - the same trade as
 * boot1's unvalidated Media->BlockSize, one layer up.  A NULL that
 * reaches the loop is indexed; in copy.c the size is also a divisor
 * and in bootinfo.c the pointer is written through.
 *
 * The assertion is that the buffer is non-NULL and the element size
 * non-zero wherever they are used.
 */

#define	EFI_SUCCESS		0
#define	EFI_BUFFER_TOO_SMALL	5

int nondet_int(void);
unsigned nondet_uint(void);

int
main(void)
{
	void *buf = 0;
	unsigned sz = 0, dsz = 0;
	int status;

	/* The sizing call, with a NULL buffer. */
	status = nondet_int() ? EFI_BUFFER_TOO_SMALL : EFI_SUCCESS;
	if (status == EFI_BUFFER_TOO_SMALL) {
		sz = nondet_uint();
		__CPROVER_assume(sz > 0 && sz < 4096);
		dsz = 8;
		buf = (void *)0x2000;	/* malloc(sz) */
		status = EFI_SUCCESS;
	} else {
		/*
		 * Firmware that answers SUCCESS to the sizing call. The
		 * specification says sz stays 0 here; nothing checks it.
		 */
		sz = nondet_uint();
		__CPROVER_assume(sz < 4096);
	}
	if (status != EFI_SUCCESS)
		return (1);

#ifndef OLD
	if (buf == 0 || dsz == 0)
		return (1);
#endif
	__CPROVER_assert(buf != 0,
	    "the buffer indexed by the loop is one something allocated");
	__CPROVER_assert(dsz != 0,
	    "the element size the count divides by is not zero");
	return ((int)(sz / dsz));
}
