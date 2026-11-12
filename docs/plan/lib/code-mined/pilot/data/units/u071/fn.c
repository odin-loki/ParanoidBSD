int perf_allow_kernel(void)
{
	if (sysctl_perf_event_paranoid > 1 && !perfmon_capable())
		return -EACCES;

	return security_perf_event_open(PERF_SECURITY_KERNEL);
}
