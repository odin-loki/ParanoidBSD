int fb_alloc_cmap_gfp(struct fb_cmap *cmap, int len, int transp, gfp_t flags)
{
	int size = len * sizeof(u16);
	int ret = -ENOMEM;

	flags |= __GFP_NOWARN;

	if (cmap->len != len) {
		fb_dealloc_cmap(cmap);
		if (!len)
			return 0;

		cmap->red = kzalloc(size, flags);
		if (!cmap->red)
			goto fail;
		cmap->green = kzalloc(size, flags);
		if (!cmap->green)
			goto fail;
		cmap->blue = kzalloc(size, flags);
		if (!cmap->blue)
			goto fail;
		if (transp) {
			cmap->transp = kzalloc(size, flags);
			if (!cmap->transp)
				goto fail;
		} else {
			cmap->transp = NULL;
		}
	}
	cmap->start = 0;
	cmap->len = len;
	ret = fb_copy_cmap(fb_default_cmap(len), cmap);
	if (ret)
		goto fail;
	return 0;

fail:
	fb_dealloc_cmap(cmap);
	return ret;
}
