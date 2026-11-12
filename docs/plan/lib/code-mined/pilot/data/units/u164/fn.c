static __inline size_t
ttyinq_bytescanonicalized(struct ttyinq *ti)
{
	MPASS(ti->ti_begin <= ti->ti_linestart);

	return (ti->ti_linestart - ti->ti_begin);
}
