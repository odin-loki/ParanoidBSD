static int atusb_alloc_urbs(struct atusb *atusb, int n)
{
	struct urb *urb;

	while (n) {
		urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!urb) {
			atusb_free_urbs(atusb);
			return -ENOMEM;
		}
		usb_anchor_urb(urb, &atusb->idle_urbs);
		usb_free_urb(urb);
		n--;
	}
	return 0;
}
