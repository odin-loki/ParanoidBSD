/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * sys/dev/acpica/acpi_pxm.c's cpu_add() and cpu_find(), which take an
 * APIC ID out of the firmware's SRAT and index cpus[] with it.
 *
 *   cbmc -DOLD --unwind 4 --signed-overflow-check --conversion-check \
 *       tools/verify/probes/acpi_pxm_cpuid.c        -> FAILED
 *   cbmc        --unwind 4 --signed-overflow-check --conversion-check \
 *       tools/verify/probes/acpi_pxm_cpuid.c        -> SUCCESSFUL
 *
 * The SRAT's X2APIC affinity entry carries its ApicId as a UINT32 and
 * both functions take an `int cpuid'.  On every architecture but arm64
 * cpus_use_indexing is 1, so the ID is the subscript:
 *
 *	if (cpuid >= max_cpus)
 *		return (NULL);
 *	last_cpu = imax(last_cpu, cpuid);
 *	cpup = &cpus[cpuid];
 *
 * One comparison, for a number with two ends.  An ApicId at or above
 * 0x80000000 converts to a negative int, passes `>= max_cpus', leaves
 * last_cpu alone because imax() keeps the larger, and addresses up to
 * two billion entries BEFORE the mapping pmap_mapbios() returned.  The
 * caller then writes through it -- domain, id and enabled.
 *
 * This is not only the adversarial case.  ACPI spells an unused
 * processor UID 0xFFFFFFFF, which is (int)-1, so cpus[-1] is what a
 * table with that entry and the ENABLED flag set already asks for.
 *
 * cpu_find() has the mirror of it, `cpuid <= last_cpu', which reads
 * rather than writes, and is reached from the exported
 * acpi_pxm_get_cpu_locality().
 *
 * The fix is the lower half of each bound.  cpu_add() returning NULL
 * for an out-of-range ID is the path that already exists: the caller
 * prints "Ignoring local APIC ID %u (too high)" and carries on.
 */

/*
 * cpuid is modelled as an arbitrary int rather than as (int)UINT32,
 * because that conversion is exactly onto the whole int range and
 * modelling it as a conversion only adds --conversion-check noise to
 * both sides.  The set of values the parameter can hold is the point.
 */
int nondet_int(void);

#define MAXCPUS	64

/* imax(), as sys/sys/libkern.h defines it. */
static int
imax(int a, int b)
{
	return (a > b ? a : b);
}

int
main(void)
{
	int max_cpus = MAXCPUS;
	int last_cpu = -1;
	int cpuid, idx;

	/* SRAT hands cpu_add() a UINT32 through an int parameter. */
	cpuid = nondet_int();

#ifdef OLD
	if (cpuid >= max_cpus)
		return (0);
#else
	if (cpuid < 0 || cpuid >= max_cpus)
		return (0);
#endif
	last_cpu = imax(last_cpu, cpuid);
	idx = cpuid;

	/* &cpus[idx], and the caller writes through it. */
	__CPROVER_assert(idx >= 0 && idx < max_cpus,
	    "cpu_add() subscripts cpus[] inside the mapping");

	/* cpu_find(), on the same ID, once last_cpu has moved. */
#ifdef OLD
	if (cpuid <= last_cpu)
#else
	if (cpuid >= 0 && cpuid <= last_cpu)
#endif
		__CPROVER_assert(cpuid >= 0 && cpuid < max_cpus,
		    "cpu_find() subscripts cpus[] inside the mapping");

	return (0);
}
