/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * cddl/.../tools/ctf/cvt/ctf.c: ctf_parse() is handed the CTF buffer and
 * no length, and every region inside it is delimited by a pair of
 * uint32_t header fields the file chose.
 *
 *   cbmc --bounds-check --unsigned-overflow-check --unwind 64 \
 *        --unwinding-assertions -DOLD \
 *        tools/verify/probes/ctf_parse_unbounded.c    -> FAILED
 *   cbmc --bounds-check --unsigned-overflow-check --unwind 64 \
 *        --unwinding-assertions \
 *        tools/verify/probes/ctf_parse_unbounded.c    -> SUCCESSFUL
 *
 * Two properties, both of which held only by accident before:
 *
 *   1. Every region size is a difference of two header fields computed
 *      into a size_t --- `cth_objtoff - cth_lbloff' (ctf.c:894),
 *      `cth_funcoff - cth_objtoff' (:939), `cth_typeoff - cth_funcoff'
 *      (:986), `cth_stroff - cth_typeoff' (:1061).  Out of order, the
 *      subtraction underflows and the region is 2^64 bytes long.
 *
 *   2. A string-table offset --- ctl_label, ctt_name, ctm_name,
 *      ctlm_name, cte_name, cth_parlabel --- was added to
 *      `ctfdata + cth_stroff' and handed to xstrdup().  Neither that the
 *      offset starts inside the table nor that the string terminates
 *      inside it was checked, and the second survives a bound on the
 *      offset alone.
 *
 * The buffer here is CTFSZ bytes and the header fields are
 * unconstrained, which is what the section gives ctf_parse().  The
 * assertions are that every region lies inside the buffer and that a
 * string handed back by strptr() is terminated inside the string table.
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define	CTFSZ	64

typedef struct {
	uint32_t cth_parlabel;
	uint32_t cth_lbloff;
	uint32_t cth_objtoff;
	uint32_t cth_funcoff;
	uint32_t cth_typeoff;
	uint32_t cth_stroff;
	uint32_t cth_strlen;
} hdr_t;

static char ctfdata[CTFSZ];

extern uint32_t nondet_u32(void);
extern size_t nondet_size(void);

#ifndef OLD
static int
check_header(const hdr_t *h)
{
	if (h->cth_lbloff > h->cth_objtoff ||
	    h->cth_objtoff > h->cth_funcoff ||
	    h->cth_funcoff > h->cth_typeoff ||
	    h->cth_typeoff > h->cth_stroff)
		return (0);

	if (h->cth_strlen > UINT32_MAX - h->cth_stroff)
		return (0);

	return (1);
}
#endif

static const char *
strptr(const hdr_t *h, char *base, uint32_t off)
{
	const char *strtab = base + h->cth_stroff;

#ifndef OLD
	if (off >= h->cth_strlen)
		return (NULL);
	if (memchr(strtab + off, '\0', h->cth_strlen - off) == NULL)
		return (NULL);
#endif
	return (strtab + off);
}

/*
 * The shape of the four resurrect_ loops: a base and a size, both
 * derived from the header, walked one record at a time.
 */
static void
walk(char *base, size_t off, size_t sz)
{
	size_t i;

	for (i = 0; i < sz; i++) {
		assert(off + i < CTFSZ);	/* the region is in the buffer */
		(void)base[off + i];
	}
}

int
main(void)
{
	hdr_t h;
	const char *s;
	size_t n;

	h.cth_parlabel = nondet_u32();
	h.cth_lbloff = nondet_u32();
	h.cth_objtoff = nondet_u32();
	h.cth_funcoff = nondet_u32();
	h.cth_typeoff = nondet_u32();
	h.cth_stroff = nondet_u32();
	h.cth_strlen = nondet_u32();

	/*
	 * ctf_load()'s one length test: the section really holds what the
	 * header says it does.  That is the whole of what the caller knows,
	 * and on its own it is not enough.
	 */
	__CPROVER_assume((size_t)h.cth_stroff + h.cth_strlen <= CTFSZ);

#ifndef OLD
	if (!check_header(&h))
		return (0);
#endif

	walk(ctfdata, h.cth_lbloff, (size_t)(h.cth_objtoff - h.cth_lbloff));
	walk(ctfdata, h.cth_objtoff, (size_t)(h.cth_funcoff - h.cth_objtoff));
	walk(ctfdata, h.cth_funcoff, (size_t)(h.cth_typeoff - h.cth_funcoff));
	walk(ctfdata, h.cth_typeoff, (size_t)(h.cth_stroff - h.cth_typeoff));

	s = strptr(&h, ctfdata, h.cth_parlabel);
	if (s != NULL) {
		/* xstrdup() walks to the NUL; it must be inside the buffer. */
		for (n = 0; ; n++) {
			assert(s + n < ctfdata + CTFSZ);
			if (s[n] == '\0')
				break;
		}
	}

	return (0);
}
