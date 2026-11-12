static int __init sbd_init(void)
{
	int i, ret;

	sbd_probe_duarts();

	ret = uart_register_driver(&sbd_reg);
	if (ret)
		return ret;

	for (i = 0; i < DUART_MAX_CHIP * DUART_MAX_SIDE; i++) {
		struct sbd_duart *duart = &sbd_duarts[i / DUART_MAX_SIDE];
		struct sbd_port *sport = &duart->sport[i % DUART_MAX_SIDE];
		struct uart_port *uport = &sport->port;

		if (sport->duart)
			uart_add_one_port(&sbd_reg, uport);
	}

	return 0;
}
