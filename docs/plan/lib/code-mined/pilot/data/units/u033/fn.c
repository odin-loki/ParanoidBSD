static enum nr_chan_width hostapd_get_nr_chan_width(struct hostapd_data *hapd,
						    int ht, int vht, int he)
{
	enum oper_chan_width oper_chwidth;

	oper_chwidth = hostapd_get_oper_chwidth(hapd->iconf);

	if (!ht && !vht && !he)
		return NR_CHAN_WIDTH_20;
	if (!hapd->iconf->secondary_channel)
		return NR_CHAN_WIDTH_20;
	if ((!vht && !he) || oper_chwidth == CONF_OPER_CHWIDTH_USE_HT)
		return NR_CHAN_WIDTH_40;
	if (oper_chwidth == CONF_OPER_CHWIDTH_80MHZ)
		return NR_CHAN_WIDTH_80;
	if (oper_chwidth == CONF_OPER_CHWIDTH_160MHZ)
		return NR_CHAN_WIDTH_160;
	if (oper_chwidth == CONF_OPER_CHWIDTH_80P80MHZ)
		return NR_CHAN_WIDTH_80P80;
	return NR_CHAN_WIDTH_20;
}
