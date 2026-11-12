static int worker(void *arg)
{
	struct pt_bcache_entry exp;
	struct pt_block_cache *bcache;
	uint64_t iter, index;

	bcache = arg;
	if (!bcache)
		return -pte_internal;

	memset(&exp, 0x00, sizeof(exp));
	exp.ninsn = 5;
	exp.displacement = 28;
	exp.mode = ptem_64bit;
	exp.qualifier = ptbq_again;
	exp.isize = 3;

	for (index = 0; index < bfix_nentries; ++index) {
		for (iter = 0; iter < bfix_iterations; ++iter) {
			struct pt_bcache_entry bce;
			int errcode;

			memset(&bce, 0xff, sizeof(bce));

			errcode = pt_bcache_lookup(&bce, bcache, index);
			if (errcode < 0)
				return errcode;

			if (!pt_bce_is_valid(bce)) {
				errcode = pt_bcache_add(bcache, index, exp);
				if (errcode < 0)
					return errcode;
			}

			errcode = pt_bcache_lookup(&bce, bcache, index);
			if (errcode < 0)
				return errcode;

			if (!pt_bce_is_valid(bce))
				return -pte_nosync;

			if (bce.ninsn != exp.ninsn)
				return -pte_nosync;

			if (bce.displacement != exp.displacement)
				return -pte_nosync;

			if (pt_bce_exec_mode(bce) != pt_bce_exec_mode(exp))
				return -pte_nosync;

			if (pt_bce_qualifier(bce) != pt_bce_qualifier(exp))
				return -pte_nosync;

			if (bce.isize != exp.isize)
				return -pte_nosync;
		}
	}

	return 0;
}
