module;

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <unistd.h>

export module pbsd.lib.libc.gen.b0321;

export namespace pbsd::lib_libc_gen::b0321 {

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993 The Regents of the University of California.
 * Copyright (c) 2017 Mariusz Zaborski <oshogbo@FreeBSD.org>
 * All rights reserved.
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

int
daemonfd(int chdirfd, int nullfd)
{
	struct sigaction osa, sa;
	pid_t newgrp;
	int oerrno;
	int osa_ok;

	/* A SIGHUP may be thrown when the parent exits below. */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	osa_ok = ::sigaction(SIGHUP, &sa, &osa);

	switch (fork()) {
	case -1:
		return (-1);
	case 0:
		break;
	default:
		/*
		 * A fine point:  _exit(0), not exit(0), to avoid triggering
		 * atexit(3) processing
		 */
		::_exit(0);
	}

	newgrp = setsid();
	oerrno = errno;
	if (osa_ok != -1)
		::sigaction(SIGHUP, &osa, NULL);

	if (newgrp == -1) {
		errno = oerrno;
		return (-1);
	}

	if (chdirfd != -1)
		(void)fchdir(chdirfd);

	if (nullfd != -1) {
		(void)::dup2(nullfd, STDIN_FILENO);
		(void)::dup2(nullfd, STDOUT_FILENO);
		(void)::dup2(nullfd, STDERR_FILENO);
	}
	return (0);
}

int
daemon(int nochdir, int noclose)
{
	int chdirfd, nullfd, ret;

	if (!noclose)
		nullfd = ::open(_PATH_DEVNULL, O_RDWR, 0);
	else
		nullfd = -1;

	if (!nochdir)
		chdirfd = ::open("/", O_EXEC);
	else
		chdirfd = -1;

	ret = daemonfd(chdirfd, nullfd);

	if (chdirfd != -1)
		::close(chdirfd);

	if (nullfd > 2)
		::close(nullfd);

	return (ret);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Tom Truscott.
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

/* ARGSUSED */
int
__freebsd11_des_setkey(const char *key)
{
	(void)key;
	fprintf(stderr, "WARNING!  des_setkey(3) not present in the system!\n");
	return (0);
}

/* ARGSUSED */
int
__freebsd11_des_cipher(const char *in, char *out, long salt, int num_iter)
{
	(void)salt;
	(void)num_iter;
	fprintf(stderr, "WARNING!  des_cipher(3) not present in the system!\n");
	std::memcpy(out, in, 8);
	return (0);
}

/* ARGSUSED */
int
__freebsd11_setkey(const char *key)
{
	(void)key;
	fprintf(stderr, "WARNING!  setkey(3) not present in the system!\n");
	return (0);
}

/* ARGSUSED */
int
__freebsd11_encrypt(char *block, int flag)
{
	(void)block;
	(void)flag;
	fprintf(stderr, "WARNING!  encrypt(3) not present in the system!\n");
	return (0);
}

} /* namespace pbsd::lib_libc_gen::b0321 */
