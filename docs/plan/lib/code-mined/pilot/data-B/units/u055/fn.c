int
setup(int nb)
{
	int64_t in;
	int64_t bl;
	int64_t reserve_in;
	int64_t reserve_bl;

	umask(0);

	if (nb == 0) {
		getdf(&bl, &in);
		size = in / op->incarnations;

		if (size > 1000)
			size = 1000;	/* arbitrary limit number of files pr. dir */

		/* Resource requirements: */
		while (size > 0) {
			reserve_in =  2 * size * op->incarnations + 2 * op->incarnations;
			reserve_bl = 30 * size * op->incarnations;
			if (reserve_bl <= bl && reserve_in <= in)
				break;
			size = size / 2;
		}
		if (size == 0)
			reserve_bl = reserve_in = 0;

		if (op->verbose > 1)
			printf("rename(size=%lu, incarnations=%d). Free(%jdk, %jd), reserve(%jdk, %jd)\n",
				size, op->incarnations, bl/1024, in, reserve_bl/1024, reserve_in);
		reservedf(reserve_bl, reserve_in);
		putval(size);
	} else {
		size = getval();
	}
	if (size == 0)
		exit(0);

	return (0);
}
