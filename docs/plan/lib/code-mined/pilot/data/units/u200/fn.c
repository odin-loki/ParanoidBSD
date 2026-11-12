static void create_adc_command(unsigned int chan,
			       u8 *muxsg0, u8 *muxsg1)
{
	if (chan < 8)
		(*muxsg0) = (*muxsg0) | (1 << chan);
	else if (chan < 16)
		(*muxsg1) = (*muxsg1) | (1 << (chan - 8));
}
