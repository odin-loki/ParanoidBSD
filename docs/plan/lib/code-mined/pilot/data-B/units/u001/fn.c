static int
parse_signal(const char *str)
{
	int sig, i;
	const char *errstr;

	sig = strtonum(str, 1, sys_nsig - 1, &errstr);

	if (errstr == NULL)
		return (sig);
	if (strncasecmp(str, "SIG", 3) == 0)
		str += 3;

	for (i = 1; i < sys_nsig; i++) {
		if (strcasecmp(str, sys_signame[i]) == 0)
			return (i);
	}

	return (-1);
}
