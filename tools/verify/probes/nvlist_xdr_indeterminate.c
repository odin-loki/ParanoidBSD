/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/libsa/zfs/nvlist.c: xdr_short(), xdr_u_short() and xdr_char()
 * read *ip before deciding whether this is an encode or a decode, and
 * on a decode *ip is the caller's OUTPUT.
 *
 *   cbmc -DOLD tools/verify/probes/nvlist_xdr_indeterminate.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       tools/verify/probes/nvlist_xdr_indeterminate.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	static bool
 *	xdr_char(xdr_t *xdr, char *cp)
 *	{
 *		int i;
 *		bool rv = false;
 *
 *		i = *cp;
 *		if ((rv = xdr_int(xdr, &i))) {
 *			if (xdr->xdr_op == XDR_OP_DECODE)
 *				*cp = i;
 *		}
 *		return (rv);
 *	}
 *
 * nvlist_print() at nvlist.c:1588-1601 declares `char c;' and
 * `unsigned short u;' and passes each straight in to be decoded, so
 * the `i = *cp' is a read of an indeterminate value - undefined
 * behaviour, and the one thing about it that is NOT benign is that a
 * compiler is entitled to assume it does not happen.
 *
 * The value is discarded on the decode path anyway, so reading it only
 * when encoding costs nothing and says what the code means.
 *
 * The assertion is that every byte the function reads out of the
 * caller's object is one the caller wrote.
 */

#define	XDR_OP_ENCODE	1
#define	XDR_OP_DECODE	2

int nondet_int(void);

static int written;		/* shadow: has the caller's object a value? */
static int object;

static int
read_object(void)
{
	__CPROVER_assert(written == 1,
	    "every byte read out of the caller's object is one it wrote");
	return (object);
}

static int
xdr_char(int op, int *cp_written)
{
	int i;

#ifdef OLD
	(void)cp_written;
	i = read_object();
#else
	i = (op == XDR_OP_ENCODE) ? read_object() : 0;
#endif
	if (op == XDR_OP_DECODE) {
		object = i;
		written = 1;
		*cp_written = 1;
	}
	return (i);
}

int
main(void)
{
	int op = nondet_int() ? XDR_OP_ENCODE : XDR_OP_DECODE;
	int cp_written = 0;

	if (op == XDR_OP_ENCODE) {
		/* An encode has a value to send. */
		object = nondet_int();
		written = 1;
	} else {
		/* nvlist_print(): `char c;' - declared and not written. */
		written = 0;
	}
	xdr_char(op, &cp_written);
	return (0);
}
