static bool
gsi_trans_tre_reserve(struct gsi_trans_info *trans_info, u32 tre_count)
{
	int avail = atomic_read(&trans_info->tre_avail);
	int new;

	do {
		new = avail - (int)tre_count;
		if (unlikely(new < 0))
			return false;
	} while (!atomic_try_cmpxchg(&trans_info->tre_avail, &avail, new));

	return true;
}
