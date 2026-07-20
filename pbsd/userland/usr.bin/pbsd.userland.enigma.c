/* Reference logic from hbsd/src/usr.bin/enigma/enigma.c (dual-world). */
#define ROTORSZ 256
#define MASK 0377

static unsigned char t1[ROTORSZ];
static unsigned char t2[ROTORSZ];
static unsigned char t3[ROTORSZ];
static unsigned char deck[ROTORSZ];
static int n1, n2;

static void
enigma_init(void)
{
	for (int i = 0; i < ROTORSZ; i++) {
		t1[i] = (unsigned char)i;
		t2[i] = (unsigned char)i;
		t3[i] = 0;
		deck[i] = (unsigned char)i;
	}
	n1 = n2 = 0;
}

static unsigned char
enigma_byte(unsigned char byte, int secure)
{
	int nr1, nr2, i, out;

	if (secure) {
		nr1 = deck[n1] & MASK;
		nr2 = deck[nr1] & MASK;
	} else {
		nr1 = n1;
		nr2 = n2;
	}
	i = (int)byte;
	out = t2[(t3[(t1[(i + nr1) & MASK] + nr2) & MASK] - nr2) & MASK] - nr1;
	n1++;
	if (n1 == ROTORSZ) {
		n1 = 0;
		n2++;
		if (n2 == ROTORSZ)
			n2 = 0;
	}
	return (unsigned char)(out & MASK);
}
