static inline u32 host1x_uclass_indoff_indroffset_f(u32 v)
{
	/*@ assert (v & 0xffff) <= 65535; */
	/*@ assert ((v & 0xffff) << 2) == (v & 0xffff) * 4; */
	/*@ assert ((v & 0xffff) * 4) <= 262140; */
	return (v & 0xffff) << 2;
}
