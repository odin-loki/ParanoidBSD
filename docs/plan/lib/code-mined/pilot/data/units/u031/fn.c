static inline __attribute__ ((const)) int fls(unsigned int x)
{
	if (__builtin_constant_p(x))
	       return constant_fls(x);

	return 32 - clz(x);
}
