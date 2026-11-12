static int adxl345_set_freefall_time(struct adxl345_state *st, u32 val_fract)
{
	/*
	 * Datasheet max. value is 255 * 5000 us = 1.275000 seconds.
	 *
	 * Recommended values between 100ms and 350ms (0x14 to 0x46)
	 */
	st->inact_time_ms = DIV_ROUND_UP(val_fract, MILLI);

	return regmap_write(st->regmap, ADXL345_REG_TIME_FF,
			    DIV_ROUND_CLOSEST(val_fract, 5));
}
