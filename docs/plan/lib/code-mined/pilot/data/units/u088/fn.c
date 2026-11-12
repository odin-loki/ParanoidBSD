static int check_version(struct dlm_plock_info *info)
{
	if ((DLM_PLOCK_VERSION_MAJOR != info->version[0]) ||
	    (DLM_PLOCK_VERSION_MINOR < info->version[1])) {
		log_print("plock device version mismatch: "
			  "kernel (%u.%u.%u), user (%u.%u.%u)",
			  DLM_PLOCK_VERSION_MAJOR,
			  DLM_PLOCK_VERSION_MINOR,
			  DLM_PLOCK_VERSION_PATCH,
			  info->version[0],
			  info->version[1],
			  info->version[2]);
		return -EINVAL;
	}
	return 0;
}
