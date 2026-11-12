#include "prelude.h"
#undef YYBYACC
#define YYBYACC 1
#undef YYMAJOR
#define YYMAJOR 2
#undef YYMINOR
#define YYMINOR 0
#undef YYCHECK
#define YYCHECK "yyyymmdd"
#undef YYEMPTY
#define YYEMPTY        (-1)
#undef yyclearin
#define yyclearin      (yychar = YYEMPTY)
#undef yyerrok
#define yyerrok        (yyerrflag = 0)
#undef YYRECOVERING
#define YYRECOVERING() (yyerrflag != 0)
#undef YYENOMEM
#define YYENOMEM       (-2)
#undef YYEOF
#define YYEOF          0
#undef YYBTYACC
#define YYBTYACC 0
#undef YYDEBUGSTR
#define YYDEBUGSTR YYPREFIX "debug"
#undef YYTABLESIZE
#define YYTABLESIZE 0
#undef YYFINAL
#define YYFINAL 2
#undef YYDEBUG
#define YYDEBUG 1
#undef YYMAXTOKEN
#define YYMAXTOKEN 256
#undef YYUNDFTOKEN
#define YYUNDFTOKEN 259
#undef YYTRANSLATE
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#undef YYLLOC_DEFAULT
#define YYLLOC_DEFAULT(loc, rhs, n)  do  {      if (n == 0)      {          (loc).first_line   = YYRHSLOC(rhs, 0).last_line;          (loc).first_column = YYRHSLOC(rhs, 0).last_column;          (loc).last_line    = YYRHSLOC(rhs, 0).last_line;          (loc).last_column  = YYRHSLOC(rhs, 0).last_column;      }      else      {          (loc).first_line   = YYRHSLOC(rhs, 1).first_line;          (loc).first_column = YYRHSLOC(rhs, 1).first_column;          (loc).last_line    = YYRHSLOC(rhs, n).last_line;          (loc).last_column  = YYRHSLOC(rhs, n).last_column;      }  } while (0)
#undef YYLVQUEUEGROWTH
#define YYLVQUEUEGROWTH 32
#undef YYINITSTACKSIZE
#define YYINITSTACKSIZE 200
#undef yydepth
#define yydepth (int)(yystack.s_mark - yystack.s_base)
#undef yytrial
#define yytrial (yyps->save)
#undef yyfreestack
#define yyfreestack(data) 
#undef YYABORT
#define YYABORT  goto yyabort
#undef YYREJECT
#define YYREJECT goto yyabort
#undef YYACCEPT
#define YYACCEPT goto yyaccept
#undef YYERROR
#define YYERROR  goto yyerrlab
#undef YYVALID
#define YYVALID        do { if (yyps->save)            goto yyvalid; } while(0)
#undef YYVALID_NESTED
#define YYVALID_NESTED do { if (yyps->save &&                                  yyps->save->save == 0) goto yyvalid; } while(0)
/*@ requires \true; ensures \result == -1; */
int
yylex(void)
{
    return -1;
}
