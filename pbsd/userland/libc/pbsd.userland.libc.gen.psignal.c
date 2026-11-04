/* Reference logic from hbsd/src/lib/libc/gen/psignal.c (dual-world). */
#include <stddef.h>

static const char *sys_siglist[] = {
	"Hangup", "Interrupt", "Quit", "Illegal instruction", "Trace/BPT trap",
};

const char *
psignal_name(int sig)
{
	if (sig < 0 || sig >= 5)
		return "Unknown signal";
	return sys_siglist[sig];
}

size_t
psignal_msg_len(const char *prefix, int sig)
{
	size_t n = 0;
	const char *name = psignal_name(sig);

	if (prefix != NULL) {
		while (*prefix != '\0')
			n++, prefix++;
		n += 2;
	}
	while (*name != '\0')
		n++, name++;
	return n + 1;
}
