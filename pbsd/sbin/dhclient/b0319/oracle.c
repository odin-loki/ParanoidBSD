/*
 * oracle.c -- concatenated C reference for dhclient batch b0319.
 * Every function body is unmodified from the original sources apart from
 * the ref_ name prefix.
 */

#define _DEFAULT_SOURCE
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

typedef char *caddr_t;

typedef struct _pair {
	caddr_t car;
	struct _pair *cdr;
} *pair;

#define DEFAULT_HASH_SIZE 97

struct string_list {
	struct string_list	*next;
	char			*string;
};

struct hash_bucket {
	struct hash_bucket *next;
	const unsigned char *name;
	int len;
	unsigned char *value;
};

struct hash_table {
	int hash_count;
	struct hash_bucket *buckets[DEFAULT_HASH_SIZE];
};

struct iaddr {
	size_t len;
	unsigned char iabuf[16];
};

void
error(const char *fmt, ...)
{
	(void)fmt;
	abort();
}

/* ---- tree.c ---- */

pair
ref_cons(caddr_t car, pair cdr)
{
	pair foo = calloc(1, sizeof(*foo));
	if (!foo)
		error("no memory for cons.");
	foo->car = car;
	foo->cdr = cdr;
	return (foo);
}

/* ---- alloc.c ---- */

struct string_list *
ref_new_string_list(size_t size)
{
	struct string_list *rval;

	rval = calloc(1, sizeof(struct string_list) + size);
	if (rval != NULL)
		rval->string = ((char *)rval) + sizeof(struct string_list);
	return (rval);
}

struct hash_table *
ref_new_hash_table(int count)
{
	struct hash_table *rval;

	rval = calloc(1, sizeof(struct hash_table) -
	    (DEFAULT_HASH_SIZE * sizeof(struct hash_bucket *)) +
	    (count * sizeof(struct hash_bucket *)));
	if (rval == NULL)
		return (NULL);
	rval->hash_count = count;
	return (rval);
}

struct hash_bucket *
ref_new_hash_bucket(void)
{
	struct hash_bucket *rval = calloc(1, sizeof(struct hash_bucket));

	return (rval);
}

/* ---- convert.c ---- */

uint32_t
ref_getULong(unsigned char *buf)
{
	uint32_t ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohl(ibuf));
}

int32_t
ref_getLong(unsigned char *(buf))
{
	int32_t ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohl(ibuf));
}

uint16_t
ref_getUShort(unsigned char *buf)
{
	uint16_t ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohs(ibuf));
}

int16_t
ref_getShort(unsigned char *buf)
{
	int16_t ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohs(ibuf));
}

void
ref_putULong(unsigned char *obuf, uint32_t val)
{
	uint32_t tmp = htonl(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
ref_putLong(unsigned char *obuf, int32_t val)
{
	int32_t tmp = htonl(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
ref_putUShort(unsigned char *obuf, unsigned int val)
{
	uint16_t tmp = htons(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
ref_putShort(unsigned char *obuf, int val)
{
	int16_t tmp = htons(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

/* ---- inet.c ---- */

struct iaddr
ref_subnet_number(struct iaddr addr, struct iaddr mask)
{
	struct iaddr rv;
	unsigned i;

	rv.len = 0;

	/* Both addresses must have the same length... */
	if (addr.len != mask.len)
		return (rv);

	rv.len = addr.len;
	for (i = 0; i < rv.len; i++)
		rv.iabuf[i] = addr.iabuf[i] & mask.iabuf[i];
	return (rv);
}

struct iaddr
ref_broadcast_addr(struct iaddr subnet, struct iaddr mask)
{
	struct iaddr rv;
	unsigned i;

	if (subnet.len != mask.len) {
		rv.len = 0;
		return (rv);
	}

	for (i = 0; i < subnet.len; i++)
		rv.iabuf[i] = subnet.iabuf[i] | (~mask.iabuf[i] & 255);
	rv.len = subnet.len;

	return (rv);
}

int
ref_addr_eq(struct iaddr addr1, struct iaddr addr2)
{
	if (addr1.len != addr2.len)
		return (0);
	return (memcmp(addr1.iabuf, addr2.iabuf, addr1.len) == 0);
}

char *
ref_piaddr(struct iaddr addr)
{
	static char pbuf[32];
	struct in_addr a;
	char *s;

	memcpy(&a, &(addr.iabuf), sizeof(struct in_addr));

	if (addr.len == 0)
		strlcpy(pbuf, "<null address>", sizeof(pbuf));
	else {
		s = inet_ntoa(a);
		if (s != NULL)
			strlcpy(pbuf, s, sizeof(pbuf));
		else
			strlcpy(pbuf, "<invalid address>", sizeof(pbuf));
	}
	return (pbuf);
}
