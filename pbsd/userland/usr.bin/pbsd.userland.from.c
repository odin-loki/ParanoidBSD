/* Reference logic from hbsd/src/usr.bin/from/from.c (dual-world). */
#include <stddef.h>

static int
is_mailbox_char(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
	    (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-' ||
	    c == '@';
}

int
from_valid_sender(const char *sender)
{
	if (sender == NULL || sender[0] == '\0')
		return 0;
	for (const char *p = sender; *p != '\0'; ++p) {
		if (!is_mailbox_char(*p))
			return 0;
	}
	return 1;
}

int
from_parse_header(const char *line, char *sender, size_t sender_len)
{
	if (line == NULL || sender == NULL || sender_len == 0)
		return -1;
	while (*line == ' ' || *line == '\t')
		++line;
	if (line[0] != 'F' || line[1] != 'r' || line[2] != 'o' ||
	    line[3] != 'm' || line[4] != ' ')
		return -1;
	line += 5;
	size_t i = 0;
	while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') {
		if (i + 1 >= sender_len)
			return -1;
		sender[i++] = *line++;
	}
	sender[i] = '\0';
	return from_valid_sender(sender) ? 0 : -1;
}
