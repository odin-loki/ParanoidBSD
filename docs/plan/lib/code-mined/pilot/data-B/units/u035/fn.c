int
rtsock_input(int s)
{
	ssize_t n;
	char msg[2048];
	char *lim, *next;
	struct rt_msghdr *rtm;
	int idx;
	ssize_t len;
	int ret = 0;
	const ssize_t lenlim =
	    offsetof(struct rt_msghdr, rtm_msglen) + sizeof(rtm->rtm_msglen);

	n = read(s, msg, sizeof(msg));

	lim = msg + n;
	for (next = msg; next < lim; next += len) {
		rtm = (struct rt_msghdr *)(void *)next;
		if (lim - next < lenlim)
			break;
		len = rtm->rtm_msglen;
		if (len < lenlim)
			break;

		if (dflag > 1) {
			warnmsg(LOG_INFO, __func__,
			    "rtmsg type %d, len=%lu", rtm->rtm_type,
			    (u_long)len);
		}

		for (idx = 0; rtsock_dispatch[idx].func; idx++) {
			if (rtm->rtm_type != rtsock_dispatch[idx].type)
				continue;
			if (rtm->rtm_msglen < rtsock_dispatch[idx].minlen) {
				warnmsg(LOG_INFO, __func__,
				    "rtmsg type %d too short!", rtm->rtm_type);
				continue;
			}

			ret = (*rtsock_dispatch[idx].func)(s, rtm, lim);
			break;
		}
	}

	return (ret);
}
