/*
 * b0132s3 differential harness for disklabel.c (gettype, getdiskbyname).
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <limits.h>

#define PBSD_B0132S3_SHARED 1
#include "port.cppm"
#undef PBSD_B0132S3_SHARED

import pbsd.lib.libc.gen.b0132s3;

namespace P = pbsd::lib_libc_gen::b0132s3;

extern "C" {
int ref_gettype(char *t, const char **names);
struct disklabel *ref_getdiskbyname(const char *name);
}

/* ------------------------------------------------------------------------ */
/* Minimal faithful cget* layer (cgetset + cgetent lookup, plus cgetcap,    */
/* cgetstr, cgetnum lifted from HardenedBSD getcap.c).  Shared by oracle   */
/* and port so both see identical capability parsing.                       */
/* ------------------------------------------------------------------------ */

#define	ESC		('[' & 037)
#define	SFRAG		100

static char *toprec;

extern "C" int
cgetset(const char *ent)
{
	if (ent == NULL) {
		free(toprec);
		toprec = NULL;
		return (0);
	}
	free(toprec);
	toprec = strdup(ent);
	return (toprec == NULL ? -1 : 0);
}

extern "C" int
cgetmatch(const char *buf, const char *name)
{
	const char *np, *bp;

	if (name == NULL || *name == '\0')
		return -1;

	bp = buf;
	for (;;) {
		np = name;
		for (;;)
			if (*np == '\0')
				if (*bp == '|' || *bp == ':' || *bp == '\0')
					return (0);
				else
					break;
			else
				if (*bp++ != *np++)
					break;

		bp--;
		for (;;)
			if (*bp == '\0' || *bp == ':')
				return (-1);
			else
				if (*bp++ == '|')
					break;
	}
}

extern "C" int
cgetent(char **buf, char **db_array, const char *name)
{
	(void)db_array;

	if (toprec != NULL && cgetmatch(toprec, name) == 0) {
		*buf = strdup(toprec);
		return (*buf == NULL ? -2 : 0);
	}
	return (-1);
}

extern "C" char *
cgetcap(char *buf, const char *cap, int type)
{
	char *bp;
	const char *cp;

	bp = buf;
	for (;;) {
		for (;;)
			if (*bp == '\0')
				return (NULL);
			else
				if (*bp++ == ':')
					break;

		for (cp = cap; *cp == *bp && *bp != '\0'; cp++, bp++)
			continue;
		if (*cp != '\0')
			continue;
		if (*bp == '@')
			return (NULL);
		if (type == ':') {
			if (*bp != '\0' && *bp != ':')
				continue;
			return(bp);
		}
		if (*bp != type)
			continue;
		bp++;
		return (*bp == '@' ? NULL : bp);
	}
}

extern "C" int
cgetstr(char *buf, const char *cap, char **str)
{
	u_int m_room;
	char *bp, *mp;
	int len;
	char *mem;

	bp = cgetcap(buf, cap, '=');
	if (bp == NULL)
		return (-1);

	if ((mem = (char *)malloc(SFRAG)) == NULL)
		return (-2);
	m_room = SFRAG;
	mp = mem;

	while (*bp != ':' && *bp != '\0') {
		if (*bp == '^') {
			bp++;
			if (*bp == ':' || *bp == '\0')
				break;
			if (*bp == '?') {
				*mp++ = '\177';
				bp++;
			} else
				*mp++ = *bp++ & 037;
		} else if (*bp == '\\') {
			bp++;
			if (*bp == ':' || *bp == '\0')
				break;
			if ('0' <= *bp && *bp <= '7') {
				int n, i;

				n = 0;
				i = 3;
				do {
					n = n * 8 + (*bp++ - '0');
				} while (--i && '0' <= *bp && *bp <= '7');
				*mp++ = n;
			}
			else switch (*bp++) {
				case 'b': case 'B':
					*mp++ = '\b';
					break;
				case 't': case 'T':
					*mp++ = '\t';
					break;
				case 'n': case 'N':
					*mp++ = '\n';
					break;
				case 'f': case 'F':
					*mp++ = '\f';
					break;
				case 'r': case 'R':
					*mp++ = '\r';
					break;
				case 'e': case 'E':
					*mp++ = ESC;
					break;
				case 'c': case 'C':
					*mp++ = ':';
					break;
				default:
					*mp++ = *(bp-1);
					break;
			}
		} else
			*mp++ = *bp++;
		m_room--;

		if (m_room == 0) {
			size_t size = mp - mem;

			mem = (char *)realloc(mem, size + SFRAG);
			if (mem == NULL)
				return (-2);
			m_room = SFRAG;
			mp = mem + size;
		}
	}
	*mp++ = '\0';
	m_room--;
	len = mp - mem - 1;

	if (m_room != 0) {
		mem = (char *)realloc(mem, (size_t)(mp - mem));
		if (mem == NULL)
			return (-2);
	}
	*str = mem;
	return (len);
}

extern "C" int
cgetnum(char *buf, const char *cap, long *num)
{
	long n;
	int base, digit;
	char *bp;

	bp = cgetcap(buf, cap, '#');
	if (bp == NULL)
		return (-1);

	if (*bp == '0') {
		bp++;
		if (*bp == 'x' || *bp == 'X') {
			bp++;
			base = 16;
		} else
			base = 8;
	} else
		base = 10;

	n = 0;
	for (;;) {
		if ('0' <= *bp && *bp <= '9')
			digit = *bp - '0';
		else if ('a' <= *bp && *bp <= 'f')
			digit = 10 + *bp - 'a';
		else if ('A' <= *bp && *bp <= 'F')
			digit = 10 + *bp - 'A';
		else
			break;

		if (digit >= base)
			break;

		n = n * base + digit;
		bp++;
	}

	*num = n;
	return (0);
}

/* ------------------------------------------------------------------------ */
/* Statistics.                                                              */
/* ------------------------------------------------------------------------ */

enum {
	S_GETTYPE = 0,
	S_GETDISKBYNAME,
	S_NFUNC
};

struct Stat {
	const char *name;
	long long cases;
	long long fails;
};

static Stat st[S_NFUNC] = {
	{ "gettype", 0, 0 },
	{ "getdiskbyname", 0, 0 },
};

static void
note(int idx, const char *what)
{
	st[idx].fails++;
	if (st[idx].fails <= 8)
		fprintf(stderr, "MISMATCH %-18s case #%lld: %s\n",
		    st[idx].name, st[idx].cases, what);
}

/* ------------------------------------------------------------------------ */
/* PRNG (splitmix64, fixed seed).                                           */
/* ------------------------------------------------------------------------ */

struct Rng {
	uint64_t s;
	explicit Rng(uint64_t seed) : s(seed) {}
	uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ULL;
		uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}
	uint32_t u32() { return (uint32_t)next(); }
	uint32_t below(uint32_t n) { return n ? u32() % n : 0; }
	int byte() { return (int)(next() & 0xff); }
};

/* ------------------------------------------------------------------------ */
/* gettype()                                                                */
/* ------------------------------------------------------------------------ */

#define GT_ALLOC	256
#define GT_NAMES	16
#define GT_NAMELEN	72

static unsigned char gtA[GT_ALLOC], gtB[GT_ALLOC];
static char gnA[GT_NAMES][GT_NAMELEN], gnB[GT_NAMES][GT_NAMELEN];
static const char *gpA[GT_NAMES + 1], *gpB[GT_NAMES + 1];

static void
check_gettype(const char *t, size_t tlen, const char *const *names, int nnames)
{
	st[S_GETTYPE].cases++;

	memset(gtA, 0x7f, GT_ALLOC);
	memset(gtB, 0x7f, GT_ALLOC);
	memcpy(gtA + 8, t, tlen);
	gtA[8 + tlen] = '\0';
	memcpy(gtB + 8, t, tlen);
	gtB[8 + tlen] = '\0';

	memset(gnA, 0x7f, sizeof gnA);
	memset(gnB, 0x7f, sizeof gnB);
	for (int i = 0; i < nnames; i++) {
		size_t l = strlen(names[i]);
		memcpy(gnA[i], names[i], l + 1);
		memcpy(gnB[i], names[i], l + 1);
		gpA[i] = gnA[i];
		gpB[i] = gnB[i];
	}
	gpA[nnames] = NULL;
	gpB[nnames] = NULL;

	int ra = ref_gettype((char *)gtA + 8, gpA);
	int rb = P::gettype((char *)gtB + 8, gpB);

	if (ra != rb) {
		char msg[160];
		snprintf(msg, sizeof msg, "ret %d vs %d (t=\"%s\", %d names)",
		    ra, rb, (const char *)gtA + 8, nnames);
		note(S_GETTYPE, msg);
		return;
	}
	if (memcmp(gtA, gtB, GT_ALLOC) != 0)
		note(S_GETTYPE, "argument buffer differs");
	if (memcmp(gnA, gnB, sizeof gnA) != 0)
		note(S_GETTYPE, "names buffer differs");
}

static void
hand_gettype(void)
{
	static const char *n0[] = { NULL };
	static const char *n1[] = { "unknown" };
	static const char *n4[] = { "unknown", "SMD", "ESDI", "SCSI" };
	static const char *nempty[] = { "a", "", "b" };
	static const char *nhigh[] = { "\x80\x81", "\xff", "A\xc3\xa9" };
	static const char *ndigit[] = { "0", "1", "12" };
	static const char *nlong[] = {
		"0123456789012345678901234567890",
		"01234567890123456789012345678901",
		"a"
	};

	struct { const char *t; const char *const *nm; int n; } tab[] = {
		{ "", n0, 0 },
		{ "a", n0, 0 },
		{ "0", n0, 0 },
		{ "9", n0, 0 },
		{ "5xyz", n0, 0 },
		{ "-5", n0, 0 },
		{ "+7", n0, 0 },
		{ " 3", n0, 0 },
		{ "2147483647", n0, 0 },
		{ "\x80", n0, 0 },
		{ "\xff\xfe", n0, 0 },

		{ "", n1, 1 },
		{ "unknown", n1, 1 },
		{ "UNKNOWN", n1, 1 },
		{ "UnKnOwN", n1, 1 },
		{ "unknow", n1, 1 },
		{ "unknownx", n1, 1 },
		{ "3", n1, 1 },

		{ "unknown", n4, 4 },
		{ "smd", n4, 4 },
		{ "SMD", n4, 4 },
		{ "esdi", n4, 4 },
		{ "scsi", n4, 4 },
		{ "SCSi", n4, 4 },
		{ "nope", n4, 4 },
		{ "2", n4, 4 },
		{ "3zzz", n4, 4 },
		{ "", n4, 4 },
		{ "\x80", n4, 4 },

		{ "", nempty, 3 },
		{ "a", nempty, 3 },
		{ "b", nempty, 3 },
		{ "c", nempty, 3 },

		{ "\x80\x81", nhigh, 3 },
		{ "\xff", nhigh, 3 },
		{ "A\xc3\xa9", nhigh, 3 },
		{ "a\xc3\xa9", nhigh, 3 },
		{ "\x80", nhigh, 3 },

		{ "0", ndigit, 3 },
		{ "1", ndigit, 3 },
		{ "12", ndigit, 3 },
		{ "13", ndigit, 3 },
		{ "007", ndigit, 3 },

		{ "0123456789012345678901234567890", nlong, 3 },
		{ "01234567890123456789012345678901", nlong, 3 },
		{ "012345678901234567890123456789012", nlong, 3 },
		{ "a", nlong, 3 },
	};

	for (size_t i = 0; i < sizeof tab / sizeof tab[0]; i++)
		check_gettype(tab[i].t, strlen(tab[i].t), tab[i].nm, tab[i].n);

	{
		char t[8] = { '\0', '7', '7', '\0' };
		check_gettype(t, 0, n4, 4);
	}
	{
		const char *nm[] = { "\x80" };
		check_gettype("\x80", 1, nm, 1);
	}
	{
		const char *nm[] = { "zz", "z", "" };
		check_gettype("", 0, nm, 3);
		check_gettype("z", 1, nm, 3);
		check_gettype("zz", 2, nm, 3);
		check_gettype("zzz", 3, nm, 3);
	}
}

static void
sweep_gettype(long long iters)
{
	Rng rng(0xB0132S3A1u);
	static const char alpha[] =
	    "abcABC019zZ\x80\xff\xa5 _-+";
	const size_t nalpha = sizeof alpha - 1;
	char names[GT_NAMES][GT_NAMELEN];
	const char *nm[GT_NAMES];
	char t[GT_NAMELEN];

	for (long long it = 0; it < iters; it++) {
		int nnames = (int)rng.below(GT_NAMES + 1);
		for (int i = 0; i < nnames; i++) {
			int l = (int)rng.below(14);
			for (int j = 0; j < l; j++)
				names[i][j] = alpha[rng.below((uint32_t)nalpha)];
			names[i][l] = '\0';
			nm[i] = names[i];
		}

		int mode = (int)rng.below(10);
		size_t tlen;
		if (mode < 4 && nnames > 0) {
			int j = (int)rng.below((uint32_t)nnames);
			tlen = strlen(names[j]);
			memcpy(t, names[j], tlen + 1);
			for (size_t k = 0; k < tlen; k++)
				if (rng.below(2)) {
					unsigned char ch = (unsigned char)t[k];
					if (isalpha(ch))
						t[k] = (char)(islower(ch) ?
						    toupper(ch) : tolower(ch));
				}
			if (rng.below(4) == 0 && tlen > 0)
				t[rng.below((uint32_t)tlen)] =
				    alpha[rng.below((uint32_t)nalpha)];
			else if (rng.below(8) == 0 && tlen + 1 < sizeof t) {
				t[tlen] = alpha[rng.below((uint32_t)nalpha)];
				t[++tlen] = '\0';
			}
		} else if (mode < 6) {
			int l = 1 + (int)rng.below(9);
			for (int j = 0; j < l; j++)
				t[j] = (char)('0' + rng.below(10));
			if (rng.below(2))
				t[l - 1] = alpha[rng.below((uint32_t)nalpha)];
			t[l] = '\0';
			tlen = (size_t)l;
		} else if (mode < 7) {
			tlen = 0;
			t[0] = '\0';
		} else {
			int l = (int)rng.below(16);
			for (int j = 0; j < l; j++)
				t[j] = alpha[rng.below((uint32_t)nalpha)];
			t[l] = '\0';
			tlen = (size_t)l;
		}
		check_gettype(t, tlen, nm, nnames);
	}
}

/* ------------------------------------------------------------------------ */
/* getdiskbyname()                                                          */
/* ------------------------------------------------------------------------ */

#define REC_MAX		2048
#define NAME_MAX	64

static void
check_getdiskbyname(const char *record, const char *name)
{
	st[S_GETDISKBYNAME].cases++;

	if (cgetset(record) != 0) {
		note(S_GETDISKBYNAME, "cgetset failed");
		return;
	}

	struct disklabel *ra = ref_getdiskbyname(name);
	struct disklabel *rb = P::getdiskbyname(name);

	if ((ra == NULL) != (rb == NULL)) {
		char msg[128];
		snprintf(msg, sizeof msg, "null mismatch (%p vs %p)",
		    (void *)ra, (void *)rb);
		note(S_GETDISKBYNAME, msg);
		cgetset(NULL);
		return;
	}
	if (ra == NULL) {
		cgetset(NULL);
		return;
	}
	if (memcmp(ra, rb, sizeof(*ra)) != 0) {
		char msg[128];
		snprintf(msg, sizeof msg, "disklabel differs (name=\"%s\")", name);
		note(S_GETDISKBYNAME, msg);
	}
	cgetset(NULL);
}

static void
hand_getdiskbyname(void)
{
	/* missing record */
	check_getdiskbyname(NULL, "nosuch");
	check_getdiskbyname("other|x:", "nosuch");

	/* minimal record: defaults only */
	check_getdiskbyname("min|Min Disk:", "min");

	/* typename stops at | */
	check_getdiskbyname("tn|alias|ignored:se#1024:", "tn");
	check_getdiskbyname("longtypename1234567890|x:", "longtypename1234567890");

	/* ty=removable and ty=simulated */
	check_getdiskbyname("r1|R1:ty=removable:", "r1");
	check_getdiskbyname("r2|R2:ty=simulated:", "r2");
	check_getdiskbyname("r3|R3:ty=other:", "r3");

	/* sf flag (boolean, type ':') */
	check_getdiskbyname("sf1|SF:sf::", "sf1");
	check_getdiskbyname("sf0|SF:", "sf0");

	/* geometry fields */
	check_getdiskbyname(
	    "geo|Geo:se#512:nt#4:ns#32:nc#100:sc#200:su#20000:"
	    "rm#5400:il#2:sk#1:cs#3:hs#10:ts#20:bs#4096:sb#8192:",
	    "geo");

	/* dt as string and numeric */
	check_getdiskbyname("dt1|DT:dt=SCSI:", "dt1");
	check_getdiskbyname("dt2|DT:dt#4:", "dt2");
	check_getdiskbyname("dt3|DT:", "dt3");

	/* single partition, defaults for frag when no bx */
	check_getdiskbyname("p1|P1:pa#1000:oa#0:fa#512:", "p1");

	/* partition with bx / frag division */
	check_getdiskbyname("p2|P2:pa#500:oa#10:fa#256:ba#2048:", "p2");

	/* fstype string vs numeric */
	check_getdiskbyname("p3|P3:pa#100:oa#0:fa#512:ba#4096:ta=swap:", "p3");
	check_getdiskbyname("p4|P4:pa#100:oa#0:fa#512:ba#4096:ta#7:", "p4");
	check_getdiskbyname("p5|P5:pa#100:oa#0:fa#512:ba#4096:ta#0:ta=4.2BSD:", "p5");

	/* gap partition: pb missing -> size 0, does not advance max */
	check_getdiskbyname("gap|Gap:pa#100:pb#200:", "gap");

	/* multiple partitions through h */
	check_getdiskbyname(
	    "mp|MP:"
	    "pa#10:oa#0:fa#512:ba#4096:ta=swap:"
	    "pb#20:ob#10:"
	    "pc#30:oc#20:fc#1024:bc#8192:tc#8:"
	    "pd#40:od#30:fd#256:bd#2048:td=MSDOS:"
	    "pe#50:oe#40:"
	    "pf#60:of#50:"
	    "pg#70:og#60:"
	    "ph#80:oh#70:",
	    "mp");

	/* drive data d0..d4 */
	check_getdiskbyname("dd|DD:d0#1:d1#2:d2#3:d3#4:d4#5:", "dd");

	/* alternate name match via | */
	check_getdiskbyname("prim|alt|third:se#256:", "alt");

	/* high-bit bytes in typename (before :) */
	check_getdiskbyname("hi\x80|Hi\x80:se#512:", "hi\x80");

	/* zero fsize: no frag path */
	check_getdiskbyname("zf|ZF:pa#100:oa#0:fa#0:", "zf");

	/* cgetstr ty with empty value edge */
	check_getdiskbyname("ety|ETY:ty=:", "ety");
}

static void
append_num(char *rec, size_t recsz, const char *cap, long val, Rng &rng)
{
	char buf[64];
	if (rng.below(8) == 0 && val != 0)
		snprintf(buf, sizeof buf, "%s#0%lo:", cap, val);
	else
		snprintf(buf, sizeof buf, "%s#%ld:", cap, val);
	if (strlen(rec) + strlen(buf) + 1 < recsz)
		strcat(rec, buf);
}

static void
append_str(char *rec, size_t recsz, const char *cap, const char *val)
{
	char buf[128];
	snprintf(buf, sizeof buf, "%s=%s:", cap, val);
	if (strlen(rec) + strlen(buf) + 1 < recsz)
		strcat(rec, buf);
}

static void
sweep_getdiskbyname(long long iters)
{
	Rng rng(0xB0132S3D1u);
	static const char *tyvals[] = {
		"removable", "simulated", "other", "scsi", ""
	};
	static const char *dtvals[] = {
		"SCSI", "ESDI", "floppy", "4.2BSD", "swap", "MSDOS", "ZFS"
	};
	char rec[REC_MAX];
	char name[NAME_MAX];

	for (long long it = 0; it < iters; it++) {
		int nlen = 1 + (int)rng.below(12);
		for (int i = 0; i < nlen; i++)
			name[i] = (char)('a' + rng.below(26));
		name[nlen] = '\0';

		snprintf(rec, sizeof rec, "%s|", name);
		{
			int tlen = (int)rng.below(20);
			size_t pos = strlen(rec);
			for (int i = 0; i < tlen && pos + 2 < sizeof rec; i++)
				rec[pos++] = (char)(32 + rng.below(95));
			rec[pos] = '\0';
		}
		strcat(rec, ":");

		if (rng.below(4) != 0)
			append_str(rec, sizeof rec, "ty",
			    tyvals[rng.below(5)]);
		if (rng.below(3) == 0)
			strcat(rec, "sf::");

		if (rng.below(2))
			append_num(rec, sizeof rec, "se", 256L << rng.below(3), rng);
		if (rng.below(2))
			append_num(rec, sizeof rec, "nt", (long)rng.below(64), rng);
		if (rng.below(2))
			append_num(rec, sizeof rec, "ns", (long)rng.below(128), rng);
		if (rng.below(2))
			append_num(rec, sizeof rec, "nc", (long)rng.below(1000), rng);

		if (rng.below(2) == 0)
			append_str(rec, sizeof rec, "dt",
			    dtvals[rng.below(7)]);
		else if (rng.below(2))
			append_num(rec, sizeof rec, "dt", (long)rng.below(17), rng);

		if (rng.below(2))
			append_num(rec, sizeof rec, "rm", (long)(3000 + rng.below(5000)), rng);
		if (rng.below(3) == 0)
			append_num(rec, sizeof rec, "il", (long)(1 + rng.below(4)), rng);

		int npart = (int)rng.below(9);
		for (int p = 0; p < npart; p++) {
			char cap[3] = { 'p', (char)('a' + p), '\0' };
			if (rng.below(6) == 0)
				continue;
			append_num(rec, sizeof rec, cap,
			    (long)(rng.below(100000) + 1), rng);
			cap[0] = 'o';
			append_num(rec, sizeof rec, cap,
			    (long)rng.below(100000), rng);
			if (rng.below(2)) {
				cap[0] = 'f';
				long fs = (long)(128 << rng.below(4));
				append_num(rec, sizeof rec, cap, fs, rng);
				if (rng.below(2)) {
					cap[0] = 'b';
					long bs = fs * (long)(1 + rng.below(16));
					append_num(rec, sizeof rec, cap, bs, rng);
				}
			}
			if (rng.below(2) == 0)
				append_str(rec, sizeof rec, "t",
				    dtvals[rng.below(7)]);
			else if (rng.below(3) == 0) {
				char tcap[3] = { 't', (char)('a' + p), '\0' };
				append_num(rec, sizeof rec, tcap,
				    (long)rng.below(32), rng);
			}
		}

		for (int d = 0; d < 5; d++) {
			if (rng.below(3) == 0) {
				char cap[3];
				snprintf(cap, sizeof cap, "d%d", d);
				append_num(rec, sizeof rec, cap,
				    (long)rng.below(0x10000), rng);
			}
		}

		check_getdiskbyname(rec, name);

		/* also query by alias sometimes */
		if (rng.below(5) == 0) {
			char alias[NAME_MAX];
			snprintf(alias, sizeof alias, "z%lld", it);
			char rec2[REC_MAX];
			snprintf(rec2, sizeof rec2, "%s|%s:", name, alias);
			strcat(rec2, strchr(rec, ':') + 1);
			check_getdiskbyname(rec2, alias);
		}

		/* miss case */
		if (rng.below(20) == 0)
			check_getdiskbyname(rec, "absent");
	}
}

/* ------------------------------------------------------------------------ */
/* main                                                                     */
/* ------------------------------------------------------------------------ */

int
main(void)
{
	hand_gettype();
	hand_getdiskbyname();
	sweep_gettype(200000);
	sweep_getdiskbyname(200000);

	long long total_cases = 0, total_fails = 0;
	printf("function            cases     fails\n");
	printf("-----------------------------------\n");
	for (int i = 0; i < S_NFUNC; i++) {
		printf("%-18s %7lld %9lld\n",
		    st[i].name, st[i].cases, st[i].fails);
		total_cases += st[i].cases;
		total_fails += st[i].fails;
	}
	printf("-----------------------------------\n");
	printf("%-18s %7lld %9lld\n", "TOTAL", total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
