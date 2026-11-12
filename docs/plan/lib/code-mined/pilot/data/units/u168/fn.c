static int read_raw(struct ldc_channel *lp, void *buf, unsigned int size)
{
	struct ldc_packet *p;
	unsigned long hv_err, new;
	int err;

	if (size < LDC_PACKET_SIZE)
		return -EINVAL;

	hv_err = sun4v_ldc_rx_get_state(lp->id,
					&lp->rx_head,
					&lp->rx_tail,
					&lp->chan_state);
	if (hv_err)
		return LDC_ABORT(lp);

	if (lp->chan_state == LDC_CHANNEL_DOWN ||
	    lp->chan_state == LDC_CHANNEL_RESETTING)
		return -ECONNRESET;

	if (lp->rx_head == lp->rx_tail)
		return 0;

	p = lp->rx_base + (lp->rx_head / LDC_PACKET_SIZE);
	memcpy(buf, p, LDC_PACKET_SIZE);

	new = rx_advance(lp, lp->rx_head);
	lp->rx_head = new;

	err = __set_rx_head(lp, new);
	if (err < 0)
		err = -ECONNRESET;
	else
		err = LDC_PACKET_SIZE;

	return err;
}
