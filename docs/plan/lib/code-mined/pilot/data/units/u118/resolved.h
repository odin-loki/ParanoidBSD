#undef X86_EFLAGS_IF
#define X86_EFLAGS_IF		_BITUL(X86_EFLAGS_IF_BIT)
#undef X86_EFLAGS_IF_BIT
#define X86_EFLAGS_IF_BIT	9
#undef _BITUL
#define _BITUL(x)	(_UL(1) << (x))
#undef _UL
#define _UL(x)		(_AC(x, UL))
