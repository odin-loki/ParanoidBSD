static int
minstrel_ht_move_sample_rates(struct minstrel_ht_sta *mi, int type,
			      u32 fast_rate_dur, u32 slow_rate_dur)
{
	u16 *rates = mi->sample[type].sample_rates;
	int i, j;

	for (i = 0, j = 0; i < MINSTREL_SAMPLE_RATES; i++) {
		u32 duration;
		bool valid = false;
		u16 cur;

		cur = rates[i];
		if (!cur)
			continue;

		duration = minstrel_get_duration(cur);
		switch (type) {
		case MINSTREL_SAMPLE_TYPE_SLOW:
			valid = duration > fast_rate_dur &&
				duration < slow_rate_dur;
			break;
		case MINSTREL_SAMPLE_TYPE_INC:
		case MINSTREL_SAMPLE_TYPE_JUMP:
			valid = duration < fast_rate_dur;
			break;
		default:
			valid = false;
			break;
		}

		if (!valid) {
			rates[i] = 0;
			continue;
		}

		if (i == j)
			continue;

		rates[j++] = cur;
		rates[i] = 0;
	}

	return j;
}
