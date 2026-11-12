#include "prelude.h"
#undef BULK_TIMEOUT
#define BULK_TIMEOUT 1000
#undef FIRMWARE
#define FIRMWARE		"usbduxsigma_firmware.bin"
#undef FIRMWARE_MAX_LEN
#define FIRMWARE_MAX_LEN	0x4000
#undef USBDUXSUB_FIRMWARE
#define USBDUXSUB_FIRMWARE	0xa0
#undef VENDOR_DIR_IN
#define VENDOR_DIR_IN		0xc0
#undef VENDOR_DIR_OUT
#define VENDOR_DIR_OUT		0x40
#undef USBDUXSUB_CPUCS
#define USBDUXSUB_CPUCS 0xE600
#undef MIN_PWM_PERIOD
#define MIN_PWM_PERIOD  ((long)(1E9 / 300))
#undef PWM_DEFAULT_PERIOD
#define PWM_DEFAULT_PERIOD ((long)(1E9 / 100))
#undef NUMCHANNELS
#define NUMCHANNELS 16
#undef SIZEADIN
#define SIZEADIN          ((sizeof(u32)))
#undef SIZEINBUF
#define SIZEINBUF         (((NUMCHANNELS + 1) * SIZEADIN))
#undef SIZEINSNBUF
#define SIZEINSNBUF       16
#undef NUMOUTCHANNELS
#define NUMOUTCHANNELS    8
#undef SIZEDAOUT
#define SIZEDAOUT          ((sizeof(u8) + sizeof(uint16_t)))
#undef SIZEOUTBUF
#define SIZEOUTBUF         ((8 * SIZEDAOUT))
#undef SIZEOFDUXBUFFER
#define SIZEOFDUXBUFFER    ((8 * SIZEDAOUT + 2))
#undef NUMOFINBUFFERSFULL
#define NUMOFINBUFFERSFULL     5
#undef NUMOFOUTBUFFERSFULL
#define NUMOFOUTBUFFERSFULL    5
#undef NUMOFINBUFFERSHIGH
#define NUMOFINBUFFERSHIGH     10
#undef NUMOFOUTBUFFERSHIGH
#define NUMOFOUTBUFFERSHIGH    10
#undef RETRIES
#define RETRIES 10
#undef USBBUXSIGMA_AD_CMD
#define USBBUXSIGMA_AD_CMD		9
#undef USBDUXSIGMA_DA_CMD
#define USBDUXSIGMA_DA_CMD		1
#undef USBDUXSIGMA_DIO_CFG_CMD
#define USBDUXSIGMA_DIO_CFG_CMD		2
#undef USBDUXSIGMA_DIO_BITS_CMD
#define USBDUXSIGMA_DIO_BITS_CMD	3
#undef USBDUXSIGMA_SINGLE_AD_CMD
#define USBDUXSIGMA_SINGLE_AD_CMD	4
#undef USBDUXSIGMA_PWM_ON_CMD
#define USBDUXSIGMA_PWM_ON_CMD		7
#undef USBDUXSIGMA_PWM_OFF_CMD
#define USBDUXSIGMA_PWM_OFF_CMD		8
/*@ requires chan < 16; ensures \old(*muxsg0) <= *muxsg0 && \old(*muxsg1) <= *muxsg1; */
static void create_adc_command(unsigned int chan,
			       u8 *muxsg0, u8 *muxsg1)
{
	if (chan < 8)
		(*muxsg0) = (*muxsg0) | (1 << chan);
	else if (chan < 16)
		(*muxsg1) = (*muxsg1) | (1 << (chan - 8));
}
