/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * cddl/.../libdtrace/common/dt_pragma.c:dt_pragma_attributes() checks
 * ONE link of the dn_list chain and then walks three.
 *
 *   cbmc --pointer-check -DOLD
 *        tools/verify/probes/dt_pragma_provider_depth.c
 *       -> dereference failure: pointer NULL, FAILED
 *   cbmc --pointer-check
 *        tools/verify/probes/dt_pragma_provider_depth.c
 *       -> 0 of N, SUCCESSFUL
 *
 *	if (dnp == NULL || dnp->dn_kind != DT_NODE_IDENT ||
 *	    dnp->dn_list == NULL || dnp->dn_list->dn_kind != DT_NODE_IDENT)
 *		xyerror(D_PRAGMA_MALFORM, ...);        // one link checked
 *	...
 *	dnp = dnp->dn_list;                            // link 1, safe
 *	name = dnp->dn_string;
 *	if (strcmp(name, "provider") == 0) {
 *		dnp = dnp->dn_list;                    // link 2, UNCHECKED
 *		name = dnp->dn_string;
 *		dnp = dnp->dn_list;                    // link 3, UNCHECKED
 *		part = dnp->dn_string;
 *
 * The list is the parsed argument list of a #pragma line in a .d
 * script, so its length is whatever the author typed:
 *
 *	#pragma D attributes Evolving/Evolving/Common provider
 *
 * ends the chain after "provider" and reaches link 2 with NULL. That is
 * a segfault in dtrace(1) where the function three lines up already
 * knows how to report a malformed pragma.
 *
 * The assertion is that every link the code walks was checked first.
 */

struct node { struct node *dn_list; const char *dn_string; };

int nondet_int(void);

/* A chain of exactly `len' nodes, 1..3. */
static struct node n1, n2, n3;

static struct node *
chain(int len)
{
	n1.dn_list = (len > 1) ? &n2 : (struct node *)0;
	n2.dn_list = (len > 2) ? &n3 : (struct node *)0;
	n3.dn_list = (struct node *)0;
	n1.dn_string = n2.dn_string = n3.dn_string = "provider";
	return (&n1);
}

int
main(void)
{
	int len = nondet_int();
	const char *name, *part;
	struct node *dnp;

	__CPROVER_assume(len >= 2 && len <= 4);   /* the head plus 1..3 */
	dnp = chain(len);

	/* The function's own guard: dnp and ONE link. */
	if (dnp == (struct node *)0 || dnp->dn_list == (struct node *)0)
		return (0);                        /* xyerror(), noreturn */

	dnp = dnp->dn_list;
	name = dnp->dn_string;
	(void)name;

#ifndef OLD
	if (dnp->dn_list == (struct node *)0 ||
	    dnp->dn_list->dn_list == (struct node *)0)
		return (0);                        /* xyerror(), noreturn */
#endif

	dnp = dnp->dn_list;
	name = dnp->dn_string;

	dnp = dnp->dn_list;
	part = dnp->dn_string;

	return (name != part);
}
