/*
 * b0132s2 oracle: hbsd/src/lib/libc/gen/utxdb.c with every function renamed
 * via the preprocessor so the bodies below are byte-for-byte the original text.
 */

/* -std=c11 hides the BSD/GNU extras; htobe32() and friends live behind this. */
#define _DEFAULT_SOURCE 1

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <endian.h>

struct utmpx {
	short		ut_type;
	struct timeval	ut_tv;
	char		ut_id[8];
	pid_t		ut_pid;
	char		ut_user[32];
	char		ut_line[16];
	char		ut_host[128];
	char		__ut_spare[64];
};

struct futx {
	uint8_t		fu_type;
	uint64_t	fu_tv;
	char		fu_id[8];
	uint32_t	fu_pid;
	char		fu_user[32];
	char		fu_line[16];
	char		fu_host[128];
} __attribute__((__packed__));

#define	MIN(a,b)	(((a)<(b))?(a):(b))

#define	EMPTY		0
#define	BOOT_TIME	1
#define	OLD_TIME	2
#define	NEW_TIME	3
#define	USER_PROCESS	4
#define	INIT_PROCESS	5
#define	LOGIN_PROCESS	6
#define	DEAD_PROCESS	7
#define	SHUTDOWN_TIME	8

#define	utx_to_futx		ref_utx_to_futx
#define	futx_to_utx		ref_futx_to_utx

#define	UTOF_STRING(ut, fu, field) do { \
	strncpy((fu)->fu_ ## field, (ut)->ut_ ## field,		\
	    MIN(sizeof (fu)->fu_ ## field, sizeof (ut)->ut_ ## field));	\
} while (0)
#define	UTOF_ID(ut, fu) do { \
	memcpy((fu)->fu_id, (ut)->ut_id,				\
	    MIN(sizeof (fu)->fu_id, sizeof (ut)->ut_id));		\
} while (0)
#define	UTOF_PID(ut, fu) do { \
	(fu)->fu_pid = htobe32((ut)->ut_pid);				\
} while (0)
#define	UTOF_TYPE(ut, fu) do { \
	(fu)->fu_type = (ut)->ut_type;					\
} while (0)
#define	UTOF_TV(fu) do { \
	struct timeval tv;						\
	gettimeofday(&tv, NULL);					\
	(fu)->fu_tv = htobe64((uint64_t)tv.tv_sec * 1000000 +		\
	    (uint64_t)tv.tv_usec);					\
} while (0)

void
utx_to_futx(const struct utmpx *ut, struct futx *fu)
{

	memset(fu, 0, sizeof *fu);

	switch (ut->ut_type) {
	case BOOT_TIME:
	case OLD_TIME:
	case NEW_TIME:
	/* Extension: shutdown time. */
	case SHUTDOWN_TIME:
		break;
	case USER_PROCESS:
		UTOF_ID(ut, fu);
		UTOF_STRING(ut, fu, user);
		UTOF_STRING(ut, fu, line);
		/* Extension: host name. */
		UTOF_STRING(ut, fu, host);
		UTOF_PID(ut, fu);
		break;
	case INIT_PROCESS:
		UTOF_ID(ut, fu);
		UTOF_PID(ut, fu);
		break;
	case LOGIN_PROCESS:
		UTOF_ID(ut, fu);
		UTOF_STRING(ut, fu, user);
		UTOF_STRING(ut, fu, line);
		UTOF_PID(ut, fu);
		break;
	case DEAD_PROCESS:
		UTOF_ID(ut, fu);
		UTOF_PID(ut, fu);
		break;
	default:
		fu->fu_type = EMPTY;
		return;
	}

	UTOF_TYPE(ut, fu);
	UTOF_TV(fu);
}

#define	FTOU_STRING(fu, ut, field) do { \
	strncpy((ut)->ut_ ## field, (fu)->fu_ ## field,		\
	    MIN(sizeof (ut)->ut_ ## field - 1, sizeof (fu)->fu_ ## field)); \
} while (0)
#define	FTOU_ID(fu, ut) do { \
	memcpy((ut)->ut_id, (fu)->fu_id,				\
	    MIN(sizeof (ut)->ut_id, sizeof (fu)->fu_id));		\
} while (0)
#define	FTOU_PID(fu, ut) do { \
	(ut)->ut_pid = be32toh((fu)->fu_pid);				\
} while (0)
#define	FTOU_TYPE(fu, ut) do { \
	(ut)->ut_type = (fu)->fu_type;					\
} while (0)
#define	FTOU_TV(fu, ut) do { \
	uint64_t t;							\
	t = be64toh((fu)->fu_tv);					\
	(ut)->ut_tv.tv_sec = t / 1000000;				\
	(ut)->ut_tv.tv_usec = t % 1000000;				\
} while (0)

struct utmpx *
futx_to_utx(const struct futx *fu)
{
	static _Thread_local struct utmpx *ut;

	if (ut == NULL) {
		ut = calloc(1, sizeof *ut);
		if (ut == NULL)
			return (NULL);
	} else
		memset(ut, 0, sizeof *ut);

	switch (fu->fu_type) {
	case BOOT_TIME:
	case OLD_TIME:
	case NEW_TIME:
	/* Extension: shutdown time. */
	case SHUTDOWN_TIME:
		break;
	case USER_PROCESS:
		FTOU_ID(fu, ut);
		FTOU_STRING(fu, ut, user);
		FTOU_STRING(fu, ut, line);
		/* Extension: host name. */
		FTOU_STRING(fu, ut, host);
		FTOU_PID(fu, ut);
		break;
	case INIT_PROCESS:
		FTOU_ID(fu, ut);
		FTOU_PID(fu, ut);
		break;
	case LOGIN_PROCESS:
		FTOU_ID(fu, ut);
		FTOU_STRING(fu, ut, user);
		FTOU_STRING(fu, ut, line);
		FTOU_PID(fu, ut);
		break;
	case DEAD_PROCESS:
		FTOU_ID(fu, ut);
		FTOU_PID(fu, ut);
		break;
	default:
		ut->ut_type = EMPTY;
		return (ut);
	}

	FTOU_TYPE(fu, ut);
	FTOU_TV(fu, ut);
	return (ut);
}
