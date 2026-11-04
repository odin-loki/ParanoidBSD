/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <libelf.h>
#include <link.h>
#include <elf.h>
#include <gelf.h>
#ifdef illumos
#include <sys/machelf.h>

#include <kstat.h>
#else
#include <sys/elf.h>
#include <sys/param.h>
#include <sys/module.h>
#include <sys/linker.h>
#endif
#include <sys/cpuvar.h>

typedef struct syment {
	uintptr_t	addr;
	char		*name;
	size_t		size;
} syment_t;

static syment_t *symbol_table;
static int nsyms, maxsyms;
static char maxsymname[64];

#ifdef illumos
#ifdef _ELF64
#define	elf_getshdr elf64_getshdr
#else
#define	elf_getshdr elf32_getshdr
#endif
#endif

static void
add_symbol(char *name, uintptr_t addr, size_t size)
{
	syment_t *sep;

	if (nsyms >= maxsyms) {
		maxsyms += 10000;
		symbol_table = realloc(symbol_table, maxsyms * sizeof (*sep));
		if (symbol_table == NULL) {
			(void) fprintf(stderr, "can't allocate symbol table\n");
			exit(3);
		}
	}
	sep = &symbol_table[nsyms++];

	sep->name = name;
	sep->addr = addr;
	sep->size = size;
}

static void
remove_symbol(uintptr_t addr)
{
	int i;
	syment_t *sep = symbol_table;

	for (i = 0; i < nsyms; i++, sep++)
		if (sep->addr == addr)
			sep->addr = 0;
}

#ifdef illumos
static void
fake_up_certain_popular_kernel_symbols(void)
{
	kstat_ctl_t *kc;
	kstat_t *ksp;
	char *name;

	if ((kc = kstat_open()) == NULL)
		return;

	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
		if (strcmp(ksp->ks_module, "cpu_info") == 0) {
			if ((name = malloc(20)) == NULL)
				break;
			/*
			 * For consistency, keep cpu[0] and toss cpu0
			 * or any other such symbols.
			 */
			if (ksp->ks_instance == 0)
				remove_symbol((uintptr_t)ksp->ks_private);
			(void) sprintf(name, "cpu[%d]", ksp->ks_instance);
			add_symbol(name, (uintptr_t)ksp->ks_private,
			    sizeof (struct cpu));
		}
	}
	(void) kstat_close(kc);
}
#else /* !illumos */
static void
fake_up_certain_popular_kernel_symbols(void)
{
	char *name;
	uintptr_t addr;
	int i;

	/* Good for up to 256 CPUs */
	for(i=0; i < 256;  i++) {
		if ((name = malloc(20)) == NULL)
			break;
		(void) sprintf(name, "cpu[%d]", i);
		addr = 0x01000000 + (i << 16); 
		add_symbol(name, addr, sizeof (uintptr_t));
	}
}
#endif /* illumos */

static int
symcmp(const void *p1, const void *p2)
{
	uintptr_t a1 = ((syment_t *)p1)->addr;
	uintptr_t a2 = ((syment_t *)p2)->addr;

	if (a1 < a2)
		return (-1);
	if (a1 > a2)
		return (1);
	return (0);
}

int
symtab_init(void)
{
	Elf		*elf;
	Elf_Scn		*scn = NULL;
	GElf_Sym	*symtab = NULL, *symp, *lastsym;
	char		*strtab = NULL;
	uint_t		cnt;
	int		fd;
	int		i;
	int		strindex = -1;

#ifndef illumos
	if ((fd = open("/dev/ksyms", O_RDONLY)) == -1) {
		if (errno == ENOENT && modfind("ksyms") == -1) {
			kldload("ksyms");
			fd = open("/dev/ksyms", O_RDONLY);
		}
		if (fd == -1)
			return (-1);
	}
#else
	if ((fd = open("/dev/ksyms", O_RDONLY)) == -1)
		return (-1);
#endif

	(void) elf_version(EV_CURRENT);

	/*
	 * Everything below reads a section header and a section's data out
	 * of /dev/ksyms, and every one of those reads could fail without
	 * this saying so.  symtab and strtab are locals; a ksyms image with
	 * no SHT_SYMTAB -- a kernel built without symbols, or an image
	 * libelf rejects for its class or version, which makes elf_begin()
	 * return NULL and both loops run zero times -- left both
	 * indeterminate, and `symtab + nsyms' below walked from one garbage
	 * pointer to another handing symp->st_name + strtab to
	 * add_symbol().
	 *
	 * Four separate things were unchecked: elf_begin() returning NULL,
	 * gelf_getshdr() failing and leaving shdr indeterminate (so
	 * sh_type, sh_size and sh_entsize are all garbage),
	 * elf_getdata() returning NULL and being dereferenced for ->d_buf,
	 * and d_buf itself being NULL, which libelf sets for a section
	 * declared SHT_NOBITS (contrib/elftoolchain/libelf/elf_data.c:269).
	 * sh_entsize is also the divisor of sh_size and was not tested for
	 * zero.
	 *
	 * The caller's contract already covers all of it: lockstat.c:1147
	 * is `if (symtab_init() == -1) fail(1, "can't load kernel
	 * symbols")'.
	 */
	if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
		(void) close(fd);
		return (-1);
	}

	for (cnt = 1; (scn = elf_nextscn(elf, scn)) != NULL; cnt++) {
		GElf_Shdr shdr;
		Elf_Data *data;

		if (gelf_getshdr(scn, &shdr) == NULL)
			continue;
		if (shdr.sh_type != SHT_SYMTAB || shdr.sh_entsize == 0)
			continue;
		if ((data = elf_getdata(scn, NULL)) == NULL ||
		    data->d_buf == NULL)
			continue;

		symtab = (GElf_Sym *)data->d_buf;
		nsyms = shdr.sh_size / shdr.sh_entsize;
		strindex = shdr.sh_link;
	}

	if (symtab == NULL || strindex < 0) {
		(void) elf_end(elf);
		(void) close(fd);
		return (-1);
	}

	for (cnt = 1; (scn = elf_nextscn(elf, scn)) != NULL; cnt++) {
		Elf_Data *data;

		if (cnt != (uint_t)strindex)
			continue;
		if ((data = elf_getdata(scn, NULL)) != NULL)
			strtab = (char *)data->d_buf;
	}

	if (strtab == NULL) {
		(void) elf_end(elf);
		(void) close(fd);
		return (-1);
	}

	lastsym = symtab + nsyms;
	nsyms = 0;
	for (symp = symtab; symp < lastsym; symp++)
		if ((uint_t)ELF32_ST_TYPE(symp->st_info) <= STT_FUNC &&
		    symp->st_size != 0)
			add_symbol(symp->st_name + strtab,
			    (uintptr_t)symp->st_value, (size_t)symp->st_size);

	fake_up_certain_popular_kernel_symbols();
	(void) sprintf(maxsymname, "0x%lx", ULONG_MAX);
	add_symbol(maxsymname, ULONG_MAX, 1);

	qsort(symbol_table, nsyms, sizeof (syment_t), symcmp);

	/*
	 * Destroy all duplicate symbols, then sort it again.
	 */
	for (i = 0; i < nsyms - 1; i++)
		if (symbol_table[i].addr == symbol_table[i + 1].addr)
			symbol_table[i].addr = 0;

	qsort(symbol_table, nsyms, sizeof (syment_t), symcmp);

	while (symbol_table[1].addr == 0) {
		symbol_table++;
		nsyms--;
	}
	symbol_table[0].name = "(usermode)";
	symbol_table[0].addr = 0;
	symbol_table[0].size = 1;

	(void) elf_end(elf);
	close(fd);
	return (0);
}

char *
addr_to_sym(uintptr_t addr, uintptr_t *offset, size_t *sizep)
{
	int lo = 0;
	int hi = nsyms - 1;
	int mid;
	syment_t *sep;

	while (hi - lo > 1) {
		mid = (lo + hi) / 2;
		if (addr >= symbol_table[mid].addr) {
			lo = mid;
		} else {
			hi = mid;
		}
	}
	sep = &symbol_table[lo];
	*offset = addr - sep->addr;
	*sizep = sep->size;
	return (sep->name);
}

uintptr_t
sym_to_addr(char *name)
{
	int i;
	syment_t *sep = symbol_table;

	for (i = 0; i < nsyms; i++) {
		if (strcmp(name, sep->name) == 0)
			return (sep->addr);
		sep++;
	}
	return (0);
}

size_t
sym_size(char *name)
{
	int i;
	syment_t *sep = symbol_table;

	for (i = 0; i < nsyms; i++) {
		if (strcmp(name, sep->name) == 0)
			return (sep->size);
		sep++;
	}
	return (0);
}
