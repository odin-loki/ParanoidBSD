int
scan_dump(FILE *fp, off_t size)
{
	struct pcap_file_header hdr;
#ifdef __FreeBSD__
	struct pcap_sf_pkthdr ph;
#else
	struct pcap_pkthdr ph;
#endif
	off_t pos;

	/*
	 * Must read the file, compare the header against our new
	 * options (in particular, snaplen) and adjust our options so
	 * that we generate a correct file. Furthermore, check the file
	 * for consistency so that we can append safely.
	 *
	 * XXX this may take a long time for large logs.
	 */
	(void) fseek(fp, 0L, SEEK_SET);

	if (fread((char *)&hdr, sizeof(hdr), 1, fp) != 1) {
		logmsg(LOG_ERR, "Short file header");
		return (1);
	}

	if (hdr.magic != TCPDUMP_MAGIC ||
	    hdr.version_major != PCAP_VERSION_MAJOR ||
	    hdr.version_minor != PCAP_VERSION_MINOR ||
	    hdr.linktype != hpcap->linktype ||
	    hdr.snaplen > PFLOGD_MAXSNAPLEN) {
		return (1);
	}

	pos = sizeof(hdr);

	while (!feof(fp)) {
		off_t len = fread((char *)&ph, 1, sizeof(ph), fp);
		if (len == 0)
			break;

		if (len != sizeof(ph))
			goto error;
		if (ph.caplen > hdr.snaplen || ph.caplen > PFLOGD_MAXSNAPLEN)
			goto error;
		pos += sizeof(ph) + ph.caplen;
		if (pos > size)
			goto error;
		fseek(fp, ph.caplen, SEEK_CUR);
	}

	if (pos != size)
		goto error;

	if (hdr.snaplen != cur_snaplen) {
		logmsg(LOG_WARNING,
		       "Existing file has different snaplen %u, using it",
		       hdr.snaplen);
		if (set_snaplen(hdr.snaplen)) {
			logmsg(LOG_WARNING,
			       "Failed, using old settings, offset %llu",
			       (unsigned long long) size);
		}
	}

	return (0);

 error:
	logmsg(LOG_ERR, "Corrupted log file.");
	return (1);
}
