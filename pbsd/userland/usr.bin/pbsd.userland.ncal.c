/* Reference logic from hbsd/src/usr.bin/ncal/ncal.c (dual-world). */
#include <stddef.h>

static int
is_leap(int year)
{
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int
days_in_month(int month, int year)
{
	static const int mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if (month < 1 || month > 12)
		return 0;
	if (month == 2 && is_leap(year))
		return 29;
	return mdays[month - 1];
}

static int
weekday_jan1(int year)
{
	/* Zeller congruence variant for Gregorian calendar. */
	int y = year;
	int m = 1;
	if (m < 3) {
		m += 12;
		y -= 1;
	}
	int k = y % 100;
	int j = y / 100;
	int h = (1 + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
	return (h + 6) % 7; /* 0=Sunday */
}

int
ncal_month_valid(int month)
{
	return month >= 1 && month <= 12;
}

int
ncal_year_valid(int year)
{
	return year >= 1 && year <= 9999;
}

int
ncal_day_offset(int year, int month, int day)
{
	if (!ncal_year_valid(year) || !ncal_month_valid(month))
		return -1;
	if (day < 1 || day > days_in_month(month, year))
		return -1;
	int offset = weekday_jan1(year);
	for (int m = 1; m < month; ++m)
		offset = (offset + days_in_month(m, year)) % 7;
	offset = (offset + day - 1) % 7;
	return offset;
}
