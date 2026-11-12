#include "prelude.h"
#undef bitrev4
#define bitrev4(tmp) (bitrev8(tmp) >> 4)
#undef phy_stacksave
#define phy_stacksave(offset)					 	do {							 		_stack_save(stack, &stackidx, 0x1, (offset),	 			    b43_phy_read(dev, (offset)));	 	} while (0)
#undef phy_stackrestore
#define phy_stackrestore(offset)				 	do {							 		b43_phy_write(dev, (offset),		 				  _stack_restore(stack, 0x1,	 						 (offset)));	 	} while (0)
#undef radio_stacksave
#define radio_stacksave(offset)						 	do {								 		_stack_save(stack, &stackidx, 0x2, (offset),		 			    b43_radio_read16(dev, (offset)));	 	} while (0)
#undef radio_stackrestore
#define radio_stackrestore(offset)					 	do {								 		b43_radio_write16(dev, (offset),			 				      _stack_restore(stack, 0x2,	 						     (offset)));	 	} while (0)
#undef ofdmtab_stacksave
#define ofdmtab_stacksave(table, offset)			 	do {							 		_stack_save(stack, &stackidx, 0x3, (offset)|(table),	 			    b43_ofdmtab_read16(dev, (table), (offset)));	 	} while (0)
#undef ofdmtab_stackrestore
#define ofdmtab_stackrestore(table, offset)			 	do {							 		b43_ofdmtab_write16(dev, (table),	(offset),	 				  _stack_restore(stack, 0x3,	 						 (offset)|(table)));	 	} while (0)
#undef LPD
#define LPD(L, P, D)	(((L) << 2) | ((P) << 1) | ((D) << 0))
struct init2050_saved_values { 	 	u16 reg_3EC; 	u16 reg_3E6; 	u16 reg_3F4; 	 	u16 radio_43; 	u16 radio_51; 	u16 radio_52; 	 	u16 phy_pgactl; 	u16 phy_cck_5A; 	u16 phy_cck_59; 	u16 phy_cck_58; 	u16 phy_cck_30; 	u16 phy_rfover; 	u16 phy_rfoverval; 	u16 phy_analogover; 	u16 phy_analogoverval; 	u16 phy_crs0; 	u16 phy_classctl; 	u16 phy_lo_mask; 	u16 phy_lo_ctl; 	u16 phy_syncctl; };
/*@ requires true; ensures (1 <= new_channel <= new_channel <= 14) ==> \result == 0; ensures (new_channel < 1 || new_channel > 14) ==> \result == -EINVAL; */
static int b43_gphy_op_switch_channel(struct b43_wldev *dev,
				      unsigned int new_channel)
{
	if ((new_channel < 1) || (new_channel > 14))
		return -EINVAL;
	b43_gphy_channel_switch(dev, new_channel, 0);

	return 0;
}
