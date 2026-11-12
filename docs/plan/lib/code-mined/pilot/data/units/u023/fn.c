static bool is_imm8_jmp_offset(int value)
{
	return value <= 123 && value >= -128;
}
