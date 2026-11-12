static int kona_pwmc_config(struct pwm_chip *chip, struct pwm_device *pwm,
			    u64 duty_ns, u64 period_ns)
{
	struct kona_pwmc *kp = to_kona_pwmc(chip);
	u64 div, rate;
	unsigned long prescale = PRESCALE_MIN, pc, dc;
	unsigned int value, chan = pwm->hwpwm;

	/*
	 * Find period count, duty count and prescale to suit duty_ns and
	 * period_ns. This is done according to formulas described below:
	 *
	 * period_ns = 10^9 * (PRESCALE + 1) * PC / PWM_CLK_RATE
	 * duty_ns = 10^9 * (PRESCALE + 1) * DC / PWM_CLK_RATE
	 *
	 * PC = (PWM_CLK_RATE * period_ns) / (10^9 * (PRESCALE + 1))
	 * DC = (PWM_CLK_RATE * duty_ns) / (10^9 * (PRESCALE + 1))
	 */

	rate = clk_get_rate(kp->clk);

	while (1) {
		div = 1000000000;
		div *= 1 + prescale;
		pc = mul_u64_u64_div_u64(rate, period_ns, div);
		dc = mul_u64_u64_div_u64(rate, duty_ns, div);

		/* If duty_ns or period_ns are not achievable then return */
		if (pc < PERIOD_COUNT_MIN)
			return -EINVAL;

		/* If pc and dc are in bounds, the calculation is done */
		if (pc <= PERIOD_COUNT_MAX && dc <= DUTY_CYCLE_HIGH_MAX)
			break;

		/* Otherwise, increase prescale and recalculate pc and dc */
		if (++prescale > PRESCALE_MAX)
			return -EINVAL;
	}

	kona_pwmc_prepare_for_settings(kp, chan);

	value = readl(kp->base + PRESCALE_OFFSET);
	value &= ~PRESCALE_MASK(chan);
	value |= prescale << PRESCALE_SHIFT(chan);
	writel(value, kp->base + PRESCALE_OFFSET);

	writel(pc, kp->base + PERIOD_COUNT_OFFSET(chan));

	writel(dc, kp->base + DUTY_CYCLE_HIGH_OFFSET(chan));

	kona_pwmc_apply_settings(kp, chan);

	return 0;
}
