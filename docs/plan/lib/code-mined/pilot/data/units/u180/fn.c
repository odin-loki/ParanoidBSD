static int
gpiobus_acquire_child_pins(device_t dev, device_t child)
{
	struct gpiobus_ivar *devi = GPIOBUS_IVAR(child);
	int i;

	for (i = 0; i < devi->npins; i++) {
		/* Reserve the GPIO pin. */
		if (gpiobus_acquire_pin(dev, devi->pins[i]) != 0) {
			device_printf(child, "cannot acquire pin %d\n",
			    devi->pins[i]);
			while (--i >= 0) {
				gpiobus_release_pin(dev, devi->pins[i]);
			}
			gpiobus_free_ivars(devi);
			return (EBUSY);
		}
	}
	for (i = 0; i < devi->npins; i++) {
		/* Use the child name as pin name. */
		GPIOBUS_PIN_SETNAME(dev, devi->pins[i],
		    device_get_nameunit(child));

	}
	return (0);
}
