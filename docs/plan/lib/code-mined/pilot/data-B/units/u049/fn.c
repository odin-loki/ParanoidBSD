static int
xbd_attach(device_t dev)
{
	struct xbd_softc *sc;
	const char *name;
	uint32_t vdevice;
	int error;
	int i;
	int unit;

	/* FIXME: Use dynamic device id if this is not set. */
	error = xs_scanf(XST_NIL, xenbus_get_node(dev),
	    "virtual-device", NULL, "%" PRIu32, &vdevice);
	if (error)
		error = xs_scanf(XST_NIL, xenbus_get_node(dev),
		    "virtual-device-ext", NULL, "%" PRIu32, &vdevice);
	if (error) {
		xenbus_dev_fatal(dev, error, "reading virtual-device");
		device_printf(dev, "Couldn't determine virtual device.\n");
		return (error);
	}

	xbd_vdevice_to_unit(vdevice, &unit, &name);
	if (!strcmp(name, "xbd"))
		device_set_unit(dev, unit);

	sc = device_get_softc(dev);
	mtx_init(&sc->xbd_io_lock, "blkfront i/o lock", NULL, MTX_DEF);
	xbd_initqs(sc);
	for (i = 0; i < XBD_MAX_RING_PAGES; i++)
		sc->xbd_ring_ref[i] = GRANT_REF_INVALID;

	sc->xbd_dev = dev;
	sc->xbd_vdevice = vdevice;
	sc->xbd_state = XBD_STATE_DISCONNECTED;

	xbd_setup_sysctl(sc);

	/* Wait for backend device to publish its protocol capabilities. */
	xenbus_set_state(dev, XenbusStateInitialising);

	return (0);
}
