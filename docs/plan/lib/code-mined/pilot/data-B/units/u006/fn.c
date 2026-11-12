static int
fetch_sysinfo(void)
{
	u_char *data;
	u_char *ptr;
	size_t datalen;
	char *name;
	char *val;

	if (ntpd_dialog(NTPC_OP_READVAR, 0,
	    "leap,stratum,precision,rootdelay,rootdispersion,refid,reftime,"
	    "poll,peer,clock,system,processor,jitter,stability",
	    &data, &datalen))
		return (-1);

	/* clear info */
	sysb_leap = 0;
	sysb_stratum = 0;
	sysb_precision = 0;
	free(sys_rootdelay);
	sys_rootdelay = NULL;
	free(sys_rootdispersion);
	sys_rootdispersion = NULL;
	free(sys_refid);
	sys_refid = NULL;
	sysb_reftime = 0;
	sysb_poll = 0;
	sysb_peer = 0;
	sysb_clock = 0;
	free(sys_system);
	sys_system = NULL;
	free(sys_processor);
	sys_processor = NULL;
	sysb_jitter = 0;
	sysb_stability = 0;

	ptr = data;
	while (ntpd_parse(&ptr, &datalen, &name, &val)) {
		if (ntp_debug & DBG_DUMP_VARS)
			syslog(LOG_DEBUG, "%s: '%s'='%s'", __func__, name, val);
		if (strcmp(name, "leap") == 0 ||
		    strcmp(name, "sys.leap") == 0) {
			sysb_leap = val_parse_int32(val, &sys_leap,
			    0, 3, 2);

		} else if (strcmp(name, "stratum") == 0 ||
		    strcmp(name, "sys.stratum") == 0) {
			sysb_stratum = val_parse_int32(val, &sys_stratum,
			    0, 255, 0);

		} else if (strcmp(name, "precision") == 0 ||
		    strcmp(name, "sys.precision") == 0) {
			sysb_precision = val_parse_int32(val, &sys_precision,
			    INT32_MIN, INT32_MAX, 0);

		} else if (strcmp(name, "rootdelay") == 0 ||
		    strcmp(name, "sys.rootdelay") == 0) {
			sys_rootdelay = strdup(val);

		} else if (strcmp(name, "rootdispersion") == 0 ||
		    strcmp(name, "sys.rootdispersion") == 0) {
			sys_rootdispersion = strdup(val);

		} else if (strcmp(name, "refid") == 0 ||
		    strcmp(name, "sys.refid") == 0) {
			sys_refid = strdup(val);

		} else if (strcmp(name, "reftime") == 0 ||
		    strcmp(name, "sys.reftime") == 0) {
			sysb_reftime = val_parse_ts(val, sys_reftime);

		} else if (strcmp(name, "poll") == 0 ||
		    strcmp(name, "sys.poll") == 0) {
			sysb_poll = val_parse_int32(val, &sys_poll,
			    INT32_MIN, INT32_MAX, 0);

		} else if (strcmp(name, "peer") == 0 ||
		    strcmp(name, "sys.peer") == 0) {
			sysb_peer = val_parse_uint32(val, &sys_peer,
			    0, UINT32_MAX, 0);

		} else if (strcmp(name, "clock") == 0 ||
		    strcmp(name, "sys.clock") == 0) {
			sysb_clock = val_parse_ts(val, sys_clock);

		} else if (strcmp(name, "system") == 0 ||
		    strcmp(name, "sys.system") == 0) {
			sys_system = strdup(val);

		} else if (strcmp(name, "processor") == 0 ||
		    strcmp(name, "sys.processor") == 0) {
			sys_processor = strdup(val);

		} else if (strcmp(name, "jitter") == 0 ||
		    strcmp(name, "sys.jitter") == 0) {
			sysb_jitter = val_parse_double(val, &sys_jitter);

		} else if (strcmp(name, "stability") == 0 ||
		    strcmp(name, "sys.stability") == 0) {
			sysb_stability = val_parse_double(val, &sys_stability);
		}
	}

	free(data);
	return (0);
}
