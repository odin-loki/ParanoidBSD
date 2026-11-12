static int c_can_read_objects(struct net_device *dev, struct c_can_priv *priv,
			      u32 pend, int quota)
{
	u32 pkts = 0, ctrl, obj;

	while ((obj = ffs(pend)) && quota > 0) {
		pend &= ~BIT(obj - 1);

		c_can_rx_object_get(dev, priv, obj);
		ctrl = priv->read_reg(priv, C_CAN_IFACE(MSGCTRL_REG, IF_NAPI));

		if (ctrl & IF_MCONT_MSGLST) {
			int n;

			n = c_can_handle_lost_msg_obj(dev, IF_NAPI, obj, ctrl);

			pkts += n;
			quota -= n;
			continue;
		}

		/* This really should not happen, but this covers some
		 * odd HW behaviour. Do not remove that unless you
		 * want to brick your machine.
		 */
		if (!(ctrl & IF_MCONT_NEWDAT))
			continue;

		/* read the data from the message object */
		c_can_read_msg_object(dev, IF_NAPI, ctrl);

		c_can_rx_finalize(dev, priv, obj);

		pkts++;
		quota--;
	}

	return pkts;
}
