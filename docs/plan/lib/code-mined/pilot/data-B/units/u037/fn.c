static int  __init s3fb_setup(char *options)
{
	char *opt;

	if (!options || !*options)
		return 0;

	while ((opt = strsep(&options, ",")) != NULL) {

		if (!*opt)
			continue;
		else if (!strncmp(opt, "mtrr:", 5))
			mtrr = simple_strtoul(opt + 5, NULL, 0);
		else if (!strncmp(opt, "fasttext:", 9))
			fasttext = simple_strtoul(opt + 9, NULL, 0);
		else
			mode_option = opt;
	}

	return 0;
}
