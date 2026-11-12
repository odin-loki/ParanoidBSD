int ext4_ext_truncate(handle_t *handle, struct inode *inode)
{
	struct super_block *sb = inode->i_sb;
	ext4_lblk_t last_block;
	int err = 0;

	/*
	 * TODO: optimization is possible here.
	 * Probably we need not scan at all,
	 * because page truncation is enough.
	 */

	/* we have to know where to truncate from in crash case */
	EXT4_I(inode)->i_disksize = inode->i_size;
	err = ext4_mark_inode_dirty(handle, inode);
	if (err)
		return err;

	last_block = (inode->i_size + sb->s_blocksize - 1)
			>> EXT4_BLOCK_SIZE_BITS(sb);
	ext4_es_remove_extent(inode, last_block, EXT_MAX_BLOCKS - last_block);

retry_remove_space:
	err = ext4_ext_remove_space(inode, last_block, EXT_MAX_BLOCKS - 1);
	if (err == -ENOMEM) {
		memalloc_retry_wait(GFP_ATOMIC);
		goto retry_remove_space;
	}
	return err;
}
