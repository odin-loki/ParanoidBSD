static __always_inline long arch_atomic64_dec_if_positive(atomic64_t *v)
{
	return __lse_ll_sc_body(atomic64_dec_if_positive, v);
}
