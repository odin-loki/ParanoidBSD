static int intel_th_gth_probe(struct intel_th_device *thdev)
{
	struct device *dev = &thdev->dev;
	struct intel_th *th = dev_get_drvdata(dev->parent);
	struct gth_device *gth;
	struct resource *res;
	void __iomem *base;
	int i, ret;

	res = intel_th_device_get_resource(thdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	base = devm_ioremap(dev, res->start, resource_size(res));
	if (!base)
		return -ENOMEM;

	gth = devm_kzalloc(dev, sizeof(*gth), GFP_KERNEL);
	if (!gth)
		return -ENOMEM;

	gth->dev = dev;
	gth->base = base;
	spin_lock_init(&gth->gth_lock);

	dev_set_drvdata(dev, gth);

	/*
	 * Host mode can be signalled via SW means or via SCRPD_DEBUGGER_IN_USE
	 * bit. Either way, don't reset HW in this case, and don't export any
	 * capture configuration attributes. Also, refuse to assign output
	 * drivers to ports, see intel_th_gth_assign().
	 */
	if (thdev->host_mode)
		return 0;

	ret = intel_th_gth_reset(gth);
	if (ret) {
		if (ret != -EBUSY)
			return ret;

		thdev->host_mode = true;

		return 0;
	}

	for (i = 0; i < TH_CONFIGURABLE_MASTERS + 1; i++)
		gth->master[i] = -1;

	for (i = 0; i < TH_POSSIBLE_OUTPUTS; i++) {
		gth->output[i].gth = gth;
		gth->output[i].index = i;
		gth->output[i].port_type =
			gth_output_parm_get(gth, i, TH_OUTPUT_PARM(port));
		if (gth->output[i].port_type == GTH_NONE)
			continue;

		ret = intel_th_output_enable(th, gth->output[i].port_type);
		/* -ENODEV is ok, we just won't have that device enumerated */
		if (ret && ret != -ENODEV)
			return ret;
	}

	if (intel_th_output_attributes(gth) ||
	    intel_th_master_attributes(gth)) {
		pr_warn("Can't initialize sysfs attributes\n");

		if (gth->output_group.attrs)
			sysfs_remove_group(&gth->dev->kobj, &gth->output_group);
		return -ENOMEM;
	}

	return 0;
}
