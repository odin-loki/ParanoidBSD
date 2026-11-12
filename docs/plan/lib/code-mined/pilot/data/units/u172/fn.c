static void l440gx_set_vpp(struct map_info *map, int vpp)
{
	unsigned long flags;

	spin_lock_irqsave(&l440gx_vpp_lock, flags);
	if (vpp) {
		if (++l440gx_vpp_refcnt == 1)   /* first nested 'on' */
			outl(inl(VPP_PORT) | 1, VPP_PORT);
	} else {
		if (--l440gx_vpp_refcnt == 0)   /* last nested 'off' */
			outl(inl(VPP_PORT) & ~1, VPP_PORT);
	}
	spin_unlock_irqrestore(&l440gx_vpp_lock, flags);
}
