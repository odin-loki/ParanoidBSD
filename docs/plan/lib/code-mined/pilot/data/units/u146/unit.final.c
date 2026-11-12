#include "prelude.h"
#undef NTB_NAME
#define NTB_NAME	"ntb_hw_idt"
#undef NTB_DESC
#define NTB_DESC	"IDT PCI-E Non-Transparent Bridge Driver"
#undef NTB_VER
#define NTB_VER		"2.0"
#undef NTB_IRQNAME
#define NTB_IRQNAME	"ntb_irq_idt"
#undef IDT_NT_INDBELLSTS
#define IDT_NT_INDBELLSTS		0x00428U
/*@ requires true; ensures \result == 0; */
static int idt_ntb_db_clear(struct ntb_dev *ntb, u64 db_bits)
{
	struct idt_ntb_dev *ndev = to_ndev_ntb(ntb);

	idt_nt_write(ndev, IDT_NT_INDBELLSTS, (u32)db_bits);

	return 0;
}
