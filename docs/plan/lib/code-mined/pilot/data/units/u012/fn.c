int
rng_int()
{
	std::uint32_t r = rng_next();

	switch (r & 3u) {
	case 0:
		/* Full 32-bit range. */
		return static_cast<int>(rng_next());
	case 1:
		/* Small non-negative, exercises the low fd/options range. */
		return static_cast<int>(rng_next() % 64u);
	case 2:
		/* Small negative. */
		return -static_cast<int>(rng_next() % 64u) - 1;
	default: {
		/* Boundary values and single high-bit bytes. */
		const std::size_t n = sizeof(g_edge_ints) /
		    sizeof(g_edge_ints[0]);
		return g_edge_ints[rng_next() % n];
	}
	}
}
