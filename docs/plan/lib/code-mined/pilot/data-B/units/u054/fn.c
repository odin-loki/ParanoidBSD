static int qedr_prepare_pbl_tbl(struct qedr_dev *dev,
				struct qedr_pbl_info *pbl_info,
				u32 num_pbes, int two_layer_capable)
{
	u32 pbl_capacity;
	u32 pbl_size;
	u32 num_pbls;

	if ((num_pbes > MAX_PBES_ON_PAGE) && two_layer_capable) {
		if (num_pbes > MAX_PBES_TWO_LAYER) {
			DP_ERR(dev, "prepare pbl table: too many pages %d\n",
			       num_pbes);
			return -EINVAL;
		}

		/* calculate required pbl page size */
		pbl_size = MIN_FW_PBL_PAGE_SIZE;
		pbl_capacity = NUM_PBES_ON_PAGE(pbl_size) *
			       NUM_PBES_ON_PAGE(pbl_size);

		while (pbl_capacity < num_pbes) {
			pbl_size *= 2;
			pbl_capacity = pbl_size / sizeof(u64);
			pbl_capacity = pbl_capacity * pbl_capacity;
		}

		num_pbls = DIV_ROUND_UP(num_pbes, NUM_PBES_ON_PAGE(pbl_size));
		num_pbls++;	/* One for the layer0 ( points to the pbls) */
		pbl_info->two_layered = true;
	} else {
		/* One layered PBL */
		num_pbls = 1;
		pbl_size = max_t(u32, MIN_FW_PBL_PAGE_SIZE,
				 roundup_pow_of_two((num_pbes * sizeof(u64))));
		pbl_info->two_layered = false;
	}

	pbl_info->num_pbls = num_pbls;
	pbl_info->pbl_size = pbl_size;
	pbl_info->num_pbes = num_pbes;

	DP_DEBUG(dev, QEDR_MSG_MR,
		 "prepare pbl table: num_pbes=%d, num_pbls=%d, pbl_size=%d\n",
		 pbl_info->num_pbes, pbl_info->num_pbls, pbl_info->pbl_size);

	return 0;
}
