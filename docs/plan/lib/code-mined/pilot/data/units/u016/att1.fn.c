static int
random_base(void)
{
	switch (rng_below(8)) {
	case 0: return 0;
	case 1: return 10;
	case 2: return 16;
	case 3: return 8;
	case 4: return 2;
	case 5: return 36;
	case 6: return (int)rng_below(40) - 2;
	default: return (int)rng_below(37);
	}
}
