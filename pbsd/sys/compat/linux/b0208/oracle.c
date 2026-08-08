/*
 * PBSD batch b0208 -- reference oracle.
 *
 * The original HardenedBSD C sources for this batch, concatenated, with every
 * function renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   sys/compat/linux/linux_errno.c   (+ linux_errno.inc, linux_errno.h)
 *   sys/compat/linux/linux_getcwd.c
 *
 * Everything above the "ORIGINAL SOURCES" banner is the freestanding kernel
 * shim needed to build these bodies in userspace.  It supplies the FreeBSD and
 * Linux errno numbers, LINUX_PATH_MAX, and the kernel primitives KASSERT,
 * panic, malloc, free, vn_getcwd and copyout.  The primitives are resolved at
 * link time against the mocks in harness.cpp, so the port and this oracle are
 * driven through byte-identical environments.
 *
 * KASSERT reports every evaluated predicate to hbsd_kassert_observe() before
 * behaving exactly as the kernel does (panic when the predicate is false).
 * The kernel's KASSERT is likewise observable when INVARIANTS is configured;
 * routing it through a hook is what makes linux_check_errtbl(), which is
 * otherwise a pure no-op, differentially testable.
 */

#include <stddef.h>

/* ------------------------------------------------------------------ */
/* sys/sys/errno.h						      */
/* ------------------------------------------------------------------ */

#define	EPERM		1		/* Operation not permitted */
#define	ENOENT		2		/* No such file or directory */
#define	ESRCH		3		/* No such process */
#define	EINTR		4		/* Interrupted system call */
#define	EIO		5		/* Input/output error */
#define	ENXIO		6		/* Device not configured */
#define	E2BIG		7		/* Argument list too long */
#define	ENOEXEC		8		/* Exec format error */
#define	EBADF		9		/* Bad file descriptor */
#define	ECHILD		10		/* No child processes */
#define	EDEADLK		11		/* Resource deadlock avoided */
#define	ENOMEM		12		/* Cannot allocate memory */
#define	EACCES		13		/* Permission denied */
#define	EFAULT		14		/* Bad address */
#define	ENOTBLK		15		/* Block device required */
#define	EBUSY		16		/* Device busy */
#define	EEXIST		17		/* File exists */
#define	EXDEV		18		/* Cross-device link */
#define	ENODEV		19		/* Operation not supported by device */
#define	ENOTDIR		20		/* Not a directory */
#define	EISDIR		21		/* Is a directory */
#define	EINVAL		22		/* Invalid argument */
#define	ENFILE		23		/* Too many open files in system */
#define	EMFILE		24		/* Too many open files */
#define	ENOTTY		25		/* Inappropriate ioctl for device */
#define	ETXTBSY		26		/* Text file busy */
#define	EFBIG		27		/* File too large */
#define	ENOSPC		28		/* No space left on device */
#define	ESPIPE		29		/* Illegal seek */
#define	EROFS		30		/* Read-only filesystem */
#define	EMLINK		31		/* Too many links */
#define	EPIPE		32		/* Broken pipe */
#define	EDOM		33		/* Numerical argument out of domain */
#define	ERANGE		34		/* Result too large */
#define	EAGAIN		35		/* Resource temporarily unavailable */
#define	EINPROGRESS	36		/* Operation now in progress */
#define	EALREADY	37		/* Operation already in progress */
#define	ENOTSOCK	38		/* Socket operation on non-socket */
#define	EDESTADDRREQ	39		/* Destination address required */
#define	EMSGSIZE	40		/* Message too long */
#define	EPROTOTYPE	41		/* Protocol wrong type for socket */
#define	ENOPROTOOPT	42		/* Protocol not available */
#define	EPROTONOSUPPORT	43		/* Protocol not supported */
#define	ESOCKTNOSUPPORT	44		/* Socket type not supported */
#define	EOPNOTSUPP	45		/* Operation not supported */
#define	EPFNOSUPPORT	46		/* Protocol family not supported */
#define	EAFNOSUPPORT	47		/* Address family not supported */
#define	EADDRINUSE	48		/* Address already in use */
#define	EADDRNOTAVAIL	49		/* Can't assign requested address */
#define	ENETDOWN	50		/* Network is down */
#define	ENETUNREACH	51		/* Network is unreachable */
#define	ENETRESET	52		/* Network dropped connection on reset */
#define	ECONNABORTED	53		/* Software caused connection abort */
#define	ECONNRESET	54		/* Connection reset by peer */
#define	ENOBUFS		55		/* No buffer space available */
#define	EISCONN		56		/* Socket is already connected */
#define	ENOTCONN	57		/* Socket is not connected */
#define	ESHUTDOWN	58		/* Can't send after socket shutdown */
#define	ETOOMANYREFS	59		/* Too many references: can't splice */
#define	ETIMEDOUT	60		/* Operation timed out */
#define	ECONNREFUSED	61		/* Connection refused */
#define	ELOOP		62		/* Too many levels of symbolic links */
#define	ENAMETOOLONG	63		/* File name too long */
#define	EHOSTDOWN	64		/* Host is down */
#define	EHOSTUNREACH	65		/* No route to host */
#define	ENOTEMPTY	66		/* Directory not empty */
#define	EPROCLIM	67		/* Too many processes */
#define	EUSERS		68		/* Too many users */
#define	EDQUOT		69		/* Disc quota exceeded */
#define	ESTALE		70		/* Stale NFS file handle */
#define	EREMOTE		71		/* Too many levels of remote in path */
#define	EBADRPC		72		/* RPC struct is bad */
#define	ERPCMISMATCH	73		/* RPC version wrong */
#define	EPROGUNAVAIL	74		/* RPC prog. not avail */
#define	EPROGMISMATCH	75		/* Program version wrong */
#define	EPROCUNAVAIL	76		/* Bad procedure for program */
#define	ENOLCK		77		/* No locks available */
#define	ENOSYS		78		/* Function not implemented */
#define	EFTYPE		79		/* Inappropriate file type or format */
#define	EAUTH		80		/* Authentication error */
#define	ENEEDAUTH	81		/* Need authenticator */
#define	EIDRM		82		/* Identifier removed */
#define	ENOMSG		83		/* No message of desired type */
#define	EOVERFLOW	84		/* Value too large for data type */
#define	ECANCELED	85		/* Operation canceled */
#define	EILSEQ		86		/* Illegal byte sequence */
#define	ENOATTR		87		/* Attribute not found */
#define	EDOOFUS		88		/* Programming error */
#define	EBADMSG		89		/* Bad message */
#define	EMULTIHOP	90		/* Multihop attempted */
#define	ENOLINK		91		/* Link has been severed */
#define	EPROTO		92		/* Protocol error */
#define	ENOTCAPABLE	93		/* Capabilities insufficient */
#define	ECAPMODE	94		/* Not permitted in capability mode */
#define	ENOTRECOVERABLE	95		/* State not recoverable */
#define	EOWNERDEAD	96		/* Previous owner died */
#define	EINTEGRITY	97		/* Integrity check failed */
#define	ELAST		97		/* Must be equal largest errno */

/* ------------------------------------------------------------------ */
/* sys/compat/linux/linux_errno.h				      */
/* ------------------------------------------------------------------ */

#define	LINUX_EPERM		1
#define	LINUX_ENOENT		2
#define	LINUX_ESRCH		3
#define	LINUX_EINTR		4
#define	LINUX_EIO		5
#define	LINUX_ENXIO		6
#define	LINUX_E2BIG		7
#define	LINUX_ENOEXEC		8
#define	LINUX_EBADF		9

#define	LINUX_ECHILD		10
#define	LINUX_EAGAIN		11
#define	LINUX_ENOMEM		12
#define	LINUX_EACCES		13
#define	LINUX_EFAULT		14
#define	LINUX_ENOTBLK		15
#define	LINUX_EBUSY		16
#define	LINUX_EEXIST		17
#define	LINUX_EXDEV		18
#define	LINUX_ENODEV		19

#define	LINUX_ENOTDIR		20
#define	LINUX_EISDIR		21
#define	LINUX_EINVAL		22
#define	LINUX_ENFILE		23
#define	LINUX_EMFILE		24
#define	LINUX_ENOTTY		25
#define	LINUX_ETXTBSY		26
#define	LINUX_EFBIG		27
#define	LINUX_ENOSPC		28
#define	LINUX_ESPIPE		29

#define	LINUX_EROFS		30
#define	LINUX_EMLINK		31
#define	LINUX_EPIPE		32
#define	LINUX_EDOM		33
#define	LINUX_ERANGE		34
#define	LINUX_EDEADLK		35
#define	LINUX_ENAMETOOLONG	36
#define	LINUX_ENOLCK		37
#define	LINUX_ENOSYS		38
#define	LINUX_ENOTEMPTY		39

#define	LINUX_ELOOP		40
/* XXX: errno 41 is not defined in Linux. */
#define	LINUX_ENOMSG		42
#define	LINUX_EIDRM		43
#define	LINUX_ECHRNG		44
#define	LINUX_EL2NSYNC		45
#define	LINUX_EL3HLT		46
#define	LINUX_EL3RST		47
#define	LINUX_ELNRNG		48
#define	LINUX_EUNATCH		49

#define	LINUX_ENOCSI		50
#define	LINUX_EL2HLT		51
#define	LINUX_EBADE		52
#define	LINUX_EBADR		53
#define	LINUX_EXFULL		54
#define	LINUX_ENOANO		55
#define	LINUX_EBADRQC		56
#define	LINUX_EBADSLT		57
/* XXX: errno 58 is not defined in Linux. */
#define	LINUX_EBFONT		59

#define	LINUX_ENOSTR		60
#define	LINUX_ENODATA		61
#define	LINUX_ENOTIME		62
#define	LINUX_ENOSR		63
#define	LINUX_ENONET		64
#define	LINUX_ENOPKG		65
#define	LINUX_EREMOTE		66
#define	LINUX_ENOLINK		67
#define	LINUX_EADV		68
#define	LINUX_ESRMNT		69

#define	LINUX_ECOMM		70
#define	LINUX_EPROTO		71
#define	LINUX_EMULTIHOP		72
#define	LINUX_EDOTDOT		73
#define	LINUX_EBADMSG		74
#define	LINUX_EOVERFLOW		75
#define	LINUX_ENOTUNIQ		76
#define	LINUX_EBADFD		77
#define	LINUX_EREMCHG		78
#define	LINUX_ELIBACC		79

#define	LINUX_ELIBBAD		80
#define	LINUX_ELIBSCN		81
#define	LINUX_ELIBMAX		82
#define	LINUX_ELIBEXEC		83
#define	LINUX_EILSEQ		84
#define	LINUX_ERESTART		85
#define	LINUX_ESTRPIPE		86
#define	LINUX_EUSERS		87
#define	LINUX_ENOTSOCK		88
#define	LINUX_EDESTADDRREQ	89

#define	LINUX_EMSGSIZE		90
#define	LINUX_EPROTOTYPE	91
#define	LINUX_ENOPROTOOPT	92
#define	LINUX_EPROTONOTSUPPORT	93
#define	LINUX_ESOCKNOTSUPPORT	94
#define	LINUX_EOPNOTSUPPORT	95
#define	LINUX_EPFNOTSUPPORT	96
#define	LINUX_EAFNOTSUPPORT	97
#define	LINUX_EADDRINUSE	98
#define	LINUX_EADDRNOTAVAIL	99

#define	LINUX_ENETDOWN		100
#define	LINUX_ENETUNREACH	101
#define	LINUX_ENETRESET		102
#define	LINUX_ECONNABORTED	103
#define	LINUX_ECONNRESET	104
#define	LINUX_ENOBUFS		105
#define	LINUX_EISCONN		106
#define	LINUX_ENOTCONN		107
#define	LINUX_ESHUTDOWN		108
#define	LINUX_ETOOMANYREFS	109

#define	LINUX_ETIMEDOUT		110
#define	LINUX_ECONNREFUSED	111
#define	LINUX_EHOSTDOWN		112
#define	LINUX_EHOSTUNREACH	113
#define	LINUX_EALREADY		114
#define	LINUX_EINPROGRESS	115
#define	LINUX_ESTALE		116
#define	LINUX_EUCLEAN		117
#define	LINUX_ENOTNAM		118
#define	LINUX_ENAVAIL		119

#define	LINUX_EISNAM		120
#define	LINUX_EREMOTEIO		121
#define	LINUX_EDQUOT		122
#define	LINUX_ENOMEDIUM		123
#define	LINUX_EMEDIUMTYPE	124
#define	LINUX_ECANCELED		125
#define	LINUX_ENOKEY		126
#define	LINUX_EKEYEXPIRED	127
#define	LINUX_EKEYREVOKED	128
#define	LINUX_EKEYREJECTED	129

#define	LINUX_EOWNERDEAD	130
#define	LINUX_ENOTRECOVERABLE	131
#define	LINUX_ERFKILL		132
#define	LINUX_EHWPOISON		133

#define	LINUX_ELAST		LINUX_EHWPOISON

/* ------------------------------------------------------------------ */
/* sys/compat/linux/linux_misc.h				      */
/* ------------------------------------------------------------------ */

#define	LINUX_PATH_MAX		4096

/* ------------------------------------------------------------------ */
/* kernel primitives						      */
/* ------------------------------------------------------------------ */

#define	INVARIANTS	1

#define	__predict_false(exp)	__builtin_expect((exp), 0)
#define	nitems(x)		(sizeof((x)) / sizeof((x)[0]))

typedef unsigned long	l_ulong;
typedef long		register_t;

struct thread {
	register_t	td_retval[2];
};

struct linux_getcwd_args {
	char		*buf;
	l_ulong		bufsize;
};

extern void hbsd_kassert_observe(int cond);
extern void panic(const char *fmt, ...)
    __attribute__((__noreturn__, __format__(__printf__, 1, 2)));
extern void *hbsd_kmalloc(size_t size, int type, int flags);
extern void hbsd_kfree(void *addr, int type);
extern int vn_getcwd(char *buf, char **retbuf, size_t *buflen);
extern int copyout(const void *kaddr, void *uaddr, size_t len);

#define	KASSERT(exp, msg) do {						\
	int hbsd_cond_ = (exp) ? 1 : 0;					\
	hbsd_kassert_observe(hbsd_cond_);				\
	if (__predict_false(!hbsd_cond_))				\
		panic msg;						\
} while (0)

#define	M_TEMP		0x544d5000
#define	M_WAITOK	0x0002

#define	malloc(size, type, flags)	hbsd_kmalloc((size), (type), (flags))
#define	free(addr, type)		hbsd_kfree((addr), (type))

/* ================================================================== */
/* ORIGINAL SOURCES						      */
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1994-1996 Soren Schmidt
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Linux syscalls return negative errno's, we do positive and map them
 * Reference:
 *   FreeBSD: src/sys/sys/errno.h
 *   Linux:   include/uapi/asm-generic/errno-base.h
 *            include/uapi/asm-generic/errno.h
 *
 * XXX: The "XXX" comments below should be replaced with rationale
 *      for the errno value chosen.
 */
static const int linux_errtbl[ELAST + 1] = {
	/* [0, 9] */
	[0] = -0,
	[EPERM] = -LINUX_EPERM,
	[ENOENT] = -LINUX_ENOENT,
	[ESRCH] = -LINUX_ESRCH,
	[EINTR] = -LINUX_EINTR,
	[EIO] = -LINUX_EIO,
	[ENXIO] = -LINUX_ENXIO,
	[E2BIG] = -LINUX_E2BIG,
	[ENOEXEC] = -LINUX_ENOEXEC,
	[EBADF] = -LINUX_EBADF,

	/* [10, 19] */
	[ECHILD] = -LINUX_ECHILD,
	[EDEADLK] = -LINUX_EDEADLK,
	[ENOMEM] = -LINUX_ENOMEM,
	[EACCES] = -LINUX_EACCES,
	[EFAULT] = -LINUX_EFAULT,
	[ENOTBLK] = -LINUX_ENOTBLK,
	[EBUSY] = -LINUX_EBUSY,
	[EEXIST] = -LINUX_EEXIST,
	[EXDEV] = -LINUX_EXDEV,
	[ENODEV] = -LINUX_ENODEV,

	/* [20, 29] */
	[ENOTDIR] = -LINUX_ENOTDIR,
	[EISDIR] = -LINUX_EISDIR,
	[EINVAL] = -LINUX_EINVAL,
	[ENFILE] = -LINUX_ENFILE,
	[EMFILE] = -LINUX_EMFILE,
	[ENOTTY] = -LINUX_ENOTTY,
	[ETXTBSY] = -LINUX_ETXTBSY,
	[EFBIG] = -LINUX_EFBIG,
	[ENOSPC] = -LINUX_ENOSPC,
	[ESPIPE] = -LINUX_ESPIPE,

	/* [30, 39] */
	[EROFS] = -LINUX_EROFS,
	[EMLINK] = -LINUX_EMLINK,
	[EPIPE] = -LINUX_EPIPE,
	[EDOM] = -LINUX_EDOM,
	[ERANGE] = -LINUX_ERANGE,
	[EAGAIN] = -LINUX_EAGAIN,
	[EINPROGRESS] = -LINUX_EINPROGRESS,
	[EALREADY] = -LINUX_EALREADY,
	[ENOTSOCK] = -LINUX_ENOTSOCK,
	[EDESTADDRREQ] = -LINUX_EDESTADDRREQ,

	/* [40, 49] */
	[EMSGSIZE] = -LINUX_EMSGSIZE,
	[EPROTOTYPE] = -LINUX_EPROTOTYPE,
	[ENOPROTOOPT] = -LINUX_ENOPROTOOPT,
	[EPROTONOSUPPORT] = -LINUX_EPROTONOTSUPPORT,
	[ESOCKTNOSUPPORT] = -LINUX_ESOCKNOTSUPPORT,
	[EOPNOTSUPP] = -LINUX_EOPNOTSUPPORT,
	[EPFNOSUPPORT] = -LINUX_EPFNOTSUPPORT,
	[EAFNOSUPPORT] = -LINUX_EAFNOTSUPPORT,
	[EADDRINUSE] = -LINUX_EADDRINUSE,
	[EADDRNOTAVAIL] = -LINUX_EADDRNOTAVAIL,

	/* [50, 59] */
	[ENETDOWN] = -LINUX_ENETDOWN,
	[ENETUNREACH] = -LINUX_ENETUNREACH,
	[ENETRESET] = -LINUX_ENETRESET,
	[ECONNABORTED] = -LINUX_ECONNABORTED,
	[ECONNRESET] = -LINUX_ECONNRESET,
	[ENOBUFS] = -LINUX_ENOBUFS,
	[EISCONN] = -LINUX_EISCONN,
	[ENOTCONN] = -LINUX_ENOTCONN,
	[ESHUTDOWN] = -LINUX_ESHUTDOWN,
	[ETOOMANYREFS] = -LINUX_ETOOMANYREFS,

	/* [60, 69] */
	[ETIMEDOUT] = -LINUX_ETIMEDOUT,
	[ECONNREFUSED] = -LINUX_ECONNREFUSED,
	[ELOOP] = -LINUX_ELOOP,
	[ENAMETOOLONG] = -LINUX_ENAMETOOLONG,
	[EHOSTDOWN] = -LINUX_EHOSTDOWN,
	[EHOSTUNREACH] = -LINUX_EHOSTUNREACH,
	[ENOTEMPTY] = -LINUX_ENOTEMPTY,
	[EPROCLIM] = -LINUX_EAGAIN,	/* XXX */
	[EUSERS] = -LINUX_EUSERS,
	[EDQUOT] = -LINUX_EDQUOT,

	/* [70, 79] */
	[ESTALE] = -LINUX_ESTALE,
	[EREMOTE] = -LINUX_EREMOTE,
	[EBADRPC] = -LINUX_ENXIO,	/* XXX */
	[ERPCMISMATCH] = -LINUX_ENXIO,	/* XXX */
	[EPROGUNAVAIL] = -LINUX_ENXIO,	/* XXX */
	[EPROGMISMATCH] = -LINUX_ENXIO,	/* XXX */
	[EPROCUNAVAIL] = -LINUX_ENXIO,	/* XXX */
	[ENOLCK] = -LINUX_ENOLCK,
	[ENOSYS] = -LINUX_ENOSYS,
	[EFTYPE] = -LINUX_EBADF,	/* XXX */

	/* [80, 89] */
	[EAUTH] = -LINUX_ENXIO,		/* XXX */
	[ENEEDAUTH] = -LINUX_ENXIO,	/* XXX */
	[EIDRM] = -LINUX_EIDRM,
	[ENOMSG] = -LINUX_ENOMSG,
	[EOVERFLOW] = -LINUX_EOVERFLOW,
	[ECANCELED] = -LINUX_ECANCELED,
	[EILSEQ] = -LINUX_EILSEQ,
	[ENOATTR] = -LINUX_ENODATA,	/* XXX */
	[EDOOFUS] = -LINUX_EINVAL,	/* XXX */
	[EBADMSG] = -LINUX_EBADMSG,

	/* [90, 99] */
	[EMULTIHOP] = -LINUX_EMULTIHOP,
	[ENOLINK] = -LINUX_ENOLINK,
	[EPROTO] = -LINUX_EPROTO,
	[ENOTCAPABLE] = -LINUX_EPERM,	/* XXX */
	[ECAPMODE] = -LINUX_EPERM,	/* XXX */
	[ENOTRECOVERABLE] = -LINUX_ENOTRECOVERABLE,
	[EOWNERDEAD] = -LINUX_EOWNERDEAD,
	[EINTEGRITY] = -LINUX_EINVAL,	/* XXX */
};

_Static_assert(ELAST == 97,
    "missing errno entries in linux_errtbl");

static const int linux_to_bsd_errtbl[LINUX_ELAST + 1] = {
	/* [0, 9] */
	[0] = 0,
	[LINUX_EPERM] = EPERM,
	[LINUX_ENOENT] = ENOENT,
	[LINUX_ESRCH] = ESRCH,
	[LINUX_EINTR] = EINTR,
	[LINUX_EIO] = EIO,
	[LINUX_ENXIO] = ENXIO,
	[LINUX_E2BIG] = E2BIG,
	[LINUX_ENOEXEC] = ENOENT,
	[LINUX_EBADF] = EBADF,

	/* [10, 19] */
	[LINUX_ECHILD] = ECHILD,
	[LINUX_EAGAIN] = EAGAIN,
	[LINUX_ENOMEM] = ENOMEM,
	[LINUX_EACCES] = EACCES,
	[LINUX_EFAULT] = EFAULT,
	[LINUX_ENOTBLK] = ENOTBLK,
	[LINUX_EBUSY] = EBUSY,
	[LINUX_EEXIST] = EEXIST,
	[LINUX_EXDEV] = EXDEV,
	[LINUX_ENODEV] = ENODEV,

	/* [20, 29] */
	[LINUX_ENOTDIR] = ENOTDIR,
	[LINUX_EISDIR] = EISDIR,
	[LINUX_EINVAL] = EINVAL,
	[LINUX_ENFILE] = ENFILE,
	[LINUX_EMFILE] = EMFILE,
	[LINUX_ENOTTY] = ENOTTY,
	[LINUX_ETXTBSY] = ETXTBSY,
	[LINUX_EFBIG] = EFBIG,
	[LINUX_ENOSPC] = ENOSPC,
	[LINUX_ESPIPE] = ESPIPE,

	/* [30, 39] */
	[LINUX_EROFS] = EROFS,
	[LINUX_EMLINK] = EMLINK,
	[LINUX_EPIPE] = EPIPE,
	[LINUX_EDOM] = EDOM,
	[LINUX_ERANGE] = ERANGE,
	[LINUX_EDEADLK] = EDEADLK,
	[LINUX_ENAMETOOLONG] = ENAMETOOLONG,
	[LINUX_ENOLCK] = ENOLCK,
	[LINUX_ENOSYS] = ENOSYS,
	[LINUX_ENOTEMPTY] = ENOTEMPTY,

	/* [40, 49] */
	[LINUX_ELOOP] = ELOOP,
	[41] = EINVAL,
	[LINUX_ENOMSG] = ENOMSG,
	[LINUX_EIDRM] = EIDRM,
	[LINUX_ECHRNG] = EINVAL,	/* XXX */
	[LINUX_EL2NSYNC] = EINVAL,	/* XXX */
	[LINUX_EL3HLT] = EINVAL,	/* XXX */
	[LINUX_EL3RST] = EINVAL,	/* XXX */
	[LINUX_ELNRNG] = EINVAL,	/* XXX */
	[LINUX_EUNATCH] = EINVAL,	/* XXX */

	/* [50, 59] */
	[LINUX_ENOCSI] = EINVAL,	/* XXX */
	[LINUX_EL2HLT] = EINVAL,	/* XXX */
	[LINUX_EBADE] = EINVAL,		/* XXX */
	[LINUX_EBADR] = EINVAL,		/* XXX */
	[LINUX_EXFULL] = EINVAL,	/* XXX */
	[LINUX_ENOANO] = EINVAL,	/* XXX */
	[LINUX_EBADRQC] = EINVAL,	/* XXX */
	[LINUX_EBADSLT] = EINVAL,	/* XXX */
	[58] = EINVAL,
	[LINUX_EBFONT] = EINVAL,	/* XXX */

	/* [60, 69] */
	[LINUX_ENOSTR] = EINVAL,	/* XXX */
	[LINUX_ENODATA] = ENOATTR,	/* XXX */
	[LINUX_ENOTIME] = EINVAL,	/* XXX */
	[LINUX_ENOSR] = EINVAL,		/* XXX */
	[LINUX_ENONET] = EINVAL,	/* XXX */
	[LINUX_ENOPKG] = EINVAL,	/* XXX */
	[LINUX_EREMOTE] = EREMOTE,
	[LINUX_ENOLINK] = ENOLINK,
	[LINUX_EADV] = EINVAL,		/* XXX */
	[LINUX_ESRMNT] = EINVAL,	/* XXX */

	/* [70, 79] */
	[LINUX_ECOMM] = EINVAL,		/* XXX */
	[LINUX_EPROTO] = EPROTO,
	[LINUX_EMULTIHOP] = EMULTIHOP,
	[LINUX_EDOTDOT] = EINVAL,	/* XXX */
	[LINUX_EBADMSG] = EBADMSG,
	[LINUX_EOVERFLOW] = EOVERFLOW,
	[LINUX_ENOTUNIQ] = EINVAL,	/* XXX */
	[LINUX_EBADFD] = EBADF,		/* XXX */
	[LINUX_EREMCHG] = EINVAL,	/* XXX */
	[LINUX_ELIBACC] = EINVAL,	/* XXX */

	/* [80, 89] */
	[LINUX_ELIBBAD] = EINVAL,	/* XXX */
	[LINUX_ELIBSCN] = EINVAL,	/* XXX */
	[LINUX_ELIBMAX] = EINVAL,	/* XXX */
	[LINUX_ELIBEXEC] = EINVAL,	/* XXX */
	[LINUX_EILSEQ] = EILSEQ,
	[LINUX_ERESTART] = EAGAIN,	/* XXX */
	[LINUX_ESTRPIPE] = EINVAL,	/* XXX */
	[LINUX_EUSERS] = EUSERS,
	[LINUX_ENOTSOCK] = ENOTSOCK,
	[LINUX_EDESTADDRREQ] = EDESTADDRREQ,

	/* [90, 99] */
	[LINUX_EMSGSIZE] = EMSGSIZE,
	[LINUX_EPROTOTYPE] = EPROTOTYPE,
	[LINUX_ENOPROTOOPT] = ENOPROTOOPT,
	[LINUX_EPROTONOTSUPPORT] = EPROTONOSUPPORT,
	[LINUX_ESOCKNOTSUPPORT] = EPROTONOSUPPORT,	/* XXX */
	[LINUX_EOPNOTSUPPORT] = EOPNOTSUPP,
	[LINUX_EPFNOTSUPPORT] = EPFNOSUPPORT,
	[LINUX_EAFNOTSUPPORT] = EAFNOSUPPORT,
	[LINUX_EADDRINUSE] = EADDRINUSE,
	[LINUX_EADDRNOTAVAIL] = EADDRNOTAVAIL,

	/* [100, 109] */
	[LINUX_ENETDOWN] = ENETDOWN,
	[LINUX_ENETUNREACH] = ENETUNREACH,
	[LINUX_ENETRESET] = ENETRESET,
	[LINUX_ECONNABORTED] = ECONNABORTED,
	[LINUX_ECONNRESET] = ECONNRESET,
	[LINUX_ENOBUFS] = ENOBUFS,
	[LINUX_EISCONN] = EISCONN,
	[LINUX_ENOTCONN] = ENOTCONN,
	[LINUX_ESHUTDOWN] = ESHUTDOWN,
	[LINUX_ETOOMANYREFS] = ETOOMANYREFS,

	/* [110, 119] */
	[LINUX_ETIMEDOUT] = ETIMEDOUT,
	[LINUX_ECONNREFUSED] = ECONNREFUSED,
	[LINUX_EHOSTDOWN] = EHOSTDOWN,
	[LINUX_EHOSTUNREACH] = EHOSTUNREACH,
	[LINUX_EALREADY] = EALREADY,
	[LINUX_EINPROGRESS] = EINPROGRESS,
	[LINUX_ESTALE] = ESTALE,
	[LINUX_EUCLEAN] = EINVAL,		/* XXX */
	[LINUX_ENOTNAM] = EINVAL,		/* XXX */
	[LINUX_ENAVAIL] = EINVAL,		/* XXX */

	/* [120, 129] */
	[LINUX_EISNAM] = EINVAL,		/* XXX */
	[LINUX_EREMOTEIO] = EINVAL,		/* XXX */
	[LINUX_EDQUOT] = EDQUOT,
	[LINUX_ENOMEDIUM] = EIO,		/* XXX */
	[LINUX_EMEDIUMTYPE] = EIO,		/* XXX */
	[LINUX_ECANCELED] = ECANCELED,
	[LINUX_ENOKEY] = EIO,			/* XXX */
	[LINUX_EKEYEXPIRED] = EIO,		/* XXX */
	[LINUX_EKEYREVOKED] = EIO,		/* XXX */
	[LINUX_EKEYREJECTED] = EIO,		/* XXX */

	/* [130, 139] */
	[LINUX_EOWNERDEAD] = EOWNERDEAD,
	[LINUX_ENOTRECOVERABLE] = ENOTRECOVERABLE,
	[LINUX_ERFKILL] = ENETDOWN,		/* XXX */
	[LINUX_EHWPOISON] = EINVAL,		/* XXX */
};

_Static_assert(LINUX_ELAST == 133,
    "missing errno entries in linux_to_bsd_errtbl");

/* --- sys/compat/linux/linux_errno.c ------------------------------- */

int
ref_bsd_to_linux_errno(int error)
{

	KASSERT(error >= 0 && error <= ELAST,
	    ("%s: bad error %d", __func__, error));

	return (linux_errtbl[error]);
}

#ifdef INVARIANTS
void
ref_linux_check_errtbl(void)
{
	int i;

	for (i = 1; i < nitems(linux_errtbl); i++) {
		KASSERT(linux_errtbl[i] != 0,
		    ("%s: linux_errtbl[%d] == 0", __func__, i));
	}

	for (i = 1; i < nitems(linux_to_bsd_errtbl); i++) {
		KASSERT(linux_to_bsd_errtbl[i] != 0,
		    ("%s: linux_to_bsd_errtbl[%d] == 0", __func__, i));
	}

}
#endif

/* --- sys/compat/linux/linux_getcwd.c ------------------------------ */

/* $OpenBSD: linux_getcwd.c,v 1.2 2001/05/16 12:50:21 ho Exp $ */
/* $NetBSD: vfs_getcwd.c,v 1.3.2.3 1999/07/11 10:24:09 sommerfeld Exp $ */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * Copyright (c) 2015 The FreeBSD Foundation
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Bill Sommerfeld.
 *
 * Portions of this software were developed by Edward Tomasz Napierala
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Find pathname of process's current directory.
 */
int
ref_linux_getcwd(struct thread *td, struct linux_getcwd_args *uap)
{
	char *buf, *retbuf;
	size_t buflen;
	int error;

	buflen = uap->bufsize;
	if (__predict_false(buflen < 2))
		return (ERANGE);
	if (buflen > LINUX_PATH_MAX)
		buflen = LINUX_PATH_MAX;

	buf = malloc(buflen, M_TEMP, M_WAITOK);
	error = vn_getcwd(buf, &retbuf, &buflen);
	if (error == ENOMEM)
		error = ERANGE;
	if (error == 0) {
		error = copyout(retbuf, uap->buf, buflen);
		if (error == 0)
			td->td_retval[0] = buflen;
	}
	free(buf, M_TEMP);
	return (error);
}
