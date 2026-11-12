static bool madvise_args_are_sane(struct xe_device *xe, const struct drm_xe_madvise *args)
{
	if (XE_IOCTL_DBG(xe, !args))
		return false;

	if (XE_IOCTL_DBG(xe, !IS_ALIGNED(args->start, SZ_4K)))
		return false;

	if (XE_IOCTL_DBG(xe, !IS_ALIGNED(args->range, SZ_4K)))
		return false;

	if (XE_IOCTL_DBG(xe, args->range < SZ_4K))
		return false;

	switch (args->type) {
	case DRM_XE_MEM_RANGE_ATTR_PREFERRED_LOC:
	{
		s32 fd = (s32)args->preferred_mem_loc.devmem_fd;

		if (XE_IOCTL_DBG(xe, fd < DRM_XE_PREFERRED_LOC_DEFAULT_SYSTEM))
			return false;

		if (XE_IOCTL_DBG(xe, args->preferred_mem_loc.migration_policy >
				     DRM_XE_MIGRATE_ONLY_SYSTEM_PAGES))
			return false;

		if (XE_IOCTL_DBG(xe, args->preferred_mem_loc.pad))
			return false;

		if (XE_IOCTL_DBG(xe, args->preferred_mem_loc.reserved))
			return false;
		break;
	}
	case DRM_XE_MEM_RANGE_ATTR_ATOMIC:
		if (XE_IOCTL_DBG(xe, args->atomic.val > DRM_XE_ATOMIC_CPU))
			return false;

		if (XE_IOCTL_DBG(xe, args->atomic.pad))
			return false;

		if (XE_IOCTL_DBG(xe, args->atomic.reserved))
			return false;

		break;
	case DRM_XE_MEM_RANGE_ATTR_PAT:
	{
		u16 pat_index, coh_mode;

		if (XE_IOCTL_DBG(xe, args->pat_index.val >= xe->pat.n_entries))
			return false;

		pat_index = array_index_nospec(args->pat_index.val, xe->pat.n_entries);
		coh_mode = xe_pat_index_get_coh_mode(xe, pat_index);
		if (XE_IOCTL_DBG(xe, !coh_mode))
			return false;

		if (XE_WARN_ON(coh_mode > XE_COH_AT_LEAST_1WAY))
			return false;

		if (XE_IOCTL_DBG(xe, args->pat_index.pad))
			return false;

		if (XE_IOCTL_DBG(xe, args->pat_index.reserved))
			return false;
		break;
	}
	default:
		if (XE_IOCTL_DBG(xe, 1))
			return false;
	}

	if (XE_IOCTL_DBG(xe, args->reserved[0] || args->reserved[1]))
		return false;

	return true;
}
