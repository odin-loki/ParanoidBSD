/*
 * PBSD batch b0298 -- C++23 port of:
 *   hbsd/src/lib/libc/stdlib/insque.c
 *   hbsd/src/lib/libc/stdlib/tfind.c
 *   hbsd/src/lib/libc/stdlib/twalk.c
 *
 * Original copyright headers follow each unit.
 */
module;

#include <stdlib.h>

export module pbsd.lib.libc.stdlib.b0298;

export namespace pbsd::lib_libc_stdlib::b0298 {

/*
 * ===== hbsd/src/lib/libc/stdlib/insque.c =====
 *
 * Initial implementation:
 * Copyright (c) 2002 Robert Drehmel
 * All rights reserved.
 *
 * As long as the above copyright statement and this notice remain
 * unchanged, you can do what ever you want with this file.
 */

/* <search.h> under _SEARCH_PRIVATE. */
struct que_elem {
	struct que_elem *next;
	struct que_elem *prev;
};

void
insque(void *element, void *pred)
{
	struct que_elem *prev, *next, *elem;

	elem = (struct que_elem *)element;
	prev = (struct que_elem *)pred;

	if (prev == NULL) {
		elem->prev = elem->next = NULL;
		return;
	}

	next = prev->next;
	if (next != NULL) {
		next->prev = elem;
	}
	prev->next = elem;
	elem->prev = prev;
	elem->next = next;
}

/*
 * ===== hbsd/src/lib/libc/stdlib/tfind.c =====
 *
 *	$NetBSD: tfind.c,v 1.2 1999/09/16 11:45:37 lukem Exp $
 *
 * Tree search generalized from Knuth (6.2.2) Algorithm T just like
 * the AT&T man page says.
 *
 * Written by reading the System V Interface Definition, not the code.
 *
 * Totally public domain.
 */

typedef struct __posix_tnode {
	void			*key;
	struct __posix_tnode	*llink, *rlink;
	signed char		 balance;
} posix_tnode;

posix_tnode *
tfind(const void *vkey, posix_tnode * const *rootp,
    int (*compar)(const void *, const void *))
{

	if (rootp == NULL)
		return NULL;

	while (*rootp != NULL) {		/* T1: */
		int r;

		if ((r = (*compar)(vkey, (*rootp)->key)) == 0)	/* T2: */
			return *rootp;		/* key found */
		rootp = (r < 0) ?
		    &(*rootp)->llink :		/* T3: follow left branch */
		    &(*rootp)->rlink;		/* T4: follow right branch */
	}
	return NULL;
}

/*
 * ===== hbsd/src/lib/libc/stdlib/twalk.c =====
 *
 *	$NetBSD: twalk.c,v 1.4 2012/03/20 16:38:45 matt Exp $
 *
 * Tree search generalized from Knuth (6.2.2) Algorithm T just like
 * the AT&T man page says.
 *
 * Written by reading the System V Interface Definition, not the code.
 *
 * Totally public domain.
 */

typedef enum {
	preorder,
	postorder,
	endorder,
	leaf
} VISIT;

typedef void (*cmp_fn_t)(const posix_tnode *, VISIT, int);

} /* namespace */

namespace pbsd::lib_libc_stdlib::b0298 {

static void
trecurse(const posix_tnode *root, cmp_fn_t action, int level)
{

	if (root->llink != NULL && root->rlink == NULL)
		(*action)(root, leaf, level);
	else {
		(*action)(root, preorder, level);
		if (root->llink != NULL)
			trecurse(root->llink, action, level + 1);
		(*action)(root, postorder, level);
		if (root->rlink != NULL)
			trecurse(root->rlink, action, level + 1);
		(*action)(root, endorder, level);
	}
}

} /* namespace */

export namespace pbsd::lib_libc_stdlib::b0298 {

void
twalk(const posix_tnode *vroot, cmp_fn_t action)
{
	if (vroot != NULL && action != NULL)
		trecurse(vroot, action, 0);
}

} /* namespace */
