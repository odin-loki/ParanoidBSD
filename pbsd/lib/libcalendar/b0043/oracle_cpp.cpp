/*
 * Reference oracle for batch b0043 (lib/libcalendar).
 *
 * Original HardenedBSD sources concatenated, every function renamed with a
 * cpp_ prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libcalendar/calendar.c
 *   hbsd/src/lib/libcalendar/easter.c
 */

#ifndef NULL
#endif

struct date {
	int y;	/* year */
	int m;	/* month */
	int d;	/* day */
};


int cpp_weekday(int nd);

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libcalendar/calendar.c                                */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1997 Wolfgang Helbig
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * For each month tabulate the number of days elapsed in a year before the
 * month. This assumes the internal date representation, where a year
 * starts on March 1st. So we don't need a special table for leap years.
 * But we do need a special table for the year 1582, since 10 days are
 * deleted in October. This is month1s for the switch from Julian to
 * Gregorian calendar.
 */
static int const month1[] =
    {0, 31, 61, 92, 122, 153, 184, 214, 245, 275, 306, 337}; 
   /*  M   A   M   J    J    A    S    O    N    D    J */
static int const month1s[]=
    {0, 31, 61, 92, 122, 153, 184, 214, 235, 265, 296, 327}; 

typedef struct date date;

/* The last day of Julian calendar, in internal and ndays representation */
static int nswitch;	/* The last day of Julian calendar */
static date jiswitch = {1582, 7, 3};

static date	*cpp_date2idt(date *idt, date *dt);
static date	*cpp_idt2date(date *dt, date *idt);
static int	 cpp_ndaysji(date *idt);
static int	 cpp_ndaysgi(date *idt);
static int	 cpp_firstweek(int year);

/*
 * Compute the Julian date from the number of days elapsed since
 * March 1st of year zero.
 */
date *
cpp_jdate(int ndays, date *dt)
{
	date    idt;		/* Internal date representation */
	int     r;		/* hold the rest of days */

	/*
	 * Compute the year by starting with an approximation not smaller
	 * than the answer and using linear search for the greatest
	 * year which does not begin after ndays.
	 */
	idt.y = ndays / 365;
	idt.m = 0;
	idt.d = 0;
	while ((r = ndaysji(&idt)) > ndays)
		idt.y--;
	
	/*
	 * Set r to the days left in the year and compute the month by
	 * linear search as the largest month that does not begin after r
	 * days.
	 */
	r = ndays - r;
	for (idt.m = 11; month1[idt.m] > r; idt.m--)
		;

	/* Compute the days left in the month */
	idt.d = r - month1[idt.m];

	/* return external representation of the date */
	return (idt2date(dt, &idt));
}

/*
 * Return the number of days since March 1st of the year zero.
 * The date is given according to Julian calendar.
 */
int
cpp_ndaysj(date *dt)
{
	date    idt;		/* Internal date representation */

	if (date2idt(&idt, dt) == NULL)
		return (-1);
	else
		return (ndaysji(&idt));
}

/*
 * Same as above, where the Julian date is given in internal notation.
 * This formula shows the beauty of this notation.
 */
static int
cpp_ndaysji(date * idt)
{

	return (idt->d + month1[idt->m] + idt->y * 365 + idt->y / 4);
}

/*
 * Compute the date according to the Gregorian calendar from the number of
 * days since March 1st, year zero. The date computed will be Julian if it
 * is older than 1582-10-05. This is the reverse of the function ndaysg().
 */
date   *
cpp_gdate(int ndays, date *dt)
{
	int const *montht;	/* month-table */
	date    idt;		/* for internal date representation */
	int     r;		/* holds the rest of days */

	/*
	 * Compute the year by starting with an approximation not smaller
	 * than the answer and search linearly for the greatest year not
	 * starting after ndays.
	 */
	idt.y = ndays / 365;
	idt.m = 0;
	idt.d = 0;
	while ((r = ndaysgi(&idt)) > ndays)
		idt.y--;

	/*
	 * Set ndays to the number of days left and compute by linear
	 * search the greatest month which does not start after ndays. We
	 * use the table month1 which provides for each month the number
	 * of days that elapsed in the year before that month. Here the
	 * year 1582 is special, as 10 days are left out in October to
	 * resynchronize the calendar with the earth's orbit. October 4th
	 * 1582 is followed by October 15th 1582. We use the "switch"
	 * table month1s for this year.
	 */
	ndays = ndays - r;
	if (idt.y == 1582)
		montht = month1s;
	else
		montht = month1;

	for (idt.m = 11; montht[idt.m] > ndays; idt.m--)
		;

	idt.d = ndays - montht[idt.m]; /* the rest is the day in month */

	/* Advance ten days deleted from October if after switch in Oct 1582 */
	if (idt.y == jiswitch.y && idt.m == jiswitch.m && jiswitch.d < idt.d)
		idt.d += 10;

	/* return external representation of found date */
	return (idt2date(dt, &idt));
}

/*
 * Return the number of days since March 1st of the year zero. The date is
 * assumed Gregorian if younger than 1582-10-04 and Julian otherwise. This
 * is the reverse of gdate.
 */
int
cpp_ndaysg(date *dt)
{
	date    idt;		/* Internal date representation */

	if (date2idt(&idt, dt) == NULL)
		return (-1);
	return (ndaysgi(&idt));
}

/*
 * Same as above, but with the Gregorian date given in internal
 * representation.
 */
static int
cpp_ndaysgi(date *idt)
{
	int     nd;		/* Number of days--return value */

	/* Cache nswitch if not already done */
	if (nswitch == 0)
		nswitch = ndaysji(&jiswitch);

	/*
	 * Assume Julian calendar and adapt to Gregorian if necessary, i. e.
	 * younger than nswitch. Gregori deleted
	 * the ten days from Oct 5th to Oct 14th 1582.
	 * Thereafter years which are multiples of 100 and not multiples
	 * of 400 were not leap years anymore.
	 * This makes the average length of a year
	 * 365d +.25d - .01d + .0025d = 365.2425d. But the tropical
	 * year measures 365.2422d. So in 10000/3 years we are
	 * again one day ahead of the earth. Sigh :-)
	 * (d is the average length of a day and tropical year is the
	 * time from one spring point to the next.)
	 */
	if ((nd = ndaysji(idt)) == -1)
		return (-1);
	if (idt->y >= 1600)
		nd = (nd - 10 - (idt->y - 1600) / 100 + (idt->y - 1600) / 400);
	else if (nd > nswitch)
		nd -= 10;
	return (nd);
}

/*
 * Compute the week number from the number of days since March 1st year 0.
 * The weeks are numbered per year starting with 1. If the first
 * week of a year includes at least four days of that year it is week 1,
 * otherwise it gets the number of the last week of the previous year.
 * The variable y will be filled with the year that contains the greater
 * part of the week.
 */
int
cpp_week(int nd, int *y)
{
	date    dt;
	int     fw;		/* 1st day of week 1 of previous, this and
				 * next year */
	gdate(nd, &dt);
	for (*y = dt.y + 1; nd < (fw = firstweek(*y)); (*y)--)
		;
	return ((nd - fw) / 7 + 1);
}
		
/* return the first day of week 1 of year y */
static int
cpp_firstweek(int y)
{
	date idt;
	int nd, wd;

	idt.y = y - 1;   /* internal representation of y-1-1 */
	idt.m = 10;
	idt.d = 0;

	nd = ndaysgi(&idt);
	/*
	 * If more than 3 days of this week are in the preceding year, the
	 * next week is week 1 (and the next monday is the answer),
	 * otherwise this week is week 1 and the last monday is the
	 * answer.
	 */
	if ((wd = weekday(nd)) > 3)
		return (nd - wd + 7);
	else
		return (nd - wd);
}

/* return the weekday (Mo = 0 .. Su = 6) */
int
cpp_weekday(int nd)
{
	date dmondaygi = {1997, 8, 16}; /* Internal repr. of 1997-11-17 */
	static int nmonday;             /* ... which is a monday        */ 

	/* Cache the daynumber of one monday */
	if (nmonday == 0)
		nmonday = ndaysgi(&dmondaygi);

	/* return (nd - nmonday) modulo 7 which is the weekday */
	nd = (nd - nmonday) % 7;
	if (nd < 0)
		return (nd + 7);
	else
		return (nd);
}

/*
 * Convert a date to internal date representation: The year starts on
 * March 1st, month and day numbering start at zero. E. g. March 1st of
 * year zero is written as y=0, m=0, d=0.
 */
static date *
cpp_date2idt(date *idt, date *dt)
{

	idt->d = dt->d - 1;
	if (dt->m > 2) {
		idt->m = dt->m - 3;
		idt->y = dt->y;
	} else {
		idt->m = dt->m + 9;
		idt->y = dt->y - 1;
	}
	if (idt->m < 0 || idt->m > 11 || idt->y < 0)
		return (NULL);
	else
		return idt;
}

/* Reverse of date2idt */
static date *
cpp_idt2date(date *dt, date *idt)
{

	dt->d = idt->d + 1;
	if (idt->m < 10) {
		dt->m = idt->m + 3;
		dt->y = idt->y;
	} else {
		dt->m = idt->m - 9;
		dt->y = idt->y + 1;
	}
	if (dt->m < 1)
		return (NULL);
	else
		return (dt);
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libcalendar/easter.c                                  */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1997 Wolfgang Helbig
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static int	 cpp_easterodn(int y);

/* Compute Easter Sunday in Gregorian Calendar */
date *
cpp_easterg(int y, date *dt)
{
	int c, i, j, k, l, n;

	n = y % 19;
	c = y / 100;
	k = (c - 17) / 25;
	i = (c - c/4 -(c-k)/3 + 19 * n + 15) % 30;
	i = i -(i/28) * (1 - (i/28) * (29/(i + 1)) * ((21 - n)/11));
	j = (y + y/4 + i + 2 - c + c/4) % 7;
	l = i - j;
	dt->m = 3 + (l + 40) / 44;
	dt->d = l + 28 - 31*(dt->m / 4);
	dt->y = y;
	return (dt);
}

/* Compute the Gregorian date of Easter Sunday in Julian Calendar */
date *
cpp_easterog(int y, date *dt)
{

	return (gdate(easterodn(y), dt));
}

/* Compute the Julian date of Easter Sunday in Julian Calendar */
date   *
cpp_easteroj(int y, date * dt)
{

	return (jdate(easterodn(y), dt));
}

/* Compute the day number of Easter Sunday in Julian Calendar */
static int
cpp_easterodn(int y)
{
	/*
	 * Table for the easter limits in one metonic (19-year) cycle. 21
	 * to 31 is in March, 1 through 18 in April. Easter is the first
	 * sunday after the easter limit.
	 */
	int     mc[] = {5, 25, 13, 2, 22, 10, 30, 18, 7, 27, 15, 4,
		    24, 12, 1, 21, 9, 29, 17};

	/* Offset from a weekday to next sunday */
	int     ns[] = {6, 5, 4, 3, 2, 1, 7};
	date	dt;
	int     dn;

	/* Assign the easter limit of y to dt */
	dt.d = mc[y % 19];

	if (dt.d < 21)
		dt.m = 4;
	else
		dt.m = 3;

	dt.y = y;

	/* Return the next sunday after the easter limit */
	dn = ndaysj(&dt);
	return (dn + ns[weekday(dn)]);
}