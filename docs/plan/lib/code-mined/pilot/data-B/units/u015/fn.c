static u32 msm_dp_panel_get_supported_bpp(struct msm_dp_panel *msm_dp_panel,
		u32 mode_edid_bpp, u32 mode_pclk_khz)
{
	const struct msm_dp_link_info *link_info;
	const u32 max_supported_bpp = 30, min_supported_bpp = 18;
	u32 bpp, data_rate_khz;

	bpp = min(mode_edid_bpp, max_supported_bpp);

	link_info = &msm_dp_panel->link_info;
	data_rate_khz = link_info->num_lanes * link_info->rate * 8;

	do {
		if (mode_pclk_khz * bpp <= data_rate_khz)
			return bpp;
		bpp -= 6;
	} while (bpp > min_supported_bpp);

	return min_supported_bpp;
}
