static std::size_t
need_for_phash(long long elements)
{
	unsigned long long b;
	const unsigned long long lim = 32749ULL * 8ULL + 8ULL;

	if (elements <= 0)
		return PRE + 64 + POST;
	b = static_cast<unsigned long long>(elements) * 8ULL + 8ULL;
	if (b > lim)
		b = lim;
	return clamp_need(b);
}
