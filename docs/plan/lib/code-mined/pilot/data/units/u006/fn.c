static int calculate_chan_offset(int width, int freq, int cf1, int cf2)
{
	int freq1 = 0;

	switch (convert2width(width)) {
	case CHAN_WIDTH_20_NOHT:
	case CHAN_WIDTH_20:
		return 0;
	case CHAN_WIDTH_40:
		freq1 = cf1 - 10;
		break;
	case CHAN_WIDTH_80:
		freq1 = cf1 - 30;
		break;
	case CHAN_WIDTH_160:
		freq1 = cf1 - 70;
		break;
	case CHAN_WIDTH_80P80:
		freq1 = cf1 - 30;
		break;
	case CHAN_WIDTH_320:
		freq1 = cf1 - 150;
		break;
	case CHAN_WIDTH_UNKNOWN:
	case CHAN_WIDTH_2160:
	case CHAN_WIDTH_4320:
	case CHAN_WIDTH_6480:
	case CHAN_WIDTH_8640:
		/* FIXME: implement this */
		return 0;
	}

	return (abs(freq - freq1) / 20) % 2 == 0 ? 1 : -1;
}
