static inline int get_verbose(char **argv, int argc)
{
	int c;
	int verbose = 0;

	while ((c = getopt(argc, argv, "v")) != -1) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		default:
			break;
		}
	}
	optind = 1;

	return verbose;
}
