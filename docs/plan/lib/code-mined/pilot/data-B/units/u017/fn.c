static int mt8192_dai_i2s_set_priv(struct mtk_base_afe *afe)
{
	int i;
	int ret;

	for (i = 0; i < DAI_I2S_NUM; i++) {
		ret = mt8192_dai_set_priv(afe, mt8192_i2s_priv[i].id,
					  sizeof(struct mtk_afe_i2s_priv),
					  &mt8192_i2s_priv[i]);
		if (ret)
			return ret;
	}

	return 0;
}
