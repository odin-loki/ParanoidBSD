static bool smu_v14_0_2_is_od_feature_supported(struct smu_context *smu,
						int od_feature_bit)
{
	PPTable_t *pptable = smu->smu_table.driver_pptable;
	const OverDriveLimits_t * const overdrive_upperlimits =
				&pptable->SkuTable.OverDriveLimitsBasicMax;

	return overdrive_upperlimits->FeatureCtrlMask & (1U << od_feature_bit);
}
