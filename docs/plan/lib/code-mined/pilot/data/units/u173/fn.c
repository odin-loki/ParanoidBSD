static int huawei_cdc_ncm_manage_power(struct usbnet *usbnet_dev, int on)
{
	struct huawei_cdc_ncm_state *drvstate = (void *)&usbnet_dev->data;
	int rv;

	if ((on && atomic_add_return(1, &drvstate->pmcount) == 1) ||
			(!on && atomic_dec_and_test(&drvstate->pmcount))) {
		rv = usb_autopm_get_interface(usbnet_dev->intf);
		usbnet_dev->intf->needs_remote_wakeup = on;
		if (!rv)
			usb_autopm_put_interface(usbnet_dev->intf);
	}
	return 0;
}
