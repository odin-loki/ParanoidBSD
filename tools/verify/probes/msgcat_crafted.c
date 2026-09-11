/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * lib/libc/nls/msgcat.c's valid_msgcat(), against catalogues a hostile
 * NLSPATH could point catopen(3) at.  Not a model -- the function's own
 * source, copied in below the line, driven with crafted headers:
 *
 *   sed -n '/^valid_msgcat/,/^}/p' \
 *       hbsd/src/lib/libc/nls/msgcat.c > /tmp/valid.inc
 *   cc -O2 -Wall -fsanitize=address,undefined \
 *       -o /tmp/h tools/verify/probes/msgcat_crafted.c && /tmp/h
 *
 *   a well-formed catalogue                    accept   ok
 *   __nsets = INT_MAX                          reject   ok
 *   __nsets = -1                               reject   ok
 *   __msg_hdr_offset = INT_MAX                 reject   ok
 *   __msg_txt_offset = -1                      reject   ok
 *   set __index huge                           reject   ok
 *   set __nmsgs = INT_MAX                      reject   ok
 *   set __index = -1                           reject   ok
 *   message __offset = INT_MAX                 reject   ok
 *   message __offset = -1                      reject   ok
 *   message text with no NUL                   reject   ok
 *   a four-byte file                           reject   ok
 *
 *   all cases as expected
 *
 * Each rejected case is one catgets(3) used to answer by returning a
 * pointer outside the mapping, which its caller then reads as a string.
 * The last two are the ones the old code's only two checks let through:
 * it tested the size against sizeof(u_int32_t) and the magic number,
 * and nothing else in the file was ever looked at.
 *
 * Rejecting hostile catalogues is only half of it -- the tree's own
 * gencat(1) was built and run to check that legitimate ones still
 * load.  A two-set, four-message catalogue (126 bytes, mhoff 24,
 * mtoff 72) and a two-set, 348-message one (15552 bytes, mhoff 24,
 * mtoff 4200) are both ACCEPTed.  The layout valid_msgcat() assumes is
 * the one gencat() writes: __msg_hdr_offset and __msg_txt_offset are
 * relative to the end of the catalogue header, __index is a cumulative
 * index into the single message-header table, and __offset is a
 * cumulative byte offset into the single string pool.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

struct _nls_cat_hdr {
	int32_t __magic, __nsets, __mem, __msg_hdr_offset, __msg_txt_offset;
};
struct _nls_set_hdr { int32_t __setno, __nmsgs, __index; };
struct _nls_msg_hdr { int32_t __msgno, __msglen, __offset; };

static int
#include "/tmp/valid.inc"

static unsigned char buf[256];
static struct _nls_cat_hdr *C = (void *)buf;
static struct _nls_set_hdr *S_ = (void *)(buf + sizeof(struct _nls_cat_hdr));

/* one set, one message, laid out the way gencat(1) does */
static size_t
build(void)
{
	struct _nls_msg_hdr *m;
	size_t mhoff = sizeof(*S_), mtoff = mhoff + sizeof(*m);

	memset(buf, 0, sizeof(buf));
	C->__nsets = htonl(1);
	C->__msg_hdr_offset = htonl((uint32_t)mhoff);
	C->__msg_txt_offset = htonl((uint32_t)mtoff);
	S_->__setno = htonl(1);
	S_->__nmsgs = htonl(1);
	S_->__index = htonl(0);
	m = (void *)(buf + sizeof(*C) + mhoff);
	m->__msgno = htonl(1);
	m->__msglen = htonl(3);
	m->__offset = htonl(0);
	memcpy(buf + sizeof(*C) + mtoff, "hi", 3);
	return (sizeof(*C) + mtoff + 3);
}

#define T(what, expect) do {						\
	int got = valid_msgcat(buf, n);					\
	printf("%-42s %-8s %s\n", what, got ? "accept" : "reject",	\
	    got == (expect) ? "ok" : "WRONG");				\
	if (got != (expect))						\
		bad++;							\
} while (0)

int
main(void)
{
	struct _nls_msg_hdr *m;
	size_t n, off;
	int bad = 0;

	n = build();					T("a well-formed catalogue", 1);
	n = build(); C->__nsets = htonl(0x7fffffff);	T("__nsets = INT_MAX", 0);
	n = build(); C->__nsets = htonl(0xffffffff);	T("__nsets = -1", 0);
	n = build(); C->__msg_hdr_offset = htonl(0x7fffffff);
							T("__msg_hdr_offset = INT_MAX", 0);
	n = build(); C->__msg_txt_offset = htonl(0xffffffff);
							T("__msg_txt_offset = -1", 0);
	n = build(); S_->__index = htonl(0x40000000);	T("set __index huge", 0);
	n = build(); S_->__nmsgs = htonl(0x7fffffff);	T("set __nmsgs = INT_MAX", 0);
	n = build(); S_->__index = htonl(0xffffffff);	T("set __index = -1", 0);

	n = build();
	m = (void *)(buf + sizeof(*C) + ntohl(C->__msg_hdr_offset));
	m->__offset = htonl(0x7fffffff);		T("message __offset = INT_MAX", 0);
	n = build();
	m = (void *)(buf + sizeof(*C) + ntohl(C->__msg_hdr_offset));
	m->__offset = htonl(0xffffffff);		T("message __offset = -1", 0);
	n = build();
	off = sizeof(*C) + ntohl(C->__msg_txt_offset);
	memset(buf + off, 'A', n - off);		T("message text with no NUL", 0);

	n = 4;						T("a four-byte file", 0);

	printf("\n%s\n", bad ? "FAILURES" : "all cases as expected");
	return (bad != 0);
}
