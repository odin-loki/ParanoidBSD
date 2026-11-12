static u32 vfp_double_fsqrt(int dd, int unused, int dm, u32 fpscr)
{
	struct vfp_double vdm, vdd;
	int ret, tm;

	vfp_double_unpack(&vdm, vfp_get_double(dm));
	tm = vfp_double_type(&vdm);
	if (tm & (VFP_NAN|VFP_INFINITY)) {
		struct vfp_double *vdp = &vdd;

		if (tm & VFP_NAN)
			ret = vfp_propagate_nan(vdp, &vdm, NULL, fpscr);
		else if (vdm.sign == 0) {
 sqrt_copy:
			vdp = &vdm;
			ret = 0;
		} else {
 sqrt_invalid:
			vdp = &vfp_double_default_qnan;
			ret = FPSCR_IOC;
		}
		vfp_put_double(vfp_double_pack(vdp), dd);
		return ret;
	}

	/*
	 * sqrt(+/- 0) == +/- 0
	 */
	if (tm & VFP_ZERO)
		goto sqrt_copy;

	/*
	 * Normalise a denormalised number
	 */
	if (tm & VFP_DENORMAL)
		vfp_double_normalise_denormal(&vdm);

	/*
	 * sqrt(<0) = invalid
	 */
	if (vdm.sign)
		goto sqrt_invalid;

	vfp_double_dump("sqrt", &vdm);

	/*
	 * Estimate the square root.
	 */
	vdd.sign = 0;
	vdd.exponent = ((vdm.exponent - 1023) >> 1) + 1023;
	vdd.significand = (u64)vfp_estimate_sqrt_significand(vdm.exponent, vdm.significand >> 32) << 31;

	vfp_double_dump("sqrt estimate1", &vdd);

	vdm.significand >>= 1 + (vdm.exponent & 1);
	vdd.significand += 2 + vfp_estimate_div128to64(vdm.significand, 0, vdd.significand);

	vfp_double_dump("sqrt estimate2", &vdd);

	/*
	 * And now adjust.
	 */
	if ((vdd.significand & VFP_DOUBLE_LOW_BITS_MASK) <= 5) {
		if (vdd.significand < 2) {
			vdd.significand = ~0ULL;
		} else {
			u64 termh, terml, remh, reml;
			vdm.significand <<= 2;
			mul64to128(&termh, &terml, vdd.significand, vdd.significand);
			sub128(&remh, &reml, vdm.significand, 0, termh, terml);
			while ((s64)remh < 0) {
				vdd.significand -= 1;
				shift64left(&termh, &terml, vdd.significand);
				terml |= 1;
				add128(&remh, &reml, remh, reml, termh, terml);
			}
			vdd.significand |= (remh | reml) != 0;
		}
	}
	vdd.significand = vfp_shiftright64jamming(vdd.significand, 1);

	return vfp_double_normaliseround(dd, &vdd, fpscr, 0, "fsqrt");
}
