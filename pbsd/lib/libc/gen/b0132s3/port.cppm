/*
 * b0132s3 -- C++23 port of HardenedBSD lib/libc/gen/disklabel.c.
 *
 * This file is also #include'd by harness.cpp (and by itself) with
 * PBSD_B0132S3_SHARED defined, which selects only the block of shared ABI
 * declarations at the top.  GCC's module implementation refuses declarations
 * in a global module fragment unless they arrive through preprocessor
 * inclusion, hence the self-include.
 */

#ifdef PBSD_B0132S3_SHARED

#ifndef DEV_BSIZE
#define DEV_BSIZE	512
#endif

#define BSD_MAGIC		0x82564557U
#define BSD_NPARTS_MIN		8
#define BSD_BOOTBLOCK_SIZE	8192
#define BSD_NDRIVEDATA		5
#define BSD_NSPARE		5

#define DISKMAGIC	BSD_MAGIC
#define MAXPARTITIONS	BSD_NPARTS_MIN
#define BBSIZE		BSD_BOOTBLOCK_SIZE
#define NDDATA		BSD_NDRIVEDATA

#define D_REMOVABLE	0x01
#define D_ECC		0x02
#define D_BADSECT	0x04
#define D_RAMDISK	0x08
#define D_CHAIN		0x10

#define _PATH_DISKTAB	"/etc/disktab"

struct disklabel {
	uint32_t d_magic;
	uint16_t d_type;
	uint16_t d_subtype;
	char	 d_typename[16];
	char	 d_packname[16];
	uint32_t d_secsize;
	uint32_t d_nsectors;
	uint32_t d_ntracks;
	uint32_t d_ncylinders;
	uint32_t d_secpercyl;
	uint32_t d_secperunit;
	uint16_t d_sparespertrack;
	uint16_t d_sparespercyl;
	uint32_t d_acylinders;
	uint16_t d_rpm;
	uint16_t d_interleave;
	uint16_t d_trackskew;
	uint16_t d_cylskew;
	uint32_t d_headswitch;
	uint32_t d_trkseek;
	uint32_t d_flags;
	uint32_t d_drivedata[BSD_NDRIVEDATA];
	uint32_t d_spare[BSD_NSPARE];
	uint32_t d_magic2;
	uint16_t d_checksum;
	uint16_t d_npartitions;
	uint32_t d_bbsize;
	uint32_t d_sbsize;
	struct partition {
		uint32_t p_size;
		uint32_t p_offset;
		uint32_t p_fsize;
		uint8_t  p_fstype;
		uint8_t  p_frag;
		uint16_t p_cpg;
	} d_partitions[BSD_NPARTS_MIN];
};

#ifdef __cplusplus
extern "C" {
#endif
extern int cgetent(char **buf, char **db_array, const char *name);
extern int cgetstr(char *buf, const char *cap, char **str);
extern int cgetnum(char *buf, const char *cap, long *num);
extern char *cgetcap(char *buf, const char *cap, int type);
#ifdef __cplusplus
}
#endif

#else /* !PBSD_B0132S3_SHARED */

module;

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdio.h>

#define PBSD_B0132S3_SHARED 1
#include "port.cppm"
#undef PBSD_B0132S3_SHARED

export module pbsd.lib.libc.gen.b0132s3;

export namespace pbsd::lib_libc_gen::b0132s3 {

static const char *dktypenames[] = {
	"unknown",
	"SMD",
	"MSCP",
	"old DEC",
	"SCSI",
	"ESDI",
	"ST506",
	"HP-IB",
	"HP-FL",
	"type 9",
	"floppy",
	"CCD",
	"Vinum",
	"DOC2K",
	"Raid",
	"?",
	"jfs",
	NULL
};

static const char *fstypenames[] = {
	"unused",
	"swap",
	"Version 6",
	"Version 7",
	"System V",
	"4.1BSD",
	"Eighth Edition",
	"4.2BSD",
	"MSDOS",
	"4.4LFS",
	"unknown",
	"HPFS",
	"ISO9660",
	"boot",
	"vinum",
	"raid",
	"Filecore",
	"EXT2FS",
	"NTFS",
	"?",
	"ccd",
	"jfs",
	"HAMMER",
	"HAMMER2",
	"UDF",
	"?",
	"EFS",
	"ZFS",
	"?",
	"?",
	"nandfs",
	NULL
};

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1983, 1987, 1993
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

/* `static' in the original; exported here so the harness can reach it. */
int
gettype(char *t, const char **names)
{
	const char **nm;

	for (nm = names; *nm; nm++)
		if (strcasecmp(t, *nm) == 0)
			return (nm - names);
	if (isdigit((unsigned char)*t))
		return (atoi(t));
	return (0);
}

struct disklabel *
getdiskbyname(const char *name)
{
	static struct	disklabel disk;
	struct	disklabel *dp = &disk;
	struct partition *pp;
	char	*buf;
	char  	*db_array[2] = { _PATH_DISKTAB, 0 };
	char	*cp, *cq;	/* can't be register */
	char	p, max, psize[3], pbsize[3],
		pfsize[3], poffset[3], ptype[3];
	u_int32_t *dx;

	if (cgetent(&buf, db_array, (char *) name) < 0)
		return NULL;

	bzero((char *)&disk, sizeof(disk));
	/*
	 * typename
	 */
	cq = dp->d_typename;
	cp = buf;
	while (cq < dp->d_typename + sizeof(dp->d_typename) - 1 &&
	    (*cq = *cp) && *cq != '|' && *cq != ':')
		cq++, cp++;
	*cq = '\0';

	if (cgetstr(buf, "ty", &cq) > 0) {
		if (strcmp(cq, "removable") == 0)
			dp->d_flags |= D_REMOVABLE;
		else  if (cq && strcmp(cq, "simulated") == 0)
			dp->d_flags |= D_RAMDISK;
		free(cq);
	}
	if (cgetcap(buf, "sf", ':') != NULL)
		dp->d_flags |= D_BADSECT;

#define getnumdflt(field, dname, dflt) \
        { long f; (field) = (cgetnum(buf, dname, &f) == -1) ? (dflt) : f; }

	getnumdflt(dp->d_secsize, "se", DEV_BSIZE);
	getnumdflt(dp->d_ntracks, "nt", 0);
	getnumdflt(dp->d_nsectors, "ns", 0);
	getnumdflt(dp->d_ncylinders, "nc", 0);

	if (cgetstr(buf, "dt", &cq) > 0) {
		dp->d_type = gettype(cq, dktypenames);
		free(cq);
	} else
		getnumdflt(dp->d_type, "dt", 0);
	getnumdflt(dp->d_secpercyl, "sc", dp->d_nsectors * dp->d_ntracks);
	getnumdflt(dp->d_secperunit, "su", dp->d_secpercyl * dp->d_ncylinders);
	getnumdflt(dp->d_rpm, "rm", 3600);
	getnumdflt(dp->d_interleave, "il", 1);
	getnumdflt(dp->d_trackskew, "sk", 0);
	getnumdflt(dp->d_cylskew, "cs", 0);
	getnumdflt(dp->d_headswitch, "hs", 0);
	getnumdflt(dp->d_trkseek, "ts", 0);
	getnumdflt(dp->d_bbsize, "bs", BBSIZE);
	getnumdflt(dp->d_sbsize, "sb", 0);
	strcpy(psize, "px");
	strcpy(pbsize, "bx");
	strcpy(pfsize, "fx");
	strcpy(poffset, "ox");
	strcpy(ptype, "tx");
	max = 'a' - 1;
	pp = &dp->d_partitions[0];
	for (p = 'a'; p < 'a' + MAXPARTITIONS; p++, pp++) {
		long l;
		psize[1] = pbsize[1] = pfsize[1] = poffset[1] = ptype[1] = p;
		if (cgetnum(buf, psize, &l) == -1)
			pp->p_size = 0;
		else {
			pp->p_size = l;
			cgetnum(buf, poffset, &l);
			pp->p_offset = l;
			getnumdflt(pp->p_fsize, pfsize, 0);
			if (pp->p_fsize) {
				long bsize;

				if (cgetnum(buf, pbsize, &bsize) == 0)
					pp->p_frag = bsize / pp->p_fsize;
				else
					pp->p_frag = 8;
			}
			getnumdflt(pp->p_fstype, ptype, 0);
			if (pp->p_fstype == 0)
				if (cgetstr(buf, ptype, &cq) >= 0) {
					pp->p_fstype = gettype(cq, fstypenames);
					free(cq);
				}
			max = p;
		}
	}
	dp->d_npartitions = max + 1 - 'a';
	(void)strcpy(psize, "dx");
	dx = dp->d_drivedata;
	for (p = '0'; p < '0' + NDDATA; p++, dx++) {
		psize[1] = p;
		getnumdflt(*dx, psize, 0);
	}
	dp->d_magic = DISKMAGIC;
	dp->d_magic2 = DISKMAGIC;
	free(buf);
	return (dp);
}

} /* namespace pbsd::lib_libc_gen::b0132s3 */

#endif /* PBSD_B0132S3_SHARED */
