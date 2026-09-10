/*
 * A model of usr.bin/diff/diffdir.c's path-building block, driven with
 * the (dp, dp2) pairs diffdir()'s own loop produces.  It is a model,
 * not the program: what it demonstrates is which pointer each version
 * dereferences and which name each writes into path2.
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>

#define PATH_MAX 1024
struct dirent { char d_name[256]; };
static int ignore_file_case = 1;          /* diff -i */
static sigjmp_buf jb;
static void segv(int s) { (void)s; siglongjmp(jb, 1); }

static void old_block(struct dirent *dp, struct dirent *dp2,
    char *path2, size_t plen2)
{
	if (ignore_file_case && strcasecmp(dp2->d_name, dp2->d_name) == 0)
		strncpy(path2 + plen2, dp2->d_name, PATH_MAX - plen2);
	else
		strncpy(path2 + plen2, dp->d_name, PATH_MAX - plen2);
}

static void new_block(struct dirent *dp, struct dirent *dp2,
    char *path2, size_t plen2)
{
	if (ignore_file_case && dp2 != NULL &&
	    strcasecmp(dp->d_name, dp2->d_name) == 0)
		strncpy(path2 + plen2, dp2->d_name, PATH_MAX - plen2);
	else
		strncpy(path2 + plen2, dp->d_name, PATH_MAX - plen2);
}

static const char *run(void (*f)(struct dirent *, struct dirent *, char *,
    size_t), struct dirent *dp, struct dirent *dp2, char *out)
{
	static char buf[PATH_MAX];
	memset(buf, 0, sizeof buf);
	if (sigsetjmp(jb, 1)) return "SIGSEGV";
	f(dp, dp2, buf, 0);
	strcpy(out, buf);
	return out;
}

int main(void)
{
	struct dirent a = {"README"}, A = {"readme"}, b = {"zulu"};
	struct {
		const char *what;
		struct dirent *dp, *dp2;
		const char *want;
	} cases[] = {
	  /* pos == 0, -i matched two spellings: path2 must use dir2's */
	  {"both present, names differ by case", &a, &A, "readme"},
	  /* pos < 0 with both present (-N): dp is absent from dir2, so
	     path2 must name dp, not the unrelated dp2 */
	  {"both present, different names (-N)", &a, &b, "README"},
	  /* pos < 0 because dent2 == NULL (-N): nothing to dereference */
	  {"only in dir1 (-N)",                  &a, NULL, "README"},
	  /* pos > 0 because dent1 == NULL (-N/-P), args swapped */
	  {"only in dir2 (-N or -P)",            &b, NULL, "zulu"},
	};
	char o[PATH_MAX], n[PATH_MAX];
	int bad_old = 0;

	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = segv;
	sa.sa_flags = SA_NODEFER;
	sigaction(SIGSEGV, &sa, NULL);
	printf("%-38s  %-10s  %-10s  %s\n", "case", "old", "new", "wanted");
	for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
		const char *ro = run(old_block, cases[i].dp, cases[i].dp2, o);
		const char *rn = run(new_block, cases[i].dp, cases[i].dp2, n);
		printf("%-38s  %-10s  %-10s  %s%s\n", cases[i].what, ro, rn,
		    cases[i].want,
		    strcmp(rn, cases[i].want) ? "   <-- NEW IS WRONG" : "");
		if (strcmp(ro, cases[i].want))
			bad_old++;
	}
	printf("\nold block wrong or crashed in %d of 4 cases\n", bad_old);
	return 0;
}
