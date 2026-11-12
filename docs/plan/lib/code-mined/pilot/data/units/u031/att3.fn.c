static inline __attribute__ ((const)) int fls(unsigned int x)
{
	if (__builtin_constant_p(x))
	       return constant_fls(x);

	return 32 - clz(x);
}

/* Note: no annotations were inserted. The contract depends entirely on the
   result of the call to clz(x), for which neither code nor specification
   exists in this translation unit (Frama-C generates a default, unconstrained
   spec for it). Since only loop invariants, loop assigns, loop variants and
   asserts may be added, and there is no loop, the only usable annotation kind
   would be an assert relating clz(x) to the contract — but such an assert
   would itself be an unprovable goal (WP would have to know properties of
   clz that are not available), so it cannot make the ensures goal provable.
   The function is therefore returned unchanged, as required. */
