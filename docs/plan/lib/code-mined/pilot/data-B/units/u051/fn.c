int
main(int argc, char **argv)
{
	int i, ch;
	struct command *cmd = NULL;

	argc = xo_parse_args(argc, argv);
	if (argc < 0)
		exit(EXIT_FAILURE);

	uflag = "/dev/ses[0-9]*";
	while ((ch = getopt_long(argc, argv, "u:", NULL, NULL)) != -1) {
		switch (ch) {
		case 'u':
			uflag = optarg;
			break;
		case '?':
		default:
			usage(NULL);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1) {
		xo_warnx("Missing command");
		usage(NULL);
	}

	for (i = 0; i < nbcmds; i++) {
		if (strcmp(argv[0], cmds[i].name) == 0) {
			cmd = &cmds[i];
			break;
		}
	}

	if (cmd == NULL) {
		xo_warnx("unknown command %s", argv[0]);
		usage(NULL);
	}

	return (cmd->exec(argc, argv));
}
