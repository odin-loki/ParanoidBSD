/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <wctype.h>

#undef iswalnum
int
ref_iswalnum(wint_t wc)
{
	return (__istype(wc, _CTYPE_A|_CTYPE_N));
}

#undef iswalpha
int
ref_iswalpha(wint_t wc)
{
	return (__istype(wc, _CTYPE_A));
}

#undef iswascii
int
ref_iswascii(wint_t wc)
{
	return ((wc & ~0x7F) == 0);
}

#undef iswblank
int
ref_iswblank(wint_t wc)
{
	return (__istype(wc, _CTYPE_B));
}

#undef iswcntrl
int
ref_iswcntrl(wint_t wc)
{
	return (__istype(wc, _CTYPE_C));
}

#undef iswdigit
int
ref_iswdigit(wint_t wc)
{
	return (__istype(wc, _CTYPE_D));
}

#undef iswgraph
int
ref_iswgraph(wint_t wc)
{
	return (__istype(wc, _CTYPE_G));
}

#undef iswhexnumber 
int
ref_iswhexnumber(wint_t wc)
{
	return (__istype(wc, _CTYPE_X));
}

#undef iswideogram
int
ref_iswideogram(wint_t wc)
{
	return (__istype(wc, _CTYPE_I));
}

#undef iswlower
int
ref_iswlower(wint_t wc)
{
	return (__istype(wc, _CTYPE_L));
}

#undef iswnumber
int
ref_iswnumber(wint_t wc)
{
	return (__istype(wc, _CTYPE_N));
}

#undef iswphonogram	
int
ref_iswphonogram(wint_t wc)
{
	return (__istype(wc, _CTYPE_Q));
}

#undef iswprint
int
ref_iswprint(wint_t wc)
{
	return (__istype(wc, _CTYPE_R));
}

#undef iswpunct
int
ref_iswpunct(wint_t wc)
{
	return (__istype(wc, _CTYPE_P));
}

#undef iswrune
int
ref_iswrune(wint_t wc)
{
	return (__istype(wc, 0xFFFFFF00L));
}

#undef iswspace
int
ref_iswspace(wint_t wc)
{
	return (__istype(wc, _CTYPE_S));
}

#undef iswspecial
int
ref_iswspecial(wint_t wc)
{
	return (__istype(wc, _CTYPE_T));
}

#undef iswupper
int
ref_iswupper(wint_t wc)
{
	return (__istype(wc, _CTYPE_U));
}

#undef iswxdigit
int
ref_iswxdigit(wint_t wc)
{
	return (__istype(wc, _CTYPE_X));
}

#undef towlower
wint_t
ref_towlower(wint_t wc)
{
        return (__tolower(wc));
}

#undef towupper
wint_t
ref_towupper(wint_t wc)
{
        return (__toupper(wc));
}

