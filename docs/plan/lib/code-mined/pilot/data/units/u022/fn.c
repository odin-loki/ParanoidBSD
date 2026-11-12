static uint32_t fpscr_to_spefscr(uint32_t fpscr)
{
	uint32_t spefscr;

	spefscr = 0;

	if (fpscr & FPSCR_VX)
		spefscr |= SPEFSCR_FINV;
	if (fpscr & FPSCR_OX)
		spefscr |= SPEFSCR_FOVF;
	if (fpscr & FPSCR_UX)
		spefscr |= SPEFSCR_FUNF;
	if (fpscr & FPSCR_ZX)
		spefscr |= SPEFSCR_FDBZ;
	if (fpscr & FPSCR_XX)
		spefscr |= SPEFSCR_FX;

	return (spefscr);
}
