static unsigned int pci_oxsemi_tornado_get_divisor(struct uart_port *port,
						   unsigned int baud,
						   unsigned int *frac)
{
	static u8 p[][2] = {
		{ 16, 14, }, { 16, 13, }, { 16, 12, }, { 16, 11, },
		{ 16, 10, }, { 16,  9, }, { 16,  8, }, { 15, 17, },
		{ 15, 16, }, { 15, 15, }, { 15, 14, }, { 15, 13, },
		{ 15, 12, }, { 15, 11, }, { 15, 10, }, { 15,  9, },
		{ 15,  8, }, { 14, 18, }, { 14, 17, }, { 14, 14, },
		{ 14, 13, }, { 14, 12, }, { 14, 11, }, { 14, 10, },
		{ 14,  9, }, { 14,  8, }, { 13, 19, }, { 13, 18, },
		{ 13, 17, }, { 13, 13, }, { 13, 12, }, { 13, 11, },
		{ 13, 10, }, { 13,  9, }, { 13,  8, }, { 12, 19, },
		{ 12, 18, }, { 12, 17, }, { 12, 11, }, { 12,  9, },
		{ 12,  8, }, { 11, 23, }, { 11, 22, }, { 11, 21, },
		{ 11, 20, }, { 11, 19, }, { 11, 18, }, { 11, 17, },
		{ 11, 11, }, { 11, 10, }, { 11,  9, }, { 11,  8, },
		{ 10, 25, }, { 10, 23, }, { 10, 20, }, { 10, 19, },
		{ 10, 17, }, { 10, 10, }, { 10,  9, }, { 10,  8, },
		{  9, 27, }, {  9, 23, }, {  9, 21, }, {  9, 19, },
		{  9, 18, }, {  9, 17, }, {  9,  9, }, {  9,  8, },
		{  8, 31, }, {  8, 29, }, {  8, 23, }, {  8, 19, },
		{  8, 17, }, {  8,  8, }, {  7, 35, }, {  7, 31, },
		{  7, 29, }, {  7, 25, }, {  7, 23, }, {  7, 21, },
		{  7, 19, }, {  7, 17, }, {  7, 15, }, {  7, 14, },
		{  7, 13, }, {  7, 12, }, {  7, 11, }, {  7, 10, },
		{  7,  9, }, {  7,  8, }, {  6, 41, }, {  6, 37, },
		{  6, 31, }, {  6, 29, }, {  6, 23, }, {  6, 19, },
		{  6, 17, }, {  6, 13, }, {  6, 11, }, {  6, 10, },
		{  6,  9, }, {  6,  8, }, {  5, 67, }, {  5, 47, },
		{  5, 43, }, {  5, 41, }, {  5, 37, }, {  5, 31, },
		{  5, 29, }, {  5, 25, }, {  5, 23, }, {  5, 19, },
		{  5, 17, }, {  5, 15, }, {  5, 13, }, {  5, 11, },
		{  5, 10, }, {  5,  9, }, {  5,  8, }, {  4, 61, },
		{  4, 59, }, {  4, 53, }, {  4, 47, }, {  4, 43, },
		{  4, 41, }, {  4, 37, }, {  4, 31, }, {  4, 29, },
		{  4, 23, }, {  4, 19, }, {  4, 17, }, {  4, 13, },
		{  4,  9, }, {  4,  8, },
	};
	/* Scale the quotient for comparison to get the fractional part.  */
	const unsigned int quot_scale = 65536;
	unsigned int sclk = port->uartclk * 2;
	unsigned int sdiv = DIV_ROUND_CLOSEST(sclk, baud);
	unsigned int best_squot;
	unsigned int squot;
	unsigned int quot;
	u16 cpr;
	u8 tcr;
	int i;

	/* Old custom speed handling.  */
	if (baud == 38400 && (port->flags & UPF_SPD_MASK) == UPF_SPD_CUST) {
		unsigned int cust_div = port->custom_divisor;

		quot = cust_div & UART_DIV_MAX;
		tcr = (cust_div >> 16) & OXSEMI_TORNADO_TCR_MASK;
		cpr = (cust_div >> 20) & OXSEMI_TORNADO_CPR_MASK;
		if (cpr < OXSEMI_TORNADO_CPR_MIN)
			cpr = OXSEMI_TORNADO_CPR_DEF;
	} else {
		best_squot = quot_scale;
		for (i = 0; i < ARRAY_SIZE(p); i++) {
			unsigned int spre;
			unsigned int srem;
			u8 cp;
			u8 tc;

			tc = p[i][0];
			cp = p[i][1];
			spre = tc * cp;

			srem = sdiv % spre;
			if (srem > spre / 2)
				srem = spre - srem;
			squot = DIV_ROUND_CLOSEST(srem * quot_scale, spre);

			if (srem == 0) {
				tcr = tc;
				cpr = cp;
				quot = sdiv / spre;
				break;
			} else if (squot < best_squot) {
				best_squot = squot;
				tcr = tc;
				cpr = cp;
				quot = DIV_ROUND_CLOSEST(sdiv, spre);
			}
		}
		while (tcr <= (OXSEMI_TORNADO_TCR_MASK + 1) >> 1 &&
		       quot % 2 == 0) {
			quot >>= 1;
			tcr <<= 1;
		}
		while (quot > UART_DIV_MAX) {
			if (tcr <= (OXSEMI_TORNADO_TCR_MASK + 1) >> 1) {
				quot >>= 1;
				tcr <<= 1;
			} else if (cpr <= OXSEMI_TORNADO_CPR_MASK >> 1) {
				quot >>= 1;
				cpr <<= 1;
			} else {
				quot = quot * cpr / OXSEMI_TORNADO_CPR_MASK;
				cpr = OXSEMI_TORNADO_CPR_MASK;
			}
		}
	}

	*frac = (cpr << 8) | (tcr & OXSEMI_TORNADO_TCR_MASK);
	return quot;
}
