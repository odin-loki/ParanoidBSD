int ovl_get_write_access(struct dentry *dentry)
{
	struct ovl_fs *ofs = OVL_FS(dentry->d_sb);
	return mnt_get_write_access(ovl_upper_mnt(ofs));
}
