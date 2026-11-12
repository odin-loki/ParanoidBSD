#include "prelude.h"
#undef EMIT
#define EMIT(bytes, len)  	do { prog = emit_code(prog, bytes, len); } while (0)
#undef EMIT1
#define EMIT1(b1)		EMIT(b1, 1)
#undef EMIT2
#define EMIT2(b1, b2)		EMIT((b1) + ((b2) << 8), 2)
#undef EMIT3
#define EMIT3(b1, b2, b3)	EMIT((b1) + ((b2) << 8) + ((b3) << 16), 3)
#undef EMIT4
#define EMIT4(b1, b2, b3, b4)   EMIT((b1) + ((b2) << 8) + ((b3) << 16) + ((b4) << 24), 4)
#undef EMIT5
#define EMIT5(b1, b2, b3, b4, b5)  	do { EMIT1(b1); EMIT4(b2, b3, b4, b5); } while (0)
#undef EMIT1_off32
#define EMIT1_off32(b1, off)  	do { EMIT1(b1); EMIT(off, 4); } while (0)
#undef EMIT2_off32
#define EMIT2_off32(b1, b2, off)  	do { EMIT2(b1, b2); EMIT(off, 4); } while (0)
#undef EMIT3_off32
#define EMIT3_off32(b1, b2, b3, off)  	do { EMIT3(b1, b2, b3); EMIT(off, 4); } while (0)
#undef EMIT4_off32
#define EMIT4_off32(b1, b2, b3, b4, off)  	do { EMIT4(b1, b2, b3, b4); EMIT(off, 4); } while (0)
#undef EMIT_mov
#define EMIT_mov(DST, SRC)								  	do {										  		if (DST != SRC)								  			EMIT3(add_2mod(0x48, DST, SRC), 0x89, add_2reg(0xC0, DST, SRC));  	} while (0)
#undef X86_JB
#define X86_JB  0x72
#undef X86_JAE
#define X86_JAE 0x73
#undef X86_JE
#define X86_JE  0x74
#undef X86_JNE
#define X86_JNE 0x75
#undef X86_JBE
#define X86_JBE 0x76
#undef X86_JA
#define X86_JA  0x77
#undef X86_JL
#define X86_JL  0x7C
#undef X86_JGE
#define X86_JGE 0x7D
#undef X86_JLE
#define X86_JLE 0x7E
#undef X86_JG
#define X86_JG  0x7F
#undef AUX_REG
#define AUX_REG (MAX_BPF_JIT_REG + 1)
#undef X86_REG_R9
#define X86_REG_R9 (MAX_BPF_JIT_REG + 2)
#undef X86_REG_R12
#define X86_REG_R12 (MAX_BPF_JIT_REG + 3)
#undef BPF_MAX_INSN_SIZE
#define BPF_MAX_INSN_SIZE	128
#undef BPF_INSN_SAFETY
#define BPF_INSN_SAFETY		64
#undef X86_PATCH_SIZE
#define X86_PATCH_SIZE		5
#undef X86_TAIL_CALL_OFFSET
#define X86_TAIL_CALL_OFFSET	(12 + ENDBR_INSN_SIZE)
#undef EMIT_LFENCE
#define EMIT_LFENCE()	EMIT3(0x0F, 0xAE, 0xE8)
#undef BPF_TAIL_CALL_CNT_PTR_STACK_OFF
#define BPF_TAIL_CALL_CNT_PTR_STACK_OFF(stack)	(-16 - round_up(stack, 8))
#undef DONT_CLEAR
#define DONT_CLEAR 1
#undef FIXUP_INSN_LEN_MASK
#define FIXUP_INSN_LEN_MASK	GENMASK(7, 0)
#undef FIXUP_REG_MASK
#define FIXUP_REG_MASK		GENMASK(15, 8)
#undef FIXUP_ARENA_REG_MASK
#define FIXUP_ARENA_REG_MASK	GENMASK(23, 16)
#undef FIXUP_ARENA_ACCESS
#define FIXUP_ARENA_ACCESS	BIT(31)
#undef DATA_ARENA_OFFSET_MASK
#define DATA_ARENA_OFFSET_MASK	GENMASK(31, 16)
#undef INSN_SZ_DIFF
#define INSN_SZ_DIFF (((addrs[i] - addrs[i - 1]) - (prog - temp)))
#undef __LOAD_TCC_PTR
#define __LOAD_TCC_PTR(off)			 	EMIT3_off32(0x48, 0x8B, 0x85, off)
#undef LOAD_TAIL_CALL_CNT_PTR
#define LOAD_TAIL_CALL_CNT_PTR(stack)				 	__LOAD_TCC_PTR(BPF_TAIL_CALL_CNT_PTR_STACK_OFF(stack))
#undef PRIV_STACK_GUARD_SZ
#define PRIV_STACK_GUARD_SZ    8
#undef PRIV_STACK_GUARD_VAL
#define PRIV_STACK_GUARD_VAL   0xEB9F12345678eb9fULL
#undef LOAD_TRAMP_TAIL_CALL_CNT_PTR
#define LOAD_TRAMP_TAIL_CALL_CNT_PTR(stack)	 	__LOAD_TCC_PTR(-round_up(stack, 8) - 8)
#undef MAX_PASSES
#define MAX_PASSES 20
#undef PADDING_PASSES
#define PADDING_PASSES (MAX_PASSES - 5)
struct jit_context { 	int cleanup_addr;   	     	int tail_call_direct_label; 	int tail_call_indirect_label; };
struct x64_jit_data { 	struct bpf_binary_header *rw_header; 	struct bpf_binary_header *header; 	int *addrs; 	u8 *image; 	int proglen; 	struct jit_context ctx; };
/*@ requires true; ensures \result == (value <= 123 && value >= -128); */
static bool is_imm8_jmp_offset(int value)
{
	return value <= 123 && value >= -128;
}
