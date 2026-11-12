static u32 cond11(const u32 *r, const u32 *imm) {
	return (r[0] != r[1] && r[2] == imm[0]);
}
