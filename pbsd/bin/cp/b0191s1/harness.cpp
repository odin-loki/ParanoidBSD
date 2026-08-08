/*
 * harness.cpp -- differential test for PBSD batch b0191s1 (cp/utils.c).
 */

#define _GNU_SOURCE

#ifndef SIGINFO
#define SIGINFO SIGUSR1
#endif

#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fts.h>
#include <getopt.h>
#include <limits.h>
#include <sysexits.h>
#include <map>
#include <memory>
#include <signal.h>
#include <string>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#if defined(__linux__)
#include <bsd/string.h>
#endif

import pbsd.bin.cp.b0191s1;

namespace P = pbsd::bin_cp::b0191s1;

typedef void *acl_t;
typedef unsigned int acl_type_t;

extern "C" {
enum ref_op { RFILE_TO_FILE, RFILE_TO_DIR, RDIR_TO_DNE };

typedef struct {
	int dir;
	char base[PATH_MAX + 1];
	char *end;
	char path[PATH_MAX];
} PATH_T;

extern PATH_T ref_to;
extern bool ref_Nflag, ref_fflag, ref_iflag, ref_lflag, ref_nflag, ref_pflag,
    ref_sfla