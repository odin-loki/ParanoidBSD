/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/efi/loader/main.c: parse_uefi_con_out() walks the device path
 * in the firmware's ConOut variable, testing only that a node BEGINS
 * inside the buffer and advancing by a Length from the same buffer.
 *
 *   cbmc -DOLD --unwind 8 tools/verify/probes/uefi_devpath_walk.c
 *       -> 2 of 2 failed, FAILED
 *   cbmc       --unwind 8 tools/verify/probes/uefi_devpath_walk.c
 *       -> 0 of 2, SUCCESSFUL
 *
 *	ep = buf + sz;
 *	node = (EFI_DEVICE_PATH *)buf;
 *	while ((char *)node < ep) {
 *		...
 *		if (DevicePathType(node) == ACPI_DEVICE_PATH && ...) {
 *			acpi = (void *)node;
 *			if (EISA_ID_TO_NUM(acpi->HID) == 0x501) {
 *				setenv_int("efi_8250_uid", acpi->UID);
 *	...
 *		node = NextDevicePathNode(node);
 *	}
 *
 * NextDevicePathNode() is `(UINT8 *)a + DevicePathNodeLength(a)', and
 * DevicePathNodeLength() reads the node's own two Length bytes.  A
 * Length of 0 never advances, so the loop does not terminate.  A node
 * beginning one byte under ep has its type, subtype and - for an ACPI
 * or UART node - a dozen further bytes read past the end of what the
 * variable actually held.
 *
 * ConOut is a UEFI global variable: firmware data, read before the
 * kernel exists.
 *
 * The assertion is that every byte read lies inside the buffer and
 * that the walk advances.
 */

#define	NODE_HDR	4		/* sizeof(EFI_DEVICE_PATH) */

unsigned nondet_uint(void);

static unsigned buflen;			/* sz */
static unsigned length_of[8];		/* each node's Length field */

int
main(void)
{
	unsigned off, i, nlen, prev;

	buflen = nondet_uint();
	__CPROVER_assume(buflen > 0 && buflen <= 64);
	for (i = 0; i < 8; i++) {
		length_of[i] = nondet_uint();
		__CPROVER_assume(length_of[i] <= 64);
	}

	off = 0;
	i = 0;
#ifdef OLD
	while (off < buflen) {
#else
	while (off + NODE_HDR <= buflen) {
#endif
		nlen = length_of[i & 7];
#ifndef OLD
		if (nlen < NODE_HDR || off + nlen > buflen)
			break;
#endif
		__CPROVER_assert(off + NODE_HDR <= buflen,
		    "the node header the walk reads lies inside the buffer");
		prev = off;
		off += nlen;
		__CPROVER_assert(off > prev, "the walk advances");
		i++;
		if (i > 6)
			break;
	}
	return (0);
}
