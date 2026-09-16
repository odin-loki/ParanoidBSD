/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/efi/libefi/time.c: to_efi_time()'s month loop advances
 * efi_time->Month without advancing anything its own condition reads.
 *
 *   cbmc -DOLD --unwind 16 tools/verify/probes/efi_to_efi_time_month.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       --unwind 16 tools/verify/probes/efi_to_efi_time_month.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	month = 13;
 *	seconds = CumulativeDays[lyear][month] * SECSPERDAY;
 *	while (time > seconds) { ... }		<- brings time under a year
 *
 *	efi_time->Month = 0;
 *	while (time > CumulativeDays[lyear][month] * SECSPERDAY) {
 *		efi_time->Month++;
 *	}
 *	month = efi_time->Month - 1;
 *	time -= CumulativeDays[lyear][month] * SECSPERDAY;
 *
 * month is still 13 - the whole year - and the loop above already
 * brought time under that, so the second loop never runs, Month stays
 * 0, and the subscript is CumulativeDays[lyear][-1]: one time_t
 * before the array.  (Had the condition ever been true it would also
 * never terminate, because the body changes nothing it reads.)
 *
 * Nothing in this tree calls to_efi_time(); it is an exported
 * function of libefi with no caller, which is why nobody noticed.
 *
 * The assertion is that the subscript is one CumulativeDays has.
 */

#define	SECSPERDAY	86400

/* CumulativeDays[lyear][m]: days before month m, m in 1..12; [13] the year. */
static const long CumulativeDays[14] = {
	0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

long nondet_long(void);

int
main(void)
{
	long time = nondet_long();
	int Month, month;

	/* After the year loop: 0 <= time <= one year. */
	__CPROVER_assume(time >= 0 &&
	    time <= CumulativeDays[13] * (long)SECSPERDAY);

#ifdef OLD
	month = 13;
	Month = 0;
	while (time > CumulativeDays[month] * (long)SECSPERDAY)
		Month++;
	month = Month - 1;
#else
	Month = 1;
	while (Month < 12 &&
	    time >= CumulativeDays[Month + 1] * (long)SECSPERDAY)
		Month++;
	month = Month;
#endif
	__CPROVER_assert(month >= 0 && month <= 13,
	    "the subscript is one CumulativeDays has");
	return ((int)CumulativeDays[month]);
}
