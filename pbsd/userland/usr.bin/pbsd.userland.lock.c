/* Reference logic from hbsd/src/usr.bin/lock/lock.c (dual-world). */

int
lock_parse_timeout(const char *arg)
{
	int val = 0;

	if (arg == NULL || arg[0] != '-')
		return -1;
	for (const char *p = arg + 1; *p != '\0'; ++p) {
		if (*p < '0' || *p > '9')
			return -1;
		val = val * 10 + (*p - '0');
	}
	return val > 0 ? val : -1;
}

int
lock_verify_pw(const char *typed, const char *expected)
{
	int i = 0;

	if (typed == NULL || expected == NULL)
		return 0;
	while (typed[i] != '\0' && expected[i] != '\0') {
		if (typed[i] != expected[i])
			return 0;
		i++;
	}
	return typed[i] == expected[i];
}
