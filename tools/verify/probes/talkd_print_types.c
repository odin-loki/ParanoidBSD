/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * libexec/talkd/print.c: three bound tests use > where the array wants
 * >=, on a type byte that came off the wire.
 *
 *   cbmc -DOLD tools/verify/probes/talkd_print_types.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       tools/verify/probes/talkd_print_types.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	static const char *types[] =
 *	    { "leave_invite", "look_up", "delete", "announce" };
 *	#define NTYPES (sizeof (types) / sizeof (types[0]))
 *	...
 *	if (mp->type > NTYPES) {
 *		(void)snprintf(tbuf, sizeof(tbuf), "type %d", mp->type);
 *		tp = tbuf;
 *	} else
 *		tp = types[mp->type];
 *
 * NTYPES is 4 and the last valid subscript is 3, so `> NTYPES' lets 4
 * through and types[4] is one pointer past the end.  mp->type is an
 * `unsigned char' in the CTL_MSG a talkd(8) reads from a UDP socket,
 * so a remote sender picks the subscript, and the const char * that
 * comes back -- whatever the next object in .data happens to hold --
 * is handed straight to syslog(3) as a %s.
 *
 * print_response() has the same test on types[rp->type] and again on
 * answers[rp->answer], where NANSWERS is 9.  Three sites, one
 * character each.
 *
 * The assertion is that every subscript this reaches the array with is
 * one the array has.
 */

unsigned char nondet_uchar(void);

#define	NTYPES	4

static const char *types[NTYPES] = { "a", "b", "c", "d" };

static const char *
print_request(unsigned char type)
{
	/* The out-of-range arm formats the number instead. */
#ifdef OLD
	if (type > NTYPES)
		return ("type N");
#else
	if (type >= NTYPES)
		return ("type N");
#endif
	__CPROVER_assert(type < NTYPES,
	    "the subscript reaching types[] is one types[] has");
	return (types[type]);
}

int
main(void)
{
	/* Every byte a remote sender can put in CTL_MSG.type. */
	print_request(nondet_uchar());
	return (0);
}
