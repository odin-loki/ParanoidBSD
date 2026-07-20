/* Reference logic from hbsd/src/usr.bin/asa/asa.c (dual-world). */
#include <stddef.h>

int
asa_classify(char ctrl, int had_eol)
{
	switch (ctrl) {
	case '0':
		return 1; /* newline */
	case '1':
		return 2; /* form feed */
	case '+':
		return had_eol ? 3 : 0; /* carriage return or space */
	default:
		return 0;
	}
}

int
asa_needs_prior_newline(char ctrl)
{
	return ctrl != '+';
}

size_t
asa_payload_len(char ctrl, size_t len)
{
	if (len <= 1)
		return 0;
	if (ctrl == '\0')
		return len;
	if (ctrl != ' ' && ctrl != '0' && ctrl != '1' && ctrl != '+')
		return len;
	return len - 1;
}
