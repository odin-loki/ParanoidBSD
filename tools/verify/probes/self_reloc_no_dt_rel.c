/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/common/self_reloc.c: a dynamic section with DT_RELSZ and no
 * DT_REL or DT_RELA leaves rel uninitialised, and the relocation loop
 * reads through it and writes through what it finds.
 *
 *   cbmc -DOLD --unwind 6 tools/verify/probes/self_reloc_no_dt_rel.c
 *       -> 2 of 2 failed, FAILED
 *   cbmc       --unwind 6 tools/verify/probes/self_reloc_no_dt_rel.c
 *       -> 0 of 2, SUCCESSFUL
 *
 *	ElfW_Rel *rel;
 *	...
 *	relsz = 0; relent = 0;
 *	for (dynp = dynamic; dynp->d_tag != DT_NULL; dynp++) {
 *		case DT_REL: case DT_RELA:  rel = ...; break;
 *		case DT_RELSZ: case DT_RELASZ:  relsz = ...; break;
 *		case DT_RELENT: case DT_RELAENT: relent = ...; break;
 *	}
 *	for (; relsz > 0; relsz -= relent) {
 *		switch (ELFW_R_TYPE(rel->r_info)) {
 *		case RELOC_TYPE_RELATIVE:
 *			newaddr = (Elf_Addr *)(rel->r_offset + baseaddr);
 *			*newaddr += baseaddr;
 *
 * relsz, relent and rel come from three independent DT_ entries and
 * nothing requires them to arrive together.  Only relsz and relent are
 * given a starting value; rel is not.  And relent == 0 with relsz > 0
 * is a loop that never decreases.
 *
 * This is the loader relocating ITSELF, before anything else runs, so
 * the input is its own ELF image rather than an attacker's - but a
 * truncated or half-written boot binary is an ordinary failure, and
 * the answer to it should not be writing through a stack word.
 *
 * The assertion is that the loop is entered only with a relocation
 * table that exists and an entry size that advances it.
 */

int nondet_int(void);
unsigned nondet_uint(void);

int
main(void)
{
	void *rel;
	unsigned relsz, relent;
	int have_rel = nondet_int();
	int iter = 0;

	relsz = 0;
	relent = 0;
	rel = 0;
	if (have_rel)
		rel = (void *)0x1000;
	relsz = nondet_uint();
	__CPROVER_assume(relsz <= 4);
	relent = nondet_uint();
	__CPROVER_assume(relent <= 2);

#ifndef OLD
	if (rel == 0 || relent == 0)
		return (0);
#else
	if (rel == 0)
		rel = (void *)0;	/* the uninitialised local */
#endif
	for (; relsz > 0; relsz -= relent) {
		__CPROVER_assert(rel != 0,
		    "the relocation table the loop reads through exists");
		__CPROVER_assert(relent != 0,
		    "the entry size advances the loop");
		if (++iter > 4)
			break;
	}
	return (0);
}
