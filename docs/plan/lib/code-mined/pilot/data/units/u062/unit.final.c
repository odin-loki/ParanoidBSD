#include "prelude.h"
#undef MODE_PWRDWN_1k
#define MODE_PWRDWN_1k		0x1
#undef MODE_PWRDWN_100k
#define MODE_PWRDWN_100k	0x2
#undef MODE_PWRDWN_TRISTATE
#define MODE_PWRDWN_TRISTATE	0x3
#undef _AD5446_CHANNEL
#define _AD5446_CHANNEL(bits, storage, _shift, ext) {  	.type = IIO_VOLTAGE,  	.indexed = 1,  	.output = 1,  	.channel = 0,  	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),  	.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),  	.scan_type = {  		.sign = 'u',  		.realbits = (bits),  		.storagebits = (storage),  		.shift = (_shift),  		},  	.ext_info = (ext),  }
#undef AD5446_CHANNEL
#define AD5446_CHANNEL(bits, storage, shift)  	_AD5446_CHANNEL(bits, storage, shift, NULL)
#undef AD5446_CHANNEL_POWERDOWN
#define AD5446_CHANNEL_POWERDOWN(bits, storage, shift)  	_AD5446_CHANNEL(bits, storage, shift, ad5446_ext_info_powerdown)
/*@ requires true; ensures (\result == 0 || \result < 0); */
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
