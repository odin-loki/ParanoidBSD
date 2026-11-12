static void create_adc_command(unsigned int chan,
			       u8 *muxsg0, u8 *muxsg1)
{
	if (chan < 8) {
		/*@ assert 0 <= chan < 8; */
		/*@ assert (1 << chan) <= 128; */
		(*muxsg0) = (*muxsg0) | (1 << chan);
		/*@ assert \at(*muxsg0, Pre) <= *muxsg0; */
	}
	else if (chan < 16) {
		/*@ assert 8 <= chan < 16; */
		/*@ assert (1 << (chan - 8)) <= 128; */
		(*muxsg1) = (*muxsg1) | (1 << (chan - 8));
		/*@ assert \at(*muxsg1, Pre) <= *muxsg1; */
	}
}
