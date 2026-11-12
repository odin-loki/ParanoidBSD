int fb_alloc_cmap(struct fb_cmap *cmap, int len, int transp)
{
	return fb_alloc_cmap_gfp(cmap, len, transp, GFP_ATOMIC);
}
