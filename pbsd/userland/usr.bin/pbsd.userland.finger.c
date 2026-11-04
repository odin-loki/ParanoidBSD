/* Reference logic from hbsd/src/usr.bin/finger/finger.c (dual-world). */
#include <stddef.h>

int
finger_short_days(int idle_minutes)
{
	return idle_minutes < 6 * 24 * 60;
}

int
finger_is_ext4(const char *phone)
{
	int digits = 0;

	if (phone == NULL)
		return 0;
	for (; *phone != '\0'; ++phone) {
		if (*phone >= '0' && *phone <= '9')
			digits++;
		else if (*phone != ' ' && *phone != '-')
			return 0;
	}
	return digits == 4;
}
