static unsigned int fun_port_type(unsigned int xcvr)
{
	if (!xcvr)
		return PORT_NONE;

	switch (xcvr & 7) {
	case FUN_XCVR_BASET:
		return PORT_TP;
	case FUN_XCVR_CU:
		return PORT_DA;
	default:
		return PORT_FIBRE;
	}
}
