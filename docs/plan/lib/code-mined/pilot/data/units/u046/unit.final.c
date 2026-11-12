#include "prelude.h"
#undef OMAP4430_VDD_IVA_I2C_DISABLE
#define OMAP4430_VDD_IVA_I2C_DISABLE		BIT(14)
#undef OMAP4430_VDD_MPU_I2C_DISABLE
#define OMAP4430_VDD_MPU_I2C_DISABLE		BIT(13)
#undef OMAP4430_VDD_CORE_I2C_DISABLE
#define OMAP4430_VDD_CORE_I2C_DISABLE		BIT(12)
#undef OMAP4430_VDD_IVA_PRESENCE
#define OMAP4430_VDD_IVA_PRESENCE		BIT(9)
#undef OMAP4430_VDD_MPU_PRESENCE
#define OMAP4430_VDD_MPU_PRESENCE		BIT(8)
#undef OMAP4430_AUTO_CTRL_VDD_IVA
#define OMAP4430_AUTO_CTRL_VDD_IVA(x)		((x) << 4)
#undef OMAP4430_AUTO_CTRL_VDD_MPU
#define OMAP4430_AUTO_CTRL_VDD_MPU(x)		((x) << 2)
#undef OMAP4430_AUTO_CTRL_VDD_CORE
#define OMAP4430_AUTO_CTRL_VDD_CORE(x)		((x) << 0)
#undef OMAP4430_AUTO_CTRL_VDD_RET
#define OMAP4430_AUTO_CTRL_VDD_RET		2
#undef OMAP4430_VDD_I2C_DISABLE_MASK
#define OMAP4430_VDD_I2C_DISABLE_MASK	 	(OMAP4430_VDD_IVA_I2C_DISABLE |  	 OMAP4430_VDD_MPU_I2C_DISABLE |  	 OMAP4430_VDD_CORE_I2C_DISABLE)
#undef OMAP4_VDD_DEFAULT_VAL
#define OMAP4_VDD_DEFAULT_VAL	 	(OMAP4430_VDD_I2C_DISABLE_MASK |  	 OMAP4430_VDD_IVA_PRESENCE | OMAP4430_VDD_MPU_PRESENCE |  	 OMAP4430_AUTO_CTRL_VDD_IVA(OMAP4430_AUTO_CTRL_VDD_RET) |  	 OMAP4430_AUTO_CTRL_VDD_MPU(OMAP4430_AUTO_CTRL_VDD_RET) |  	 OMAP4430_AUTO_CTRL_VDD_CORE(OMAP4430_AUTO_CTRL_VDD_RET))
#undef OMAP4_VDD_RET_VAL
#define OMAP4_VDD_RET_VAL	 	(OMAP4_VDD_DEFAULT_VAL & ~OMAP4430_VDD_I2C_DISABLE_MASK)
#undef CFG_CHANNEL_MASK
#define CFG_CHANNEL_MASK 0x1f
struct omap_vc_channel_cfg { 	u8 sa; 	u8 rav; 	u8 rac; 	u8 racen; 	u8 cmd; };
struct omap3_vc_timings { 	u32 voltsetup1; 	u32 voltsetup2; };
struct omap3_vc { 	struct voltagedomain *vd; 	u32 voltctrl; 	u32 voltsetup1; 	u32 voltsetup2; 	struct omap3_vc_timings timings[2]; };
struct i2c_init_data { 	u8 loadbits; 	u8 load; 	u8 hsscll_38_4; 	u8 hsscll_26; 	u8 hsscll_19_2; 	u8 hsscll_16_8; 	u8 hsscll_12; };
/*@ requires usec <= 4294967295; ensures \result == (32768 * usec + 999999) / 1000000; */
static inline u32 omap_usec_to_32k(u32 usec)
{
	return DIV_ROUND_UP_ULL(32768ULL * (u64)usec, 1000000ULL);
}
