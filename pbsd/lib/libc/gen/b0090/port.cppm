/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
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
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
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

/*
 * Ports of:
 *	lib/libc/gen/getgrouplist.c
 *	lib/libc/gen/getbootfile.c
 *	lib/libc/gen/getdomainname.c
 */

module;

#include <stddef.h>
#include <sys/types.h>

/*
 * <sys/param.h> / <sys/sysctl.h> are not available in this build environment;
 * the FreeBSD values of the macros the bodies below reference are supplied
 * verbatim.  MAXPATHLEN is force-defined so that the static buffer in
 * getbootfile() has the same size the original had, regardless of any host
 * header that may have defined it differently.
 */
#undef MAXPATHLEN
#define MAXPATHLEN 1024

#ifndef CTL_KERN
#define CTL_KERN 1
#endif
#ifndef KERN_NISDOMAINNAME
#define KERN_NISDOMAINNAME 22
#endif
#ifndef KERN_BOOTFILE
#define KERN_BOOTFILE 26
#endif

export module pbsd.lib.libc.gen.b0090;

extern "C" {
int sysctl(const int *, unsigned int, void *, size_t *, const void *, size_t);
int __getgroupmembership(const char *, gid_t, gid_t *, int, int *);
}

export namespace pbsd::lib_libc_gen::b0090 {

/* lib/libc/gen/getgrouplist.c */

int
getgrouplist(const char *uname, gid_t agroup, gid_t *groups, int *grpcnt)
{
	return __getgroupmembership(uname, agroup, groups, *grpcnt, grpcnt);
}

/* lib/libc/gen/getbootfile.c */

const char *
getbootfile(void)
{
	static char name[MAXPATHLEN];
	size_t size = sizeof name;
	int mib[2];

	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTFILE;
	if (sysctl(mib, 2, name, &size, NULL, 0) == -1)
		return ("/boot/kernel/kernel");
	return (name);
}

/* lib/libc/gen/getdomainname.c */

int
getdomainname(char *name, int namelen)
{
	int mib[2];
	size_t size;

	mib[0] = CTL_KERN;
	mib[1] = KERN_NISDOMAINNAME;
	size = namelen;
	if (sysctl(mib, 2, name, &size, NULL, 0) == -1)
		return (-1);
	return (0);
}

} /* namespace pbsd::lib_libc_gen::b0090 */
