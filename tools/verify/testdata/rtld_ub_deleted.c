/* SPDX-License-Identifier: AGPL-3.0-or-later */
/* SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au> */
/*
 * Evidence for docs/BUILDING.md, "SOLVED - run 57 boots".
 *
 * This is libexec/rtld-elf/rtld.c's _rtld() prologue reduced to the one
 * thing that mattered: aux_info is an automatic array, and the
 * HARDENEDBSD block read aux_info[AT_PAXFLAGS] before either loop wrote
 * it - then dereferenced it and stored through it.
 *
 * Reading an indeterminate automatic is undefined behaviour, and a
 * compiler may assume undefined behaviour does not occur. If the read
 * cannot legally happen the code depending on it is unreachable, and
 * unreachable code can be deleted.
 *
 *   clang -O2 -c rtld_ub_deleted.c -o broken.o
 *   clang -O2 -c -DFIXED rtld_ub_deleted.c -o fixed.o
 *   objdump -t broken.o fixed.o | grep rtldish
 *
 * With clang 18.1.3 at -O2:
 *
 *   broken.o   rtldish = 0x0    bytes   <- the whole function, gone
 *   fixed.o    rtldish = 0x73   bytes
 *
 * `clang' and not `cc': gcc 13.3.0 on the same machine, same flags,
 * keeps the broken form at 0x8f bytes. Two compilers one `cc' symlink
 * apart disagree about what this source means, which is the whole
 * argument in one line - and it is why "upstream HardenedBSD has this
 * too and upstream boots" was not evidence about the code. PBSD builds
 * with an external clang.
 *
 * The shipped binaries agree. `_rtld' occupied 32 bytes in boot run
 * 56's ld-elf.so.1 and 13,136 in run 57's, and pid 1 spent every one of
 * runs 43 to 56 spinning at one instruction inside the hole.
 *
 * Not a gate. A future compiler that stops deleting the broken form is
 * not a PBSD regression, and one that starts deleting more is not
 * either - the defect is the read, not what any one optimiser does with
 * it. This file exists so the claim in the write-up can be re-run
 * rather than believed.
 */
#include <stddef.h>

struct au {
	unsigned long a_type;
	union { unsigned long a_val; } a_un;
};

#define	AT_COUNT	41
#define	AT_PAXFLAGS	40

static unsigned long pax_flags = 0;
extern void work(struct au **, int);

#define	READ_PAX_FLAGS()						\
	do {								\
		if (aux_info[AT_PAXFLAGS] != NULL) {			\
			pax_flags = aux_info[AT_PAXFLAGS]->a_un.a_val;	\
			aux_info[AT_PAXFLAGS]->a_un.a_val = 0;		\
		}							\
	} while (0)

#define	DIGEST_AUXV()							\
	do {								\
		for (i = 0; i < AT_COUNT; i++)				\
			aux_info[i] = NULL;				\
		for (p = aux; p->a_type != 0; p++)			\
			if (p->a_type < AT_COUNT)			\
				aux_info[p->a_type] = p;		\
	} while (0)

void
rtldish(struct au *aux, int n)
{
	struct au *aux_info[AT_COUNT];
	struct au *p;
	int i;

#ifdef FIXED
	DIGEST_AUXV();
	READ_PAX_FLAGS();
#else
	READ_PAX_FLAGS();	/* aux_info is indeterminate here */
	DIGEST_AUXV();
#endif
	work(aux_info, n);
}
