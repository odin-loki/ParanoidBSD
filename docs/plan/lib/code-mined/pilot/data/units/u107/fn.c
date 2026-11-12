static int dsi_10nm_phy_parse_dt(struct msm_dsi_phy *phy)
{
	struct device *dev = &phy->pdev->dev;
	struct dsi_phy_10nm_tuning_cfg *tuning_cfg;
	s8 offset_top[DSI_LANE_MAX] = { 0 }; /* No offset */
	s8 offset_bot[DSI_LANE_MAX] = { 0 }; /* No offset */
	u32 ldo_level = 400; /* 400mV */
	u8 level;
	int ret, i;

	tuning_cfg = devm_kzalloc(dev, sizeof(*tuning_cfg), GFP_KERNEL);
	if (!tuning_cfg)
		return -ENOMEM;

	/* Drive strength adjustment parameters */
	ret = of_property_read_u8_array(dev->of_node, "qcom,phy-rescode-offset-top",
					offset_top, DSI_LANE_MAX);
	if (ret && ret != -EINVAL) {
		DRM_DEV_ERROR(dev, "failed to parse qcom,phy-rescode-offset-top, %d\n", ret);
		return ret;
	}

	for (i = 0; i < DSI_LANE_MAX; i++) {
		if (offset_top[i] < -32 || offset_top[i] > 31) {
			DRM_DEV_ERROR(dev,
				"qcom,phy-rescode-offset-top value %d is not in range [-32..31]\n",
				offset_top[i]);
			return -EINVAL;
		}
		tuning_cfg->rescode_offset_top[i] = 0x3f & offset_top[i];
	}

	ret = of_property_read_u8_array(dev->of_node, "qcom,phy-rescode-offset-bot",
					offset_bot, DSI_LANE_MAX);
	if (ret && ret != -EINVAL) {
		DRM_DEV_ERROR(dev, "failed to parse qcom,phy-rescode-offset-bot, %d\n", ret);
		return ret;
	}

	for (i = 0; i < DSI_LANE_MAX; i++) {
		if (offset_bot[i] < -32 || offset_bot[i] > 31) {
			DRM_DEV_ERROR(dev,
				"qcom,phy-rescode-offset-bot value %d is not in range [-32..31]\n",
				offset_bot[i]);
			return -EINVAL;
		}
		tuning_cfg->rescode_offset_bot[i] = 0x3f & offset_bot[i];
	}

	/* Drive level/amplitude adjustment parameters */
	ret = of_property_read_u32(dev->of_node, "qcom,phy-drive-ldo-level", &ldo_level);
	if (ret && ret != -EINVAL) {
		DRM_DEV_ERROR(dev, "failed to parse qcom,phy-drive-ldo-level, %d\n", ret);
		return ret;
	}

	switch (ldo_level) {
	case 375:
		level = 0;
		break;
	case 400:
		level = 1;
		break;
	case 425:
		level = 2;
		break;
	case 450:
		level = 3;
		break;
	case 475:
		level = 4;
		break;
	case 500:
		level = 5;
		break;
	default:
		DRM_DEV_ERROR(dev, "qcom,phy-drive-ldo-level %d is not supported\n", ldo_level);
		return -EINVAL;
	}
	tuning_cfg->vreg_ctrl = 0x58 | (0x7 & level);

	phy->tuning_cfg = tuning_cfg;

	return 0;
}
