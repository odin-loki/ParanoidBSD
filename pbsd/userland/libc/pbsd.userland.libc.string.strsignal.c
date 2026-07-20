/* Reference logic from hbsd/src/lib/libc/string/strsignal.c (dual-world). */
#include <stddef.h>

static const char *const sig_names[] = {
	"HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE",
	"KILL", "BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", "URG",
};

const char *
strsignal_name(int sig)
{
	if (sig < 1 || sig > 15)
		return NULL;
	return sig_names[sig - 1];
}
