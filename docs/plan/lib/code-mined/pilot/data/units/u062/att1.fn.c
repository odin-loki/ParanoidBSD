static int __init ad5446_init(void)
{
	int ret;

	ret = ad5446_spi_register_driver();
	if (ret)
		return ret;

	ret = ad5446_i2c_register_driver();
	if (ret) {
		ad5446_spi_unregister_driver();
		return ret;
	}

	return 0;
}
