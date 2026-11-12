int __ceph_caps_wanted(struct ceph_inode_info *ci)
{
	int w = __ceph_caps_file_wanted(ci) | __ceph_caps_used(ci);
	if (S_ISDIR(ci->netfs.inode.i_mode)) {
		/* we want EXCL if holding caps of dir ops */
		if (w & CEPH_CAP_ANY_DIR_OPS)
			w |= CEPH_CAP_FILE_EXCL;
	} else {
		/* we want EXCL if dirty data */
		if (w & CEPH_CAP_FILE_BUFFER)
			w |= CEPH_CAP_FILE_EXCL;
	}
	return w;
}
