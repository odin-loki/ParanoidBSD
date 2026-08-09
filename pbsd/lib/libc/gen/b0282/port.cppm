module;

#include <cstdlib>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
int getgrouplist(const char *, gid_t, gid_t *, int *);
int setgroups(int, const gid_t *);
int freebsd14_setgroups(int, const gid_t *);
}

export module pbsd.lib.libc.gen.b0282;

export namespace pbsd::lib_libc_gen::b0282 {

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 2025 The FreeBSD Foundation
 *
 * Portions of this software were developed by Olivier Certner
 * <olce@FreeBSD.org> at Kumacom SARL under sponsorship from the FreeBSD
 * Foundation.
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

extern "C" int freebsd14_setgroups(int gidsize, const gid_t *gidset);

static int
initgroups_impl(const char *uname, gid_t agroup,
    int (*setgroups)(int, const gid_t *))
{
	gid_t *groups;
	long ngroups_max;
	int ngroups, ret;

	/*
	 * Provide space for one group more than possible to allow setgroups()
	 * to fail and set 'errno' in case we get back more than {NGROUPS_MAX} +
	 * 1 groups.
	 */
	ngroups_max = sysconf(_SC_NGROUPS_MAX) + 2;
	groups = (gid_t *)malloc(sizeof(*groups) * ngroups_max);
	if (groups == NULL)
		return (-1); /* malloc() set 'errno'. */

	ngroups = (int)ngroups_max;
	(void)getgrouplist(uname, agroup, groups, &ngroups);
	ret = (*setgroups)(ngroups, groups);

	free(groups);
	return (ret); /* setgroups() set 'errno'. */
}

int
initgroups(const char *uname, gid_t agroup)
{
	return (initgroups_impl(uname, agroup, setgroups));
}

int
freebsd14_initgroups(const char *uname, gid_t agroup)
{
	return (initgroups_impl(uname, agroup, freebsd14_setgroups));
}

} /* namespace pbsd::lib_libc_gen::b0282 */
