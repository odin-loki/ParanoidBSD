static u_int
dbprog_discid(void)
{
	struct	ioc_toc_header h;
	int	rc;
	int	i, ntr,
		t = 0,
		n = 0;

	rc = ioctl (fd, CDIOREADTOCHEADER, &h);
	if (rc < 0)
		return 0;
	ntr = h.ending_track - h.starting_track + 1;
	i = msf;
	msf = 1;
	rc = read_toc_entrys ((ntr + 1) * sizeof (struct cd_toc_entry));
	msf = i;
	if (rc < 0)
		return 0;
	/* For backward compatibility this algorithm must not change */
	for (i = 0; i < ntr; i++) {
#define TC_MM(a) toc_buffer[a].addr.msf.minute
#define TC_SS(a) toc_buffer[a].addr.msf.second
		n += dbprog_sum((TC_MM(i) * 60) + TC_SS(i));

		t += ((TC_MM(i+1) * 60) + TC_SS(i+1)) -
		    ((TC_MM(i) * 60) + TC_SS(i));
	}

	return((n % 0xff) << 24 | t << 8 | ntr);
}
