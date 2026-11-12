#include "prelude.h"
#undef SMBIOS_ENTRY_POINT_SCAN_START
#define SMBIOS_ENTRY_POINT_SCAN_START 0xF0000
struct kobject *dmi_kobj;
/*@ requires len >= 0; ensures \result == 1 || \result == 0; */
static int __init dmi_checksum(const u8 *buf, u8 len)
{
	u8 sum = 0;
	int a;

	for (a = 0; a < len; a++)
		sum += buf[a];

	return sum == 0;
}
