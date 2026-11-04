/* Does CSA treat a visible inline function whose body is inline asm as
 * writing through its pointer argument? */
#include <stddef.h>

struct req { unsigned char type; unsigned char code; unsigned short val; };

/* (1) the shape of bus_space_read_multi_1: visible, inline, asm body */
static __inline void
read_multi(unsigned long port, unsigned char *addr, size_t count)
{
	__asm __volatile("				\n\
	1:	movb (%2),%%al				\n\
		stosb					\n\
		loop 1b"				:
	    "=D" (addr), "=c" (count)			:
	    "r" (port), "0" (addr), "1" (count)		:
	    "%eax", "memory");
}

/* (2) the control: an UNKNOWN function, declared and not defined */
void unknown_fill(void *addr, size_t count);

int
via_inline_asm(unsigned long port)
{
	struct req r;
	read_multi(port, (unsigned char *)&r, sizeof(r));
	if (r.type == 0x40)          /* <-- reported? */
		return (1);
	return (0);
}

int
via_unknown_call(void)
{
	struct req r;
	unknown_fill(&r, sizeof(r));
	if (r.type == 0x40)          /* <-- control: must NOT be reported */
		return (1);
	return (0);
}

/* (3) a second control: nothing writes it at all, so it MUST be reported */
int
never_written(void)
{
	struct req r;
	if (r.type == 0x40)
		return (1);
	return (0);
}
