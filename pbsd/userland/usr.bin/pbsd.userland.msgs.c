/* Reference logic from hbsd/src/usr.bin/msgs/msgs.c (dual-world). */

int
msgs_parse_mode(char ch)
{
	switch (ch) {
	case 's':
		return 1;
	case 'c':
		return 2;
	case 'f':
	case 'h':
	case 'l':
	case 'o':
	case 'p':
	case 'q':
		return 0;
	default:
		return -1;
	}
}

int
msgs_is_prompt(char ch)
{
	switch (ch) {
	case 'y':
	case 'n':
	case 'q':
	case 'p':
	case 'P':
	case '-':
	case 's':
	case 'm':
	case 'x':
		return 1;
	default:
		return ch >= '0' && ch <= '9';
	}
}
