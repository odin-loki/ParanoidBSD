#include "prelude.h"
#undef _GNU_SOURCE
#define _GNU_SOURCE
#undef IP4_HLEN
#define IP4_HLEN	(sizeof(struct iphdr))
#undef IP6_HLEN
#define IP6_HLEN	(sizeof(struct ip6_hdr))
#undef UDP_HLEN
#define UDP_HLEN	(sizeof(struct udphdr))
#undef FRAG_HLEN
#define FRAG_HLEN	8
#undef MSG_LEN_MAX
#define MSG_LEN_MAX	10000	
#undef IP4_MF
#define IP4_MF		(1u << 13)  
#undef IP6_MF
#define IP6_MF		(1)  
#undef CSUM_MANGLED_0
#define CSUM_MANGLED_0 (0xffff)
/*@ requires len >= 0; ensures \result <= 0xffff; */
static uint32_t raw_checksum(uint8_t *buf, int len, uint32_t sum)
{
	int i;

	for (i = 0; i < (len & ~1U); i += 2) {
		sum += (u_int16_t)ntohs(*((u_int16_t *)(buf + i)));
		if (sum > 0xffff)
			sum -= 0xffff;
	}

	if (i < len) {
		sum += buf[i] << 8;
		if (sum > 0xffff)
			sum -= 0xffff;
	}

	return sum;
}
