int
r21a_init_bcnq1_boundary(struct rtwn_softc *sc)
{
#define RTWN_CHK(res) do {	\
	if (res != 0)		\
		return (EIO);	\
} while(0)
	RTWN_CHK(rtwn_write_1(sc, R88E_TXPKTBUF_BCNQ1_BDNY,
	    R21A_BCNQ0_BOUNDARY));
	RTWN_CHK(rtwn_write_1(sc, R21A_DWBCN1_CTRL + 1,
	    R21A_BCNQ0_BOUNDARY));
	RTWN_CHK(rtwn_setbits_1_shift(sc, R21A_DWBCN1_CTRL, 0,
	    R21A_DWBCN1_CTRL_SEL_EN, 2));

	return (0);
#undef RTWN_CHK
}
