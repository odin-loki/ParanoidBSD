#include "prelude.h"
#undef _BPF_JIT_H
#define _BPF_JIT_H
#undef rv_ext_enabled
#define rv_ext_enabled(ext)  	(IS_ENABLED(CONFIG_RISCV_ISA_##ext) && riscv_has_extension_likely(RISCV_ISA_EXT_##ext))
enum { 	RV_REG_ZERO =	0,	 	RV_REG_RA =	1,	 	RV_REG_SP =	2,	 	RV_REG_GP =	3,	 	RV_REG_TP =	4,	 	RV_REG_T0 =	5,	 	RV_REG_T1 =	6, 	RV_REG_T2 =	7, 	RV_REG_FP =	8,	 	RV_REG_S1 =	9,	 	RV_REG_A0 =	10,	 	RV_REG_A1 =	11,	 	RV_REG_A2 =	12, 	RV_REG_A3 =	13, 	RV_REG_A4 =	14, 	RV_REG_A5 =	15, 	RV_REG_A6 =	16, 	RV_REG_A7 =	17, 	RV_REG_S2 =	18,	 	RV_REG_S3 =	19, 	RV_REG_S4 =	20, 	RV_REG_S5 =	21, 	RV_REG_S6 =	22, 	RV_REG_S7 =	23, 	RV_REG_S8 =	24, 	RV_REG_S9 =	25, 	RV_REG_S10 =	26, 	RV_REG_S11 =	27, 	RV_REG_T3 =	28,	 	RV_REG_T4 =	29, 	RV_REG_T5 =	30, 	RV_REG_T6 =	31, };
struct rv_jit_context { 	struct bpf_prog *prog; 	u16 *insns;		 	u16 *ro_insns; 	int ninsns; 	int prologue_len; 	int epilogue_offset; 	int *offset;		 	int nexentries; 	int ex_insn_off; 	int ex_jmp_off; 	unsigned long flags; 	int stack_size; 	u64 arena_vm_start; 	u64 user_vm_start; };
struct rv_jit_data { 	struct bpf_binary_header *header; 	struct bpf_binary_header *ro_header; 	u8 *image; 	u8 *ro_image; 	struct rv_jit_context ctx; };
/*@ requires true; ensures \result == (val < (1UL << 10)); */
static inline bool is_10b_uint(unsigned long val)
{
	return val < (1UL << 10);
}
