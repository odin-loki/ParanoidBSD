/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/: OF_getprop() returns the PROPERTY's length, not what it
 * copied, and four callers used the return value as a bound on the
 * buffer they passed.
 *
 *   cbmc -DOLD --unwind 40 tools/verify/probes/ofw_getprop_length.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       --unwind 40 tools/verify/probes/ofw_getprop_length.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	powerpc/ofw/cas.c:206
 *	    uint8_t buf[16];
 *	    len = OF_getprop(pkg, "ibm,arch-vec-5-platform-support", buf,
 *	        sizeof(buf));
 *	    ...
 *	    for (i = 0; i < len; i += 2) {
 *	            idx = buf[i];
 *	            val = buf[i + 1];
 *
 * libofw/openfirm.c:253 is
 *
 *	if (openfirmware(&args) == -1)
 *	        return (-1);
 *	return (OUT(args.size));
 *
 * and IEEE 1275's getprop returns the property's actual length while
 * copying at most buflen of it.  A property longer than the buffer
 * therefore walks past the end, with the overshoot chosen by the
 * device tree.
 *
 * libofw/ofw_memory.c:64 divides the same return into entries and
 * indexes a 256-entry array by the result; libofw/ofw_net.c:202 and
 * powerpc/ofw/main.c:192 take it for a string that OF_getprop() need
 * not have terminated.
 *
 * CBMC found this one as `array 'buf' upper bound' in ppc64_cas(), on
 * the strength of OF_getprop() having no model - which is the
 * extern-driven bucket, and would normally be a deferral.  It is not
 * one here: the return really is unconstrained by the buffer, so the
 * unmodelled callee and the specification agree.
 *
 * The assertion is that every subscript the loop reaches buf[] with is
 * one buf[] has.
 */

#define	BUFSZ	16

int nondet_int(void);

int
main(void)
{
	int len = nondet_int();
	int i;

	/* A property length, bounded only by what the tree can say. */
	__CPROVER_assume(len >= -1 && len <= 64);
	if (len == -1)
		return (0);

#ifndef OLD
	if (len > BUFSZ)
		len = BUFSZ;
	len &= ~1;
#endif
	for (i = 0; i < len; i += 2) {
		__CPROVER_assert(i >= 0 && i + 1 < BUFSZ,
		    "the subscript the loop reaches buf[] with is one it has");
	}
	return (0);
}
