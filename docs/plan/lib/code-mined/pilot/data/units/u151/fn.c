static int uniphier_gpio_set_multiple(struct gpio_chip *chip,
				      unsigned long *mask, unsigned long *bits)
{
	unsigned long i, bank, bank_mask, bank_bits;

	for_each_set_clump8(i, bank_mask, mask, chip->ngpio) {
		bank = i / UNIPHIER_GPIO_LINES_PER_BANK;
		bank_bits = bitmap_get_value8(bits, i);

		uniphier_gpio_bank_write(chip, bank, UNIPHIER_GPIO_PORT_DATA,
					 bank_mask, bank_bits);
	}

	return 0;
}
