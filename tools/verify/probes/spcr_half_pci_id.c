/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/efi/loader/main.c: check_acpi_spcr() formats mm, rs and rw
 * into hw.uart.console on a path that never sets them.
 *
 *   cbmc -DOLD tools/verify/probes/spcr_half_pci_id.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       tools/verify/probes/spcr_half_pci_id.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	io = -1;
 *	pv = spcr->PciVendorId;
 *	pd = spcr->PciDeviceId;
 *	if (pv == 0xffff && pd == 0xffff) {
 *		if (spcr->SerialPort.SpaceId == 1)
 *			io = spcr->SerialPort.Address;
 *		else {
 *			mm = spcr->SerialPort.Address;
 *			rs = ffs(spcr->SerialPort.BitWidth) - 4;
 *			rw = acpi_uart_regionwidth(...);
 *		}
 *	} else {
 *		// XXX todo: bus:device:function + flags and segment
 *	}
 *	...
 *	if (io != -1)           asprintf(... io ...)
 *	else if (pv != 0xffff && pd != 0xffff)  asprintf(... pv, pd ...)
 *	else                    asprintf(... mm, rs, rw ...)
 *
 * The memory-mapped arm needs pv and pd BOTH 0xffff; the PCI arm needs
 * them BOTH not.  An SPCR that names one and not the other satisfies
 * neither, falls to the third asprintf(), and prints this frame's
 * stack into hw.uart.console - which is where the kernel then goes
 * looking for the UART.
 *
 * The SPCR is an ACPI fixed table: firmware data, not the loader's.
 *
 * The assertion is that every value formatted has been assigned.
 */

int nondet_int(void);

static int mm_set, rs_set, rw_set;

int
main(void)
{
	int io = -1;
	int pv = nondet_int(), pd = nondet_int();
	int space_id = nondet_int();

	__CPROVER_assume(pv == 0xffff || pv == 0x8086);
	__CPROVER_assume(pd == 0xffff || pd == 0x1234);

#ifndef OLD
	mm_set = rs_set = rw_set = 1;	/* mm = 0; rs = rw = 0; */
#endif
	if (pv == 0xffff && pd == 0xffff) {
		if (space_id == 1)
			io = 0x3f8;
		else
			mm_set = rs_set = rw_set = 1;
	}

	if (io != -1)
		return (0);
	if (pv != 0xffff && pd != 0xffff)
		return (0);
#ifndef OLD
	if (mm_set == 0)
		return (0);
#endif
	__CPROVER_assert(mm_set && rs_set && rw_set,
	    "every value formatted into hw.uart.console has been assigned");
	return (0);
}
