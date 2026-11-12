static inline bool arch_validate_prot(unsigned long prot, unsigned long addr)
{
	return (prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC | PROT_SEM)) == 0;
}
