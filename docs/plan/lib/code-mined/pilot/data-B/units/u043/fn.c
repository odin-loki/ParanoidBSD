static int cx25821_risc_decode(u32 risc)
{
	static const char * const instr[16] = {
		[RISC_SYNC >> 28] = "sync",
		[RISC_WRITE >> 28] = "write",
		[RISC_WRITEC >> 28] = "writec",
		[RISC_READ >> 28] = "read",
		[RISC_READC >> 28] = "readc",
		[RISC_JUMP >> 28] = "jump",
		[RISC_SKIP >> 28] = "skip",
		[RISC_WRITERM >> 28] = "writerm",
		[RISC_WRITECM >> 28] = "writecm",
		[RISC_WRITECR >> 28] = "writecr",
	};
	static const int incr[16] = {
		[RISC_WRITE >> 28] = 3,
		[RISC_JUMP >> 28] = 3,
		[RISC_SKIP >> 28] = 1,
		[RISC_SYNC >> 28] = 1,
		[RISC_WRITERM >> 28] = 3,
		[RISC_WRITECM >> 28] = 3,
		[RISC_WRITECR >> 28] = 4,
	};
	static const char * const bits[] = {
		"12", "13", "14", "resync",
		"cnt0", "cnt1", "18", "19",
		"20", "21", "22", "23",
		"irq1", "irq2", "eol", "sol",
	};
	int i;

	pr_cont("0x%08x [ %s",
		risc, instr[risc >> 28] ? instr[risc >> 28] : "INVALID");
	for (i = ARRAY_SIZE(bits) - 1; i >= 0; i--) {
		if (risc & (1 << (i + 12)))
			pr_cont(" %s", bits[i]);
	}
	pr_cont(" count=%d ]\n", risc & 0xfff);
	return incr[risc >> 28] ? incr[risc >> 28] : 1;
}
