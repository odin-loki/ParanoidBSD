static __always_inline bool riscv_insn_is_system(u32 code)
{
	return (code & RV_INSN_OPCODE_MASK) == RVG_OPCODE_SYSTEM;
}
