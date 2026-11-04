/*
 * oracle.c -- reference implementation for PBSD batch b0298.
 *
 * The original HardenedBSD C sources of the batch, concatenated, with every
 * function renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.
 */

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
#define	_SEARCH_PRIVATE
#include <search.h>
#ifdef DEBUG
#include <stdio.h>
#else
#include <stdlib.h>	/* for NULL */
#endif

/*
 * Host <search.h> may not expose que_elem under _SEARCH_PRIVATE.
 */
struct que_elem {
	struct que_elem *next;
	struct que_elem *prev;
};

void
ref_insque(void *element, void *pred)
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
#ifdef DEBUG
		if (next->prev != prev) {
			fprintf(stderr, "insque: Inconsistency detected:"
			    " next(%p)->prev(%p) != prev(%p)\n",
			    next, next->prev, prev);
		}
#endif
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

#include <stdlib.h>

/*
 * Host <search.h> may not expose posix_tnode under _SEARCH_PRIVATE.
 */
typedef struct __posix_tnode {
	void			*key;
	struct __posix_tnode	*llink, *rlink;
	signed char		 balance;
} posix_tnode;

/* 
 * find a node, or return 0
 *
 * vkey   - key to be found 
 * vrootp - address of the tree root 
 */
posix_tnode *
ref_tfind(const void *vkey, posix_tnode * const *rootp,
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

typedef void (*cmp_fn_t)(const posix_tnode *, VISIT, int);

/* Walk the nodes of a tree */
static void
ref_trecurse(const posix_tnode *root, cmp_fn_t action, int level)
{

	if (root->llink == NULL && root->rlink == NULL)
		(*action)(root, leaf, level);
	else {
		(*action)(root, preorder, level);
		if (root->llink != NULL)
			ref_trecurse(root->llink, action, level + 1);
		(*action)(root, postorder, level);
		if (root->rlink != NULL)
			ref_trecurse(root->rlink, action, level + 1);
		(*action)(root, endorder, level);
	}
}

/* Walk the nodes of a tree */
void
ref_twalk(const posix_tnode *vroot, cmp_fn_t action)
{
	if (vroot != NULL && action != NULL)
		ref_trecurse(vroot, action, 0);
}
