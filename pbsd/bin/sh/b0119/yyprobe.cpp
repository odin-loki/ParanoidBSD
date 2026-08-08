#include <cstdio>
#include <cstring>
#include <cstdlib>
import pbsd.bin.sh.b0119;
namespace port = pbsd::bin_sh::b0119;
extern "C" int ref_yylex(void);
extern "C" const char *arith_buf;
extern "C" union yystype { long long val; char *name; } yylval;

static const char *yy_strs[] = {
	"", " ", "\t\n", "42", "0", "007", "1234567890", "a", "A", "_", "z9_",
	"var_name", "=", "==", ">", ">=", ">>", ">>>", ">>=", "<", "<=",
	"<<", "<<=", "|", "||", "|=", "&", "&&", "&=", "!", "!=", "(", ")",
	"*", "*=", "/", "/=", "%", "%=", "+", "+=", "++", "-", "-=", "--",
	"~", "^", "^=", "?", ":", "@", "#", "\200", "\xff", "a+1", "1+a",
	"x==y", "a>=b", "c<<d", "e>>f", "g|=h", "i&=j", "k*=l", "m/=n",
	"o%=p", "q+=r", "s-=t", "u^=v", "!x", "(1)", "  99  ", "\n\t42\n",
	"a++b", "a--b", "++", "--", "===", ">>>=", "====", "0xffffffff",
	"-42", "0x10", "var1 var2", "___", "Z9", "a\200b", "\200a", "a|b",
	"a||b", "a&b", "a&&b", "a=b", "a==b", "a<b", "a<=b", "a>b", "a>=b",
	"a<<b", "a<<=b", "a>>b", "a>>=b", "a*b", "a*=b", "a/b", "a/=b",
	"a%b", "a%=b", "a+b", "a+=b", "a-b", "a-=b", "a^b", "a^=b",
	"a?b:c", "~a", "!a", "!=a", "(a)", ")a", "a)",
};

void run(const char *input) {
	char in_copy[256];
	size_t in_len = strlen(input);
	if (in_len >= sizeof(in_copy)) in_len = sizeof(in_copy)-1;
	memcpy(in_copy, input, in_len);
	in_copy[in_len] = '\0';
	port::yylval.name = nullptr;
	yylval.name = nullptr;
	const char *cursor = in_copy;
	for (;;) {
		port::arith_buf = cursor;
		arith_buf = cursor;
		int ta = port::yylex();
		arith_buf = cursor;
		int tb = ref_yylex();
		if (ta != tb) exit(2);
		if (ta == 0) break;
		cursor = port::arith_buf;
	}
	free(port::yylval.name);
	free(yylval.name);
}

int main(int argc, char **argv) {
	if (argc > 1) {
		run(argv[1]);
		return 0;
	}
	for (size_t i = 0; i < sizeof(yy_strs)/sizeof(yy_strs[0]); i++) {
		printf("case %zu\n", i);
		run(yy_strs[i]);
	}
	printf("ok\n");
	return 0;
}
