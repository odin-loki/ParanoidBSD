static int gcm_out_walk_done(struct gcm_sg_walk *gw, unsigned int bytesdone)
{
	int i, n;

	if (gw->ptr == NULL)
		return 0;

	if (gw->ptr == gw->buf) {
		for (i = 0; i < bytesdone; i += n) {
			if (!_gcm_sg_clamp_and_map(gw))
				return i;
			n = min(gw->walk_bytes, bytesdone - i);
			memcpy(gw->walk.addr, gw->buf + i, n);
			_gcm_sg_unmap_and_advance(gw, n, true);
		}
	} else
		_gcm_sg_unmap_and_advance(gw, bytesdone, true);

	return bytesdone;
}
