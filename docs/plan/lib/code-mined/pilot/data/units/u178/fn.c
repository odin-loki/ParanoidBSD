unsigned int f2fs_shrink_read_extent_tree(struct f2fs_sb_info *sbi, int nr_shrink)
{
	if (!test_opt(sbi, READ_EXTENT_CACHE))
		return 0;

	return __shrink_extent_tree(sbi, nr_shrink, EX_READ);
}
