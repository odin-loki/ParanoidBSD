/*
 * oracle.c -- reference specification for PBSD batch b0176 (nproc.c).
 *
 * nproc.c is renamed with a ref_ prefix; bodies are otherwise UNMODIFIED.
 * Supporting defines, globals, and shims are added only where required so the
 * unmodified bodies compile and link.
 */

#define _GNU_SOURCE

#include <limits.h>
#include <sched.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <bsd/stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sysexits.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef __unreachable
#define __unreachable() __builtin_unreachable()
#endif

#ifndef cpuset_t
typedef cpu_set_t cpuset_t;
#endif

#ifndef CPU_LEVEL_WHICH
#define CPU_LEVEL_WHICH 0
#endif

#ifndef CPU_WHICH_TID
#define CPU_WHICH_TID 0
#endif

static inline int
cpuset_getaffinity(int level, int which, id_t id, size_t setsize, cpuset_t *mask)
{
	(void)level;
	(void)which;
	(void)id;
	return (sched_getaffinity(0, setsize, mask));
}

jmp_buf oracle_err_jmp;
static int oracle_err_armed;
int oracle_err_called;
int oracle_err_status;

void
oracle_err_arm(void)
{
	oracle_err_armed = 1;
}

void
oracle_err_disarm(void)
{
	oracle_err_armed = 0;
}

static void
err(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	oracle_err_called = 1;
	oracle_err_status = eval;
	if (oracle_err_armed)
		longjmp(oracle_err_jmp, 1);
	exit(eval);
}

static void
errx(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	oracle_err_called = 1;
	oracle_err_status = eval;
	if (oracle_err_armed)
		longjmp(oracle_err_jmp, 1);
	exit(eval);
}

#define help		ref_help
#define usage		ref_usage
#define version		ref_version

#define OPT_ALL		(CHAR_MAX + 1)
#define OPT_IGNORE	(CHAR_MAX + 2)
#define OPT_VERSION	(CHAR_MAX + 3)
#define OPT_HELP	(CHAR_MAX + 4)

static struct option long_opts[] = {
	{ "all", no_argument, NULL, OPT_ALL },
	{ "ignore", required_argument, NULL, OPT_IGNORE },
	{ "version", no_argument, NULL, OPT_VERSION },
	{ "help", no_argument, NULL, OPT_HELP },
	{ NULL, 0, NULL, 0 }
};

/*-
 * Copyright (c) 2023 Mateusz Guzik
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/*
 * This program is intended to be compatible with nproc as found in GNU
 * coreutils.
 *
 * In order to maintain that, do not add any features here if they are not
 * present in said program.  If you are looking for anything more advanced you
 * probably should patch cpuset(1) instead.
 */

void
ref_help(void)
{
	fprintf(stderr,
    "usage: nproc [--all] [--ignore=count]\n");
	fprintf(stderr,
    "       nproc --help\n");
	fprintf(stderr,
    "       nproc --version\n");
}

void
ref_usage(void)
{
	help();
	exit(EX_USAGE);
}

/*
 * GNU variant ships with the --version switch.
 *
 * While we don't have anything to put there, print something which is
 * whitespace-compatible with the original. Version number was taken
 * from coreutils this code is in sync with.
 */
void
ref_version(void)
{
	printf("nproc (neither_GNU nor_coreutils) 8.32\n");
	exit(EXIT_SUCCESS);
}

int
ref_main(int argc, char *argv[])
{
	const char *errstr;
	cpuset_t mask;
	int ch, cpus, ignore;
	bool all_flag;

	ignore = 0;
	all_flag = false;

	while ((ch = getopt_long(argc, argv, "", long_opts, NULL)) != -1) {
		switch (ch) {
		case OPT_ALL:
			all_flag = true;
			break;
		case OPT_IGNORE:
			ignore = strtonum(optarg, 0, INT_MAX, &errstr);
			if (errstr)
				errx(1, "bad ignore count: %s", errstr);
			break;
		case OPT_VERSION:
			version();
			__unreachable();
		case OPT_HELP:
			help();
			exit(EXIT_SUCCESS);
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 0)
		usage();

	if (all_flag) {
		cpus = sysconf(_SC_NPROCESSORS_CONF);
		if (cpus == -1)
			err(1, "sysconf");
	} else {
		CPU_ZERO(&mask);
		if (cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1,
		    sizeof(mask), &mask) != 0)
			err(1, "cpuset_getaffinity");
		cpus = CPU_COUNT(&mask);
	}

	if (ignore >= cpus)
		cpus = 1;
	else
		cpus -= ignore;

	printf("%u\n", cpus);

	exit(EXIT_SUCCESS);
}

#undef help
#undef usage
#undef version
