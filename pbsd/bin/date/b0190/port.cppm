module;

#define _GNU_SOURCE

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csetjmp>
#include <ctime>
#include <strings.h>

export module pbsd.bin.date.b0190;

export namespace pbsd::bin_date::b0190 {

struct vary {
	char *arg;
	vary *next;
};

struct trans {
	std::int64_t val;
	const char *str;
};

inline const trans *
trans_mon_table()
{
	static const trans t[] = {
		{ 1, "january" }, { 2, "february" }, { 3, "march" }, { 4, "april" },
		{ 5, "may"}, { 6, "june" }, { 7, "july" }, { 8, "august" },
		{ 9, "september" }, { 10, "october" }, { 11, "november" },
		{ 12, "december" },
		{ -1, nullptr }
	};
	return t;
}

inline const trans *
trans_wday_table()
{
	static const trans t[] = {
		{ 0, "sunday" }, { 1, "monday" }, { 2, "tuesday" },
		{ 3, "wednesday" }, { 4, "thursday" }, { 5, "friday" },
		{ 6, "saturday" },
		{ -1, nullptr }
	};
	return t;
}

inline const char *
digits_table()
{
	return "0123456789";
}

inline const int *
mdays_table()
{
	static const int t[12] = { 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	return t;
}

#define trans_mon trans_mon_table()
#define trans_wday trans_wday_table()
#define digits digits_table()
#define mdays mdays_table()

inline std::jmp_buf port_err_jmp;
inline int port_err_armed;
inline int port_err_called;
inline int port_err_status;

inline void
port_err_arm()
{
	port_err_armed = 1;
}

inline void
port_err_disarm()
{
	port_err_armed = 0;
}

inline void
err(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	port_err_called = 1;
	port_err_status = eval;
	if (port_err_armed)
		std::longjmp(port_err_jmp, 1);
	std::exit(eval);
}

int adjhour(std::tm *, char, std::int64_t, int);

inline int
domktime(std::tm *t, char type)
{
	time_t ret;

	while ((ret = mktime(t)) == -1 && t->tm_year > 68 && t->tm_year < 138)
		adjhour(t, type == '-' ? type : '+', 1, 0);

	return ret;
}

inline int
trans(const trans t[], const char *arg)
{
	int f;

	for (f = 0; t[f].val != -1; f++)
		if (!strncasecmp(t[f].str, arg, 3) ||
		    !strncasecmp(t[f].str, arg, std::strlen(t[f].str)))
			return t[f].val;

	return -1;
}

inline vary *
vary_append(vary *v, char *arg)
{
	vary *result, **nextp;

	if (v) {
		result = v;
		while (v->next)
			v = v->next;
		nextp = &v->next;
	} else
		nextp = &result;

	if ((*nextp = (vary *)std::malloc(sizeof(vary))) == nullptr)
		err(1, "malloc");
	(*nextp)->arg = arg;
	(*nextp)->next = nullptr;
	return result;
}

inline int
daysinmonth(const std::tm *t)
{
	int year;

	year = t->tm_year + 1900;

	if (t->tm_mon == 1)
		if (!(year % 400))
			return 29;
		else if (!(year % 100))
			return 28;
		else if (!(year % 4))
			return 29;
		else
			return 28;
	else if (t->tm_mon >= 0 && t->tm_mon < 12)
		return mdays[t->tm_mon];

	return 0;
}

inline int
adjyear(std::tm *t, char type, std::int64_t val, int mk)
{
	switch (type) {
		case '+':
			t->tm_year += val;
			break;
		case '-':
			t->tm_year -= val;
			break;
		default:
			t->tm_year = val;
			if (t->tm_year < 69)
				t->tm_year += 100;
			else if (t->tm_year > 1900)
				t->tm_year -= 1900;
			break;
	}
	return !mk || domktime(t, type) != -1;
}

inline int
adjmon(std::tm *t, char type, std::int64_t val, int istext, int mk)
{
	int lmdays;

	if (val < 0)
		return 0;

	switch (type) {
		case '+':
			if (istext) {
				if (val <= t->tm_mon)
					val += 11 - t->tm_mon;
				else
					val -= t->tm_mon + 1;
			}
			if (val) {
				if (!adjyear(t, '+', (t->tm_mon + val) / 12, 0))
					return 0;
				val %= 12;
				t->tm_mon += val;
				if (t->tm_mon > 11)
					t->tm_mon -= 12;
			}
			break;

		case '-':
			if (istext) {
				if (val-1 > t->tm_mon)
					val = 13 - val + t->tm_mon;
				else
					val = t->tm_mon - val + 1;
			}
			if (val) {
				if (!adjyear(t, '-', val / 12, 0))
					return 0;
				val %= 12;
				if (val > t->tm_mon) {
					if (!adjyear(t, '-', 1, 0))
						return 0;
					val -= 12;
				}
				t->tm_mon -= val;
			}
			break;

		default:
			if (val > 12 || val < 1)
				return 0;
			t->tm_mon = --val;
	}

	lmdays = daysinmonth(t);
	if (t->tm_mday > lmdays)
		t->tm_mday = lmdays;

	return !mk || domktime(t, type) != -1;
}

inline int
adjday(std::tm *t, char type, std::int64_t val, int mk)
{
	int lmdays;

	switch (type) {
		case '+':
			while (val) {
				lmdays = daysinmonth(t);
				if (val > lmdays - t->tm_mday) {
					val -= lmdays - t->tm_mday + 1;
					t->tm_mday = 1;
					if (!adjmon(t, '+', 1, 0, 0))
						return 0;
				} else {
					t->tm_mday += val;
					val = 0;
				}
			}
			break;
		case '-':
			while (val)
				if (val >= t->tm_mday) {
					val -= t->tm_mday;
					t->tm_mday = 1;
					if (!adjmon(t, '-', 1, 0, 0))
						return 0;
					t->tm_mday = daysinmonth(t);
				} else {
					t->tm_mday -= val;
					val = 0;
				}
			break;
		default:
			if (val > 0 && val <= daysinmonth(t))
				t->tm_mday = val;
			else
				return 0;
			break;
	}

	return !mk || domktime(t, type) != -1;
}

inline int
adjwday(std::tm *t, char type, std::int64_t val, int istext, int mk)
{
	if (val < 0)
		return 0;

	switch (type) {
		case '+':
			if (istext)
				if (val < t->tm_wday)
					val = 7 - t->tm_wday + val;
				else
					val -= t->tm_wday;
			else
				val *= 7;
			return !val || adjday(t, '+', val, mk);
		case '-':
			if (istext) {
				if (val > t->tm_wday)
					val = 7 - val + t->tm_wday;
				else
					val = t->tm_wday - val;
			} else
				val *= 7;
			return !val || adjday(t, '-', val, mk);
		default:
			if (val < t->tm_wday)
				return adjday(t, '-', t->tm_wday - val, mk);
			else if (val > 6)
				return 0;
			else if (val > t->tm_wday)
				return adjday(t, '+', val - t->tm_wday, mk);
	}
	return 1;
}

inline int
adjhour(std::tm *t, char type, std::int64_t val, int mk)
{
	if (val < 0)
		return 0;

	switch (type) {
		case '+':
			if (val) {
				int days;

				days = (t->tm_hour + val) / 24;
				val %= 24;
				t->tm_hour += val;
				t->tm_hour %= 24;
				if (!adjday(t, '+', days, 0))
					return 0;
			}
			break;

		case '-':
			if (val) {
				int days;

				days = val / 24;
				val %= 24;
				if (val > t->tm_hour) {
					days++;
					val -= 24;
				}
				t->tm_hour -= val;
				if (!adjday(t, '-', days, 0))
					return 0;
			}
			break;

		default:
			if (val > 23)
				return 0;
			t->tm_hour = val;
	}

	return !mk || domktime(t, type) != -1;
}

inline int
adjmin(std::tm *t, char type, std::int64_t val, int mk)
{
	if (val < 0)
		return 0;

	switch (type) {
		case '+':
			if (val) {
				if (!adjhour(t, '+', (t->tm_min + val) / 60, 0))
					return 0;
				val %= 60;
				t->tm_min += val;
				if (t->tm_min > 59)
					t->tm_min -= 60;
			}
			break;

		case '-':
			if (val) {
				if (!adjhour(t, '-', val / 60, 0))
					return 0;
				val %= 60;
				if (val > t->tm_min) {
					if (!adjhour(t, '-', 1, 0))
						return 0;
					val -= 60;
				}
				t->tm_min -= val;
			}
			break;

		default:
			if (val > 59)
				return 0;
			t->tm_min = val;
	}

	return !mk || domktime(t, type) != -1;
}

inline int
adjsec(std::tm *t, char type, std::int64_t val, int mk)
{
	if (val < 0)
		return 0;

	switch (type) {
		case '+':
			if (val) {
				if (!adjmin(t, '+', (t->tm_sec + val) / 60, 0))
					return 0;
				val %= 60;
				t->tm_sec += val;
				if (t->tm_sec > 59)
					t->tm_sec -= 60;
			}
			break;

		case '-':
			if (val) {
				if (!adjmin(t, '-', val / 60, 0))
					return 0;
				val %= 60;
				if (val > t->tm_sec) {
					if (!adjmin(t, '-', 1, 0))
						return 0;
					val -= 60;
				}
				t->tm_sec -= val;
			}
			break;

		default:
			if (val > 59)
				return 0;
			t->tm_sec = val;
	}

	return !mk || domktime(t, type) != -1;
}

inline const vary *
vary_apply(const vary *v, std::tm *t)
{
	char type;
	char which;
	char *arg;
	std::size_t len;
	std::int64_t val;

	for (; v; v = v->next) {
		type = *v->arg;
		arg = v->arg;
		if (type == '+' || type == '-')
			arg++;
		else
			type = '\0';
		len = std::strlen(arg);
		if (len < 2)
			return v;

		if (type == '\0')
			t->tm_isdst = -1;

		if (strspn(arg, digits) != len-1) {
			val = trans(trans_wday, arg);
			if (val != -1) {
				if (!adjwday(t, type, val, 1, 1))
					return v;
			} else {
				val = trans(trans_mon, arg);
				if (val != -1) {
					if (!adjmon(t, type, val, 1, 1))
						return v;
				} else
					return v;
			}
		} else {
			val = std::atoi(arg);
			which = arg[len-1];

			switch (which) {
				case 'S':
					if (!adjsec(t, type, val, 1))
						return v;
					break;
				case 'M':
					if (!adjmin(t, type, val, 1))
						return v;
					break;
				case 'H':
					if (!adjhour(t, type, val, 1))
						return v;
					break;
				case 'd':
					t->tm_isdst = -1;
					if (!adjday(t, type, val, 1))
						return v;
					break;
				case 'w':
					t->tm_isdst = -1;
					if (!adjwday(t, type, val, 0, 1))
						return v;
					break;
				case 'm':
					t->tm_isdst = -1;
					if (!adjmon(t, type, val, 0, 1))
						return v;
					break;
				case 'y':
					t->tm_isdst = -1;
					if (!adjyear(t, type, val, 1))
						return v;
					break;
				default:
					return v;
			}
		}
	}
	return 0;
}

inline void
vary_destroy(vary *v)
{
	vary *n;

	while (v) {
		n = v->next;
		std::free(v);
		v = n;
	}
}

inline std::size_t
strftime_ns(char * __restrict s, std::size_t maxsize, const char * __restrict format,
    const std::tm * __restrict t, long nsec, long res)
{
	std::size_t ret;
	char *newformat;
	char *oldformat;
	const char *prefix;
	const char *suffix;
	const char *tok;
	long number;
	int i, len, prefixlen, width, zeroes;
	bool seen_percent, seen_dash, seen_width;

	seen_percent = false;
	if ((newformat = strdup(format)) == nullptr)
		err(1, "strdup");
	tok = newformat;
	for (tok = newformat; *tok != '\0'; tok++) {
		switch (*tok) {
		case '%':
			if (seen_percent) {
				seen_percent = false;
			} else {
				seen_percent = true;
				seen_dash = seen_width = false;
				prefixlen = tok - newformat;
				width = 0;
			}
			break;
		case 'N':
			if (!seen_percent)
				break;
			oldformat = newformat;
			prefix = oldformat;
			suffix = tok + 1;
			if (seen_dash) {
				for (width = 9, number = res;
				     width > 0 && number > 0;
				     width--, number /= 10)
					;
			}
			number = nsec;
			zeroes = 0;
			if (width == 0) {
				width = 9;
			} else if (width > 9) {
				zeroes = width - 9;
				width = 9;
			} else {
				for (i = 0; i < 9 - width; i++)
					number /= 10;
			}
			(void)asprintf(&newformat, "%.*s%.*ld%.*d%n%s", prefixlen,
			    prefix, width, number, zeroes, 0, &len, suffix);
			if (newformat == nullptr)
				err(1, "asprintf");
			std::free(oldformat);
			tok = newformat + len - 1;
			seen_percent = false;
			break;
		case '-':
			if (seen_percent) {
				if (seen_dash || seen_width) {
					seen_percent = false;
					break;
				}
				seen_dash = true;
			}
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (seen_percent) {
				if (seen_dash) {
					seen_percent = false;
					break;
				}
				width = width * 10 + *tok - '0';
				seen_width = true;
			}
			break;
		default:
			seen_percent = false;
			break;
		}
	}

	ret = strftime(s, maxsize, newformat, t);
	std::free(newformat);
	return (ret);
}

}
