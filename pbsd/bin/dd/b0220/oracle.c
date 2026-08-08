/*
 * oracle.c -- reference implementation for PBSD batch b0220.
 * Original C sources concatenated; every function renamed ref_<name>.
 * Bodies otherwise unmodified.  Mechanical shims only.
 */

#define _GNU_SOURCE

#include <sys/param.h>
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef nitems
#define nitems(x) (sizeof(x) / sizeof((x)[0]))
#endif

typedef unsigned char u_char;
typedef unsigned int u_int;

typedef struct {
	u_char		*db;
	u_char		*dbp;
	ssize_t		dbcnt;
	ssize_t		dbrcnt;
	ssize_t		dbsz;
#define	ISCHR		0x01
#define	ISPIPE		0x02
#define	ISTAPE		0x04
#define	ISSEEK		0x08
#define	NOREAD		0x10
#define	ISTRUNC		0x20
	u_int		flags;
	const char	*name;
	int		fd;
	off_t		offset;
	off_t		seek_offset;
} IO;

typedef struct {
	uintmax_t	in_full;
	uintmax_t	in_part;
	uintmax_t	out_full;
	uintmax_t	out_part;
	uintmax_t	trunc;
	uintmax_t	swab;
	uintmax_t	bytes;
	struct timespec	start;
} STAT;

#define	C_ASCII		0x0000000000000001ULL
#define	C_BLOCK		0x0000000000000002ULL
#define	C_BS		0x0000000000000004ULL
#define	C_CBS		0x0000000000000008ULL
#define	C_COUNT		0x0000000000000010ULL
#define	C_EBCDIC	0x0000000000000020ULL
#define	C_FILES		0x0000000000000040ULL
#define	C_IBS		0x0000000000000080ULL
#define	C_IF		0x0000000000000100ULL
#define	C_LCASE		0x0000000000000200ULL
#define	C_NOERROR	0x0000000000000400ULL
#define	C_NOTRUNC	0x0000000000000800ULL
#define	C_OBS		0x0000000000001000ULL
#define	C_OF		0x0000000000002000ULL
#define	C_OSYNC		0x0000000000004000ULL
#define	C_PAREVEN	0x0000000000008000ULL
#define	C_PARNONE	0x0000000000010000ULL
#define	C_PARODD	0x0000000000020000ULL
#define	C_PARSET	0x0000000000040000ULL
#define	C_SEEK		0x0000000000080000ULL
#define	C_SKIP		0x0000000000100000ULL
#define	C_SPARSE	0x0000000000200000ULL
#define	C_SWAB		0x0000000000400000ULL
#define	C_SYNC		0x0000000000800000ULL
#define	C_UCASE		0x0000000001000000ULL
#define	C_UNBLOCK	0x0000000002000000ULL
#define	C_FILL		0x0000000004000000ULL
#define	C_STATUS	0x0000000008000000ULL
#define	C_NOXFER	0x0000000010000000ULL
#define	C_NOINFO	0x0000000020000000ULL
#define	C_PROGRESS	0x0000000040000000ULL
#define	C_FSYNC		0x0000000080000000ULL
#define	C_FDATASYNC	0x0000000100000000ULL
#define	C_OFSYNC	0x0000000200000000ULL
#define	C_IFULLBLOCK	0x0000000400000000ULL
#define	C_IDIRECT	0x0000000800000000ULL
#define	C_ODIRECT	0x0000001000000000ULL

#define	C_PARITY	(C_PAREVEN | C_PARODD | C_PARNONE | C_PARSET)

IO in, out;
STAT st;
void (*cfunc)(void);
uintmax_t cpy_cnt;
size_t cbsz;
uint64_t ddflags;
size_t speed;
uintmax_t files_cnt;
const u_char *ctab;
char fill_char;
volatile sig_atomic_t need_summary;
volatile sig_atomic_t need_progress;
volatile sig_atomic_t kill_signal;

char *oper;

jmp_buf b0220_jmp;
int b0220_err_exit;

void
err(int eval, const char *fmt, ...)
{
	(void)fmt;
	b0220_err_exit = eval;
	longjmp(b0220_jmp, 1);
}

void
errx(int eval, const char *fmt, ...)
{
	(void)fmt;
	b0220_err_exit = eval;
	longjmp(b0220_jmp, 1);
}

void
errc(int eval, int code, const char *fmt, ...)
{
	(void)code;
	(void)fmt;
	b0220_err_exit = eval;
	longjmp(b0220_jmp, 1);
}

void
warnx(const char *fmt, ...)
{
	(void)fmt;
}

void
warn(const char *fmt, ...)
{
	(void)fmt;
}

void def(void) {}
void block(void) {}
void unblock(void) {}

void
b0220_reset_state(void)
{
	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));
	memset(&st, 0, sizeof(st));
	cfunc = def;
	cpy_cnt = 0;
	cbsz = 0;
	ddflags = 0;
	speed = 0;
	files_cnt = 1;
	ctab = NULL;
	fill_char = 0;
	oper = NULL;
	b0220_err_exit = 0;
}

/* conv_tab.c data (no functions in source file) */
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego and Lance
 * Visser of Convex Computer Corporation.
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
 * There are currently six tables:
 *
 *	ebcdic		-> ascii	32V		conv=oldascii
 *	ascii		-> ebcdic	32V		conv=oldebcdic
 *	ascii		-> ibm ebcdic	32V		conv=oldibm
 *
 *	ebcdic		-> ascii	POSIX/S5	conv=ascii
 *	ascii		-> ebcdic	POSIX/S5	conv=ebcdic
 *	ascii		-> ibm ebcdic	POSIX/S5	conv=ibm
 *
 * Other tables are built from these if multiple conversions are being
 * done.
 *
 * Tables used for conversions to/from IBM and EBCDIC to support an extension
 * to POSIX P1003.2/D11. The tables referencing POSIX contain data extracted
 * from tables 4-3 and 4-4 in P1003.2/Draft 11.  The historic tables were
 * constructed by running against a file with all possible byte values.
 *
 * More information can be obtained in "Correspondences of 8-Bit and Hollerith
 * Codes for Computer Environments-A USASI Tutorial", Communications of the
 * ACM, Volume 11, Number 11, November 1968, pp. 783-789.
 */

u_char casetab[256];

/* EBCDIC to ASCII -- 32V compatible. */
const u_char e2a_32V[] = {
	0000, 0001, 0002, 0003, 0234, 0011, 0206, 0177,		/* 0000 */
	0227, 0215, 0216, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0235, 0205, 0010, 0207,		/* 0020 */
	0030, 0031, 0222, 0217, 0034, 0035, 0036, 0037,		/* 0030 */
	0200, 0201, 0202, 0203, 0204, 0012, 0027, 0033,		/* 0040 */
	0210, 0211, 0212, 0213, 0214, 0005, 0006, 0007,		/* 0050 */
	0220, 0221, 0026, 0223, 0224, 0225, 0226, 0004,		/* 0060 */
	0230, 0231, 0232, 0233, 0024, 0025, 0236, 0032,		/* 0070 */
	0040, 0240, 0241, 0242, 0243, 0244, 0245, 0246,		/* 0100 */
	0247, 0250, 0133, 0056, 0074, 0050, 0053, 0041,		/* 0110 */
	0046, 0251, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0120 */
	0260, 0261, 0135, 0044, 0052, 0051, 0073, 0136,		/* 0130 */
	0055, 0057, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0140 */
	0270, 0271, 0174, 0054, 0045, 0137, 0076, 0077,		/* 0150 */
	0272, 0273, 0274, 0275, 0276, 0277, 0300, 0301,		/* 0160 */
	0302, 0140, 0072, 0043, 0100, 0047, 0075, 0042,		/* 0170 */
	0303, 0141, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0200 */
	0150, 0151, 0304, 0305, 0306, 0307, 0310, 0311,		/* 0210 */
	0312, 0152, 0153, 0154, 0155, 0156, 0157, 0160,		/* 0220 */
	0161, 0162, 0313, 0314, 0315, 0316, 0317, 0320,		/* 0230 */
	0321, 0176, 0163, 0164, 0165, 0166, 0167, 0170,		/* 0240 */
	0171, 0172, 0322, 0323, 0324, 0325, 0326, 0327,		/* 0250 */
	0330, 0331, 0332, 0333, 0334, 0335, 0336, 0337,		/* 0260 */
	0340, 0341, 0342, 0343, 0344, 0345, 0346, 0347,		/* 0270 */
	0173, 0101, 0102, 0103, 0104, 0105, 0106, 0107,		/* 0300 */
	0110, 0111, 0350, 0351, 0352, 0353, 0354, 0355,		/* 0310 */
	0175, 0112, 0113, 0114, 0115, 0116, 0117, 0120,		/* 0320 */
	0121, 0122, 0356, 0357, 0360, 0361, 0362, 0363,		/* 0330 */
	0134, 0237, 0123, 0124, 0125, 0126, 0127, 0130,		/* 0340 */
	0131, 0132, 0364, 0365, 0366, 0367, 0370, 0371,		/* 0350 */
	0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067,		/* 0360 */
	0070, 0071, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* ASCII to EBCDIC -- 32V compatible. */
const u_char a2e_32V[] = {
	0000, 0001, 0002, 0003, 0067, 0055, 0056, 0057,		/* 0000 */
	0026, 0005, 0045, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0074, 0075, 0062, 0046,		/* 0020 */
	0030, 0031, 0077, 0047, 0034, 0035, 0036, 0037,		/* 0030 */
	0100, 0117, 0177, 0173, 0133, 0154, 0120, 0175,		/* 0040 */
	0115, 0135, 0134, 0116, 0153, 0140, 0113, 0141,		/* 0050 */
	0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,		/* 0060 */
	0370, 0371, 0172, 0136, 0114, 0176, 0156, 0157,		/* 0070 */
	0174, 0301, 0302, 0303, 0304, 0305, 0306, 0307,		/* 0100 */
	0310, 0311, 0321, 0322, 0323, 0324, 0325, 0326,		/* 0110 */
	0327, 0330, 0331, 0342, 0343, 0344, 0345, 0346,		/* 0120 */
	0347, 0350, 0351, 0112, 0340, 0132, 0137, 0155,		/* 0130 */
	0171, 0201, 0202, 0203, 0204, 0205, 0206, 0207,		/* 0140 */
	0210, 0211, 0221, 0222, 0223, 0224, 0225, 0226,		/* 0150 */
	0227, 0230, 0231, 0242, 0243, 0244, 0245, 0246,		/* 0160 */
	0247, 0250, 0251, 0300, 0152, 0320, 0241, 0007,		/* 0170 */
	0040, 0041, 0042, 0043, 0044, 0025, 0006, 0027,		/* 0200 */
	0050, 0051, 0052, 0053, 0054, 0011, 0012, 0033,		/* 0210 */
	0060, 0061, 0032, 0063, 0064, 0065, 0066, 0010,		/* 0220 */
	0070, 0071, 0072, 0073, 0004, 0024, 0076, 0341,		/* 0230 */
	0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110,		/* 0240 */
	0111, 0121, 0122, 0123, 0124, 0125, 0126, 0127,		/* 0250 */
	0130, 0131, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0260 */
	0150, 0151, 0160, 0161, 0162, 0163, 0164, 0165,		/* 0270 */
	0166, 0167, 0170, 0200, 0212, 0213, 0214, 0215,		/* 0300 */
	0216, 0217, 0220, 0232, 0233, 0234, 0235, 0236,		/* 0310 */
	0237, 0240, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0320 */
	0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0330 */
	0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,		/* 0340 */
	0312, 0313, 0314, 0315, 0316, 0317, 0332, 0333,		/* 0350 */
	0334, 0335, 0336, 0337, 0352, 0353, 0354, 0355,		/* 0360 */
	0356, 0357, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* ASCII to IBM EBCDIC -- 32V compatible. */
const u_char a2ibm_32V[] = {
	0000, 0001, 0002, 0003, 0067, 0055, 0056, 0057,		/* 0000 */
	0026, 0005, 0045, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0074, 0075, 0062, 0046,		/* 0020 */
	0030, 0031, 0077, 0047, 0034, 0035, 0036, 0037,		/* 0030 */
	0100, 0132, 0177, 0173, 0133, 0154, 0120, 0175,		/* 0040 */
	0115, 0135, 0134, 0116, 0153, 0140, 0113, 0141,		/* 0050 */
	0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,		/* 0060 */
	0370, 0371, 0172, 0136, 0114, 0176, 0156, 0157,		/* 0070 */
	0174, 0301, 0302, 0303, 0304, 0305, 0306, 0307,		/* 0100 */
	0310, 0311, 0321, 0322, 0323, 0324, 0325, 0326,		/* 0110 */
	0327, 0330, 0331, 0342, 0343, 0344, 0345, 0346,		/* 0120 */
	0347, 0350, 0351, 0255, 0340, 0275, 0137, 0155,		/* 0130 */
	0171, 0201, 0202, 0203, 0204, 0205, 0206, 0207,		/* 0140 */
	0210, 0211, 0221, 0222, 0223, 0224, 0225, 0226,		/* 0150 */
	0227, 0230, 0231, 0242, 0243, 0244, 0245, 0246,		/* 0160 */
	0247, 0250, 0251, 0300, 0117, 0320, 0241, 0007,		/* 0170 */
	0040, 0041, 0042, 0043, 0044, 0025, 0006, 0027,		/* 0200 */
	0050, 0051, 0052, 0053, 0054, 0011, 0012, 0033,		/* 0210 */
	0060, 0061, 0032, 0063, 0064, 0065, 0066, 0010,		/* 0220 */
	0070, 0071, 0072, 0073, 0004, 0024, 0076, 0341,		/* 0230 */
	0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110,		/* 0240 */
	0111, 0121, 0122, 0123, 0124, 0125, 0126, 0127,		/* 0250 */
	0130, 0131, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0260 */
	0150, 0151, 0160, 0161, 0162, 0163, 0164, 0165,		/* 0270 */
	0166, 0167, 0170, 0200, 0212, 0213, 0214, 0215,		/* 0300 */
	0216, 0217, 0220, 0232, 0233, 0234, 0235, 0236,		/* 0310 */
	0237, 0240, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0320 */
	0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0330 */
	0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,		/* 0340 */
	0312, 0313, 0314, 0315, 0316, 0317, 0332, 0333,		/* 0350 */
	0334, 0335, 0336, 0337, 0352, 0353, 0354, 0355,		/* 0360 */
	0356, 0357, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* EBCDIC to ASCII -- POSIX and System V compatible. */
const u_char e2a_POSIX[] = {
	0000, 0001, 0002, 0003, 0234, 0011, 0206, 0177,		/* 0000 */
	0227, 0215, 0216, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0235, 0205, 0010, 0207,		/* 0020 */
	0030, 0031, 0222, 0217, 0034, 0035, 0036, 0037,		/* 0030 */
	0200, 0201, 0202, 0203, 0204, 0012, 0027, 0033,		/* 0040 */
	0210, 0211, 0212, 0213, 0214, 0005, 0006, 0007,		/* 0050 */
	0220, 0221, 0026, 0223, 0224, 0225, 0226, 0004,		/* 0060 */
	0230, 0231, 0232, 0233, 0024, 0025, 0236, 0032,		/* 0070 */
	0040, 0240, 0241, 0242, 0243, 0244, 0245, 0246,		/* 0100 */
	0247, 0250, 0325, 0056, 0074, 0050, 0053, 0174,		/* 0110 */
	0046, 0251, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0120 */
	0260, 0261, 0041, 0044, 0052, 0051, 0073, 0176,		/* 0130 */
	0055, 0057, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0140 */
	0270, 0271, 0313, 0054, 0045, 0137, 0076, 0077,		/* 0150 */
	0272, 0273, 0274, 0275, 0276, 0277, 0300, 0301,		/* 0160 */
	0302, 0140, 0072, 0043, 0100, 0047, 0075, 0042,		/* 0170 */
	0303, 0141, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0200 */
	0150, 0151, 0304, 0305, 0306, 0307, 0310, 0311,		/* 0210 */
	0312, 0152, 0153, 0154, 0155, 0156, 0157, 0160,		/* 0220 */
	0161, 0162, 0136, 0314, 0315, 0316, 0317, 0320,		/* 0230 */
	0321, 0345, 0163, 0164, 0165, 0166, 0167, 0170,		/* 0240 */
	0171, 0172, 0322, 0323, 0324, 0133, 0326, 0327,		/* 0250 */
	0330, 0331, 0332, 0333, 0334, 0335, 0336, 0337,		/* 0260 */
	0340, 0341, 0342, 0343, 0344, 0135, 0346, 0347,		/* 0270 */
	0173, 0101, 0102, 0103, 0104, 0105, 0106, 0107,		/* 0300 */
	0110, 0111, 0350, 0351, 0352, 0353, 0354, 0355,		/* 0310 */
	0175, 0112, 0113, 0114, 0115, 0116, 0117, 0120,		/* 0320 */
	0121, 0122, 0356, 0357, 0360, 0361, 0362, 0363,		/* 0330 */
	0134, 0237, 0123, 0124, 0125, 0126, 0127, 0130,		/* 0340 */
	0131, 0132, 0364, 0365, 0366, 0367, 0370, 0371,		/* 0350 */
	0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067,		/* 0360 */
	0070, 0071, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* ASCII to EBCDIC -- POSIX and System V compatible. */
const u_char a2e_POSIX[] = {
	0000, 0001, 0002, 0003, 0067, 0055, 0056, 0057,		/* 0000 */
	0026, 0005, 0045, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0074, 0075, 0062, 0046,		/* 0020 */
	0030, 0031, 0077, 0047, 0034, 0035, 0036, 0037,		/* 0030 */
	0100, 0132, 0177, 0173, 0133, 0154, 0120, 0175,		/* 0040 */
	0115, 0135, 0134, 0116, 0153, 0140, 0113, 0141,		/* 0050 */
	0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,		/* 0060 */
	0370, 0371, 0172, 0136, 0114, 0176, 0156, 0157,		/* 0070 */
	0174, 0301, 0302, 0303, 0304, 0305, 0306, 0307,		/* 0100 */
	0310, 0311, 0321, 0322, 0323, 0324, 0325, 0326,		/* 0110 */
	0327, 0330, 0331, 0342, 0343, 0344, 0345, 0346,		/* 0120 */
	0347, 0350, 0351, 0255, 0340, 0275, 0232, 0155,		/* 0130 */
	0171, 0201, 0202, 0203, 0204, 0205, 0206, 0207,		/* 0140 */
	0210, 0211, 0221, 0222, 0223, 0224, 0225, 0226,		/* 0150 */
	0227, 0230, 0231, 0242, 0243, 0244, 0245, 0246,		/* 0160 */
	0247, 0250, 0251, 0300, 0117, 0320, 0137, 0007,		/* 0170 */
	0040, 0041, 0042, 0043, 0044, 0025, 0006, 0027,		/* 0200 */
	0050, 0051, 0052, 0053, 0054, 0011, 0012, 0033,		/* 0210 */
	0060, 0061, 0032, 0063, 0064, 0065, 0066, 0010,		/* 0220 */
	0070, 0071, 0072, 0073, 0004, 0024, 0076, 0341,		/* 0230 */
	0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110,		/* 0240 */
	0111, 0121, 0122, 0123, 0124, 0125, 0126, 0127,		/* 0250 */
	0130, 0131, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0260 */
	0150, 0151, 0160, 0161, 0162, 0163, 0164, 0165,		/* 0270 */
	0166, 0167, 0170, 0200, 0212, 0213, 0214, 0215,		/* 0300 */
	0216, 0217, 0220, 0152, 0233, 0234, 0235, 0236,		/* 0310 */
	0237, 0240, 0252, 0253, 0254, 0112, 0256, 0257,		/* 0320 */
	0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0330 */
	0270, 0271, 0272, 0273, 0274, 0241, 0276, 0277,		/* 0340 */
	0312, 0313, 0314, 0315, 0316, 0317, 0332, 0333,		/* 0350 */
	0334, 0335, 0336, 0337, 0352, 0353, 0354, 0355,		/* 0360 */
	0356, 0357, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* ASCII to IBM EBCDIC -- POSIX and System V compatible. */
const u_char a2ibm_POSIX[] = {
	0000, 0001, 0002, 0003, 0067, 0055, 0056, 0057,		/* 0000 */
	0026, 0005, 0045, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0074, 0075, 0062, 0046,		/* 0020 */
	0030, 0031, 0077, 0047, 0034, 0035, 0036, 0037,		/* 0030 */
	0100, 0132, 0177, 0173, 0133, 0154, 0120, 0175,		/* 0040 */
	0115, 0135, 0134, 0116, 0153, 0140, 0113, 0141,		/* 0050 */
	0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,		/* 0060 */
	0370, 0371, 0172, 0136, 0114, 0176, 0156, 0157,		/* 0070 */
	0174, 0301, 0302, 0303, 0304, 0305, 0306, 0307,		/* 0100 */
	0310, 0311, 0321, 0322, 0323, 0324, 0325, 0326,		/* 0110 */
	0327, 0330, 0331, 0342, 0343, 0344, 0345, 0346,		/* 0120 */
	0347, 0350, 0351, 0255, 0340, 0275, 0137, 0155,		/* 0130 */
	0171, 0201, 0202, 0203, 0204, 0205, 0206, 0207,		/* 0140 */
	0210, 0211, 0221, 0222, 0223, 0224, 0225, 0226,		/* 0150 */
	0227, 0230, 0231, 0242, 0243, 0244, 0245, 0246,		/* 0160 */
	0247, 0250, 0251, 0300, 0117, 0320, 0241, 0007,		/* 0170 */
	0040, 0041, 0042, 0043, 0044, 0025, 0006, 0027,		/* 0200 */
	0050, 0051, 0052, 0053, 0054, 0011, 0012, 0033,		/* 0210 */
	0060, 0061, 0032, 0063, 0064, 0065, 0066, 0010,		/* 0220 */
	0070, 0071, 0072, 0073, 0004, 0024, 0076, 0341,		/* 0230 */
	0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110,		/* 0240 */
	0111, 0121, 0122, 0123, 0124, 0125, 0126, 0127,		/* 0250 */
	0130, 0131, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0260 */
	0150, 0151, 0160, 0161, 0162, 0163, 0164, 0165,		/* 0270 */
	0166, 0167, 0170, 0200, 0212, 0213, 0214, 0215,		/* 0300 */
	0216, 0217, 0220, 0232, 0233, 0234, 0235, 0236,		/* 0310 */
	0237, 0240, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0320 */
	0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0330 */
	0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,		/* 0340 */
	0312, 0313, 0314, 0315, 0316, 0317, 0332, 0333,		/* 0350 */
	0334, 0335, 0336, 0337, 0352, 0353, 0354, 0355,		/* 0360 */
	0356, 0357, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};


/* args.c */
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego and Lance
 * Visser of Convex Computer Corporation.
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




int	ref_c_arg(const void *, const void *);
int	ref_c_conv(const void *, const void *);
int	ref_c_iflag(const void *, const void *);
int	ref_c_oflag(const void *, const void *);
void	ref_f_bs(char *);
void	ref_f_cbs(char *);
void	ref_f_conv(char *);
void	ref_f_count(char *);
void	ref_f_files(char *);
void	ref_f_fillchar(char *);
void	ref_f_ibs(char *);
void	ref_f_if(char *);
void	ref_f_iflag(char *);
void	ref_f_obs(char *);
void	ref_f_of(char *);
void	ref_f_oflag(char *);
void	ref_f_seek(char *);
void	ref_f_skip(char *);
void	ref_f_speed(char *);
void	ref_f_status(char *);
uintmax_t ref_get_num(const char *);
off_t	ref_get_off_t(const char *);

const struct arg {
	const char *name;
	void (*f)(char *);
	uint64_t set, noset;
} args[] = {
	{ "bs",		ref_f_bs,		C_BS,	 C_BS|C_IBS|C_OBS|C_OSYNC },
	{ "cbs",	ref_f_cbs,		C_CBS,	 C_CBS },
	{ "conv",	ref_f_conv,		0,	 0 },
	{ "count",	ref_f_count,	C_COUNT, C_COUNT },
	{ "files",	ref_f_files,	C_FILES, C_FILES },
	{ "fillchar",	ref_f_fillchar,	C_FILL,	 C_FILL },
	{ "ibs",	ref_f_ibs,		C_IBS,	 C_BS|C_IBS },
	{ "if",		ref_f_if,		C_IF,	 C_IF },
	{ "iflag",	ref_f_iflag,	0,	 0 },
	{ "iseek",	ref_f_skip,		C_SKIP,	 C_SKIP },
	{ "obs",	ref_f_obs,		C_OBS,	 C_BS|C_OBS },
	{ "of",		ref_f_of,		C_OF,	 C_OF },
	{ "oflag",	ref_f_oflag,	0,	 0 },
	{ "oseek",	ref_f_seek,		C_SEEK,	 C_SEEK },
	{ "seek",	ref_f_seek,		C_SEEK,	 C_SEEK },
	{ "skip",	ref_f_skip,		C_SKIP,	 C_SKIP },
	{ "speed",	ref_f_speed,	0,	 0 },
	{ "status",	ref_f_status,	C_STATUS,C_STATUS },
};

/* oper defined in shim */

/*
 * args -- parse JCL syntax of dd.
 */
void
ref_jcl(char **argv)
{
	struct arg *ap, tmp;
	char *arg;

	in.dbsz = out.dbsz = 512;

	while ((oper = *++argv) != NULL) {
		if ((oper = strdup(oper)) == NULL)
			errx(1, "unable to allocate space for the argument \"%s\"", *argv);
		if ((arg = strchr(oper, '=')) == NULL)
			errx(1, "unknown operand %s", oper);
		*arg++ = '\0';
		if (!*arg)
			errx(1, "no value specified for %s", oper);
		tmp.name = oper;
		if (!(ap = (struct arg *)bsearch(&tmp, args,
		    sizeof(args)/sizeof(struct arg), sizeof(struct arg),
		    ref_c_arg)))
			errx(1, "unknown operand %s", tmp.name);
		if (ddflags & ap->noset)
			errx(1, "%s: illegal argument combination or already set",
			    tmp.name);
		ddflags |= ap->set;
		ap->f(arg);
	}

	/* Final sanity checks. */

	if (ddflags & C_BS) {
		/*
		 * Bs is turned off by any conversion -- we assume the user
		 * just wanted to set both the input and output block sizes
		 * and didn't want the bs semantics, so we don't warn.
		 */
		if (ddflags & (C_BLOCK | C_LCASE | C_SWAB | C_UCASE |
		    C_UNBLOCK))
			ddflags &= ~C_BS;

		/* Bs supersedes ibs and obs. */
		if (ddflags & C_BS && ddflags & (C_IBS | C_OBS))
			warnx("bs supersedes ibs and obs");
	}

	/*
	 * Ascii/ebcdic and cbs implies block/unblock.
	 * Block/unblock requires cbs and vice-versa.
	 */
	if (ddflags & (C_BLOCK | C_UNBLOCK)) {
		if (!(ddflags & C_CBS))
			errx(1, "record operations require cbs");
		if (cbsz == 0)
			errx(1, "cbs cannot be zero");
		cfunc = ddflags & C_BLOCK ? block : unblock;
	} else if (ddflags & C_CBS) {
		if (ddflags & (C_ASCII | C_EBCDIC)) {
			if (ddflags & C_ASCII) {
				ddflags |= C_UNBLOCK;
				cfunc = unblock;
			} else {
				ddflags |= C_BLOCK;
				cfunc = block;
			}
		} else
			errx(1, "cbs meaningless if not doing record operations");
	} else
		cfunc = def;
}

int
ref_c_arg(const void *a, const void *b)
{

	return (strcmp(((const struct arg *)a)->name,
	    ((const struct arg *)b)->name));
}

void
ref_f_bs(char *arg)
{
	uintmax_t res;

	res = ref_get_num(arg);
	if (res < 1 || res > SSIZE_MAX)
		errx(1, "bs must be between 1 and %zd", (ssize_t)SSIZE_MAX);
	in.dbsz = out.dbsz = (size_t)res;
}

void
ref_f_cbs(char *arg)
{
	uintmax_t res;

	res = ref_get_num(arg);
	if (res < 1 || res > SSIZE_MAX)
		errx(1, "cbs must be between 1 and %zd", (ssize_t)SSIZE_MAX);
	cbsz = (size_t)res;
}

void
ref_f_count(char *arg)
{
	uintmax_t res;

	res = ref_get_num(arg);
	if (res == UINTMAX_MAX)
		errc(1, ERANGE, "%s", oper);
	if (res == 0)
		cpy_cnt = UINTMAX_MAX;
	else
		cpy_cnt = res;
}

void
ref_f_files(char *arg)
{

	files_cnt = ref_get_num(arg);
	if (files_cnt < 1)
		errx(1, "files must be between 1 and %zu", SIZE_MAX);
}

void
ref_f_fillchar(char *arg)
{

	if (strlen(arg) != 1)
		errx(1, "need exactly one fill char");

	fill_char = arg[0];
}

void
ref_f_ibs(char *arg)
{
	uintmax_t res;

	if (!(ddflags & C_BS)) {
		res = ref_get_num(arg);
		if (res < 1 || res > SSIZE_MAX)
			errx(1, "ibs must be between 1 and %zd",
			    (ssize_t)SSIZE_MAX);
		in.dbsz = (size_t)res;
	}
}

void
ref_f_if(char *arg)
{

	in.name = arg;
}

const struct iflag {
	const char *name;
	uint64_t set, noset;
} ilist[] = {
	{ "direct",	C_IDIRECT,	0 },
	{ "fullblock",	C_IFULLBLOCK,	C_SYNC },
};

void
ref_f_iflag(char *arg)
{
	struct iflag *ip, tmp;

	while (arg != NULL) {
		tmp.name = strsep(&arg, ",");
		ip = bsearch(&tmp, ilist, nitems(ilist), sizeof(struct iflag),
		    ref_c_iflag);
		if (ip == NULL)
			errx(1, "unknown iflag %s", tmp.name);
		if (ddflags & ip->noset)
			errx(1, "%s: illegal conversion combination", tmp.name);
		ddflags |= ip->set;
	}
}

int
ref_c_iflag(const void *a, const void *b)
{

	return (strcmp(((const struct iflag *)a)->name,
	    ((const struct iflag *)b)->name));
}

void
ref_f_obs(char *arg)
{
	uintmax_t res;

	if (!(ddflags & C_BS)) {
		res = ref_get_num(arg);
		if (res < 1 || res > SSIZE_MAX)
			errx(1, "obs must be between 1 and %zd",
			    (ssize_t)SSIZE_MAX);
		out.dbsz = (size_t)res;
	}
}

void
ref_f_of(char *arg)
{

	out.name = arg;
}

void
ref_f_seek(char *arg)
{

	out.offset = ref_get_off_t(arg);
}

void
ref_f_skip(char *arg)
{

	in.offset = ref_get_off_t(arg);
}

void
ref_f_speed(char *arg)
{

	speed = ref_get_num(arg);
}

void
ref_f_status(char *arg)
{

	if (strcmp(arg, "none") == 0)
		ddflags |= C_NOINFO;
	else if (strcmp(arg, "noxfer") == 0)
		ddflags |= C_NOXFER;
	else if (strcmp(arg, "progress") == 0)
		ddflags |= C_PROGRESS;
	else
		errx(1, "unknown status %s", arg);
}
 
const struct conv {
	const char *name;
	uint64_t set, noset;
	const u_char *ctab;
} clist[] = {
	{ "ascii",	C_ASCII,	C_EBCDIC,	e2a_POSIX },
	{ "block",	C_BLOCK,	C_UNBLOCK,	NULL },
	{ "ebcdic",	C_EBCDIC,	C_ASCII,	a2e_POSIX },
	{ "fdatasync",	C_FDATASYNC,	0,		NULL },
	{ "fsync",	C_FSYNC,	0,		NULL },
	{ "ibm",	C_EBCDIC,	C_ASCII,	a2ibm_POSIX },
	{ "lcase",	C_LCASE,	C_UCASE,	NULL },
	{ "noerror",	C_NOERROR,	0,		NULL },
	{ "notrunc",	C_NOTRUNC,	0,		NULL },
	{ "oldascii",	C_ASCII,	C_EBCDIC,	e2a_32V },
	{ "oldebcdic",	C_EBCDIC,	C_ASCII,	a2e_32V },
	{ "oldibm",	C_EBCDIC,	C_ASCII,	a2ibm_32V },
	{ "osync",	C_OSYNC,	C_BS,		NULL },
	{ "pareven",	C_PAREVEN,	C_PARODD|C_PARSET|C_PARNONE, NULL},
	{ "parnone",	C_PARNONE,	C_PARODD|C_PARSET|C_PAREVEN, NULL},
	{ "parodd",	C_PARODD,	C_PAREVEN|C_PARSET|C_PARNONE, NULL},
	{ "parset",	C_PARSET,	C_PARODD|C_PAREVEN|C_PARNONE, NULL},
	{ "sparse",	C_SPARSE,	0,		NULL },
	{ "swab",	C_SWAB,		0,		NULL },
	{ "sync",	C_SYNC,		C_IFULLBLOCK,	NULL },
	{ "ucase",	C_UCASE,	C_LCASE,	NULL },
	{ "unblock",	C_UNBLOCK,	C_BLOCK,	NULL },
};

void
ref_f_conv(char *arg)
{
	struct conv *cp, tmp;

	while (arg != NULL) {
		tmp.name = strsep(&arg, ",");
		cp = bsearch(&tmp, clist, nitems(clist), sizeof(struct conv),
		    ref_c_conv);
		if (cp == NULL)
			errx(1, "unknown conversion %s", tmp.name);
		if (ddflags & cp->noset)
			errx(1, "%s: illegal conversion combination", tmp.name);
		ddflags |= cp->set;
		if (cp->ctab)
			ctab = cp->ctab;
	}
}

int
ref_c_conv(const void *a, const void *b)
{

	return (strcmp(((const struct conv *)a)->name,
	    ((const struct conv *)b)->name));
}

const struct oflag {
	const char *name;
	uint64_t set;
} olist[] = {
	{ "direct",	C_ODIRECT },
	{ "fsync",	C_OFSYNC },
	{ "sync",	C_OFSYNC },
};

void
ref_f_oflag(char *arg)
{
	struct oflag *op, tmp;

	while (arg != NULL) {
		tmp.name = strsep(&arg, ",");
		op = bsearch(&tmp, olist, nitems(olist), sizeof(struct oflag),
		    ref_c_oflag);
		if (op == NULL)
			errx(1, "unknown open flag %s", tmp.name);
		ddflags |= op->set;
	}
}

int
ref_c_oflag(const void *a, const void *b)
{

	return (strcmp(((const struct oflag *)a)->name,
	    ((const struct oflag *)b)->name));
}

intmax_t
ref_postfix_to_mult(const char expr)
{
	intmax_t mult;

	mult = 0;
	switch (expr) {
	case 'B':
	case 'b':
		mult = 512;
		break;
	case 'K':
	case 'k':
		mult = 1 << 10;
		break;
	case 'M':
	case 'm':
		mult = 1 << 20;
		break;
	case 'G':
	case 'g':
		mult = 1 << 30;
		break;
	case 'T':
	case 't':
		mult = (uintmax_t)1 << 40;
		break;
	case 'P':
	case 'p':
		mult = (uintmax_t)1 << 50;
		break;
	case 'W':
	case 'w':
		mult = sizeof(int);
		break;
	}

	return (mult);
}

/*
 * Convert an expression of the following forms to a uintmax_t.
 * 	1) A positive decimal number.
 *	2) A positive decimal number followed by a 'b' or 'B' (mult by 512).
 *	3) A positive decimal number followed by a 'k' or 'K' (mult by 1 << 10).
 *	4) A positive decimal number followed by a 'm' or 'M' (mult by 1 << 20).
 *	5) A positive decimal number followed by a 'g' or 'G' (mult by 1 << 30).
 *	6) A positive decimal number followed by a 't' or 'T' (mult by 1 << 40).
 *	7) A positive decimal number followed by a 'p' or 'P' (mult by 1 << 50).
 *	8) A positive decimal number followed by a 'w' or 'W' (mult by sizeof int).
 *	9) Two or more positive decimal numbers (with/without [BbKkMmGgWw])
 *	   separated by 'x' or 'X' (also '*' for backwards compatibility),
 *	   specifying the product of the indicated values.
 */
uintmax_t
ref_get_num(const char *val)
{
	uintmax_t num, mult, prevnum;
	char *expr;

	errno = 0;
	num = strtoumax(val, &expr, 0);
	if (expr == val)			/* No valid digits. */
		errx(1, "%s: invalid numeric value", oper);
	if (errno != 0)
		err(1, "%s", oper);

	mult = ref_postfix_to_mult(*expr);

	if (mult != 0) {
		prevnum = num;
		num *= mult;
		/* Check for overflow. */
		if (num / mult != prevnum)
			goto erange;
		expr++;
	}

	switch (*expr) {
		case '\0':
			break;
		case '*':			/* Backward compatible. */
		case 'X':
		case 'x':
			mult = ref_get_num(expr + 1);
			prevnum = num;
			num *= mult;
			if (num / mult == prevnum)
				break;
erange:
			errx(1, "%s: %s", oper, strerror(ERANGE));
		default:
			errx(1, "%s: illegal numeric value", oper);
	}
	return (num);
}

/*
 * Convert an expression of the following forms to an off_t.  This is the
 * same as ref_get_num(), but it uses signed numbers.
 *
 * The major problem here is that an off_t may not necessarily be a intmax_t.
 */
off_t
ref_get_off_t(const char *val)
{
	intmax_t num, mult, prevnum;
	char *expr;

	errno = 0;
	num = strtoimax(val, &expr, 0);
	if (expr == val)			/* No valid digits. */
		errx(1, "%s: invalid numeric value", oper);
	if (errno != 0)
		err(1, "%s", oper);

	mult = ref_postfix_to_mult(*expr);

	if (mult != 0) {
		prevnum = num;
		num *= mult;
		/* Check for overflow. */
		if ((prevnum > 0) != (num > 0) || num / mult != prevnum)
			goto erange;
		expr++;
	}

	switch (*expr) {
		case '\0':
			break;
		case '*':			/* Backward compatible. */
		case 'X':
		case 'x':
			mult = (intmax_t)ref_get_off_t(expr + 1);
			prevnum = num;
			num *= mult;
			if ((prevnum > 0) == (num > 0) && num / mult == prevnum)
				break;
erange:
			errx(1, "%s: %s", oper, strerror(ERANGE));
		default:
			errx(1, "%s: illegal numeric value", oper);
	}
	return (num);
}


/* dd.c */
int
ref_parity(u_char c)
{
	int i;

	i = c ^ (c >> 1) ^ (c >> 2) ^ (c >> 3) ^ 
	    (c >> 4) ^ (c >> 5) ^ (c >> 6) ^ (c >> 7);
	return (i & 1);
}

void
ref_swapbytes(void *v, size_t len)
{
	unsigned char *p = v;
	unsigned char t;

	while (len > 1) {
		t = p[0];
		p[0] = p[1];
		p[1] = t;
		p += 2;
		len -= 2;
	}
}
