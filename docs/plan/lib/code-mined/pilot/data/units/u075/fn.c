static int b43_gphy_op_switch_channel(struct b43_wldev *dev,
				      unsigned int new_channel)
{
	if ((new_channel < 1) || (new_channel > 14))
		return -EINVAL;
	b43_gphy_channel_switch(dev, new_channel, 0);

	return 0;
}
