static uint64_t
ntb_plx_db_vector_mask(device_t dev, uint32_t vector)
{

	if (vector > 0)
		return (0);
	return ((1LL << PLX_NUM_DB) - 1);
}
