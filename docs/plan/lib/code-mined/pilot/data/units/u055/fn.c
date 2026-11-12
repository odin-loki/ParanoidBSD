static int
tpcl_init(void)
{
	struct tpcl_softc *tsoftc = &tpcl_softc;
	struct ctl_port *port;
	struct scsi_transportid_spi *tid;
	int error, len;

	memset(tsoftc, 0, sizeof(*tsoftc));

	port = &tsoftc->port;
	port->frontend = &tpcl_frontend;
	port->port_type = CTL_PORT_INTERNAL;
	port->num_requested_ctl_io = 100;
	port->port_name = "tpc";
	port->fe_datamove = tpcl_datamove;
	port->fe_done = tpcl_done;
	port->targ_port = -1;
	port->max_initiators = 1;

	if ((error = ctl_port_register(port)) != 0) {
		printf("%s: tpc port registration failed\n", __func__);
		return (error);
	}

	len = sizeof(struct scsi_transportid_spi);
	port->init_devid = malloc(sizeof(struct ctl_devid) + len,
	    M_CTL, M_WAITOK | M_ZERO);
	port->init_devid->len = len;
	tid = (struct scsi_transportid_spi *)port->init_devid->data;
	tid->format_protocol = SCSI_TRN_SPI_FORMAT_DEFAULT | SCSI_PROTO_SPI;
	scsi_ulto2b(0, tid->scsi_addr);
	scsi_ulto2b(port->targ_port, tid->rel_trgt_port_id);

	ctl_port_online(port);
	return (0);
}
