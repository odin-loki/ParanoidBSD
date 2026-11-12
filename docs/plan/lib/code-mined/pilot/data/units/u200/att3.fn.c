static void create_adc_command(unsigned int chan,
			       u8 *muxsg0, u8 *muxsg1)
{
	if (chan < 8) {
		/*@ assert 0 <= chan && chan < 8; */
		/*@ assert (1 << chan) >= 1 && (1 << chan) <= 128; */
		/*@ assert \at(*muxsg0, Pre) <= (\at(*muxsg0, Pre) | (1 << chan))
		    && (\at(*muxsg0, Pre) | (1 << chan)) < 256; */
		(*muxsg0) = (*muxsg0) | (1 << chan);
		/*@ assert \at(*muxsg0, Pre) <= *muxsg0; */
	}
	else if (chan < 16) {
		/*@ assert 0 <= chan - 8 && chan - 8 < 8; */
		/*@ assert (1 << (chan - 8)) >= 1 && (1 << (chan - 8)) <= 128; */
		/*@ assert \at(*muxsg1, Pre) <= (\at(*muxsg1, Pre) | (1 << (chan - 8)))
		    && (\at(*muxsg1, Pre) | (1 << (chan - 8))) < 256; */
		(*muxsg1) = (*muxsg1) | (1 << (chan - 8));
		/*@ assert \at(*muxsg1, Pre) <= *muxsg1; */
	}
}
