#ifndef GDTOA_B0066_SHIM_H
#define GDTOA_B0066_SHIM_H
#define GDTOAIMP_H_INCLUDED
#define IEEE_8087 1
#define Long int
typedef unsigned int ULong;
typedef unsigned short UShort;
#define Void void
#define ANSI(x) x
#define CONST const
#define MULTIPLE_THREADS 1
#define MALLOC malloc
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <fenv.h>
extern int __isthreaded;
extern pthread_mutex_t __gdtoa_locks[2];
#define _pthread_mutex_lock pthread_mutex_lock
#define _pthread_mutex_unlock pthread_mutex_unlock
#define ACQUIRE_DTOA_LOCK(n) do { if (__isthreaded) pthread_mutex_lock(&__gdtoa_locks[n]); } while(0)
#define FREE_DTOA_LOCK(n) do { if (__isthreaded) pthread_mutex_unlock(&__gdtoa_locks[n]); } while(0)
#define dtoa __dtoa
#define gdtoa __gdtoa
#define freedtoa __freedtoa
#define nrv_alloc __nrv_alloc_D2A
#define rv_alloc __rv_alloc_D2A
#define Balloc __Balloc_D2A
#define Bfree __Bfree_D2A
#define ULtoQ __ULtoQ_D2A
#define ULtof __ULtof_D2A
#define ULtod __ULtod_D2A
#define ULtodd __ULtodd_D2A
#define ULtox __ULtox_D2A
#define ULtoxL __ULtoxL_D2A
#define any_on __any_on_D2A
#define b2d __b2d_D2A
#define cmp __cmp_D2A
#define copybits __copybits_D2A
#define d2b __d2b_D2A
#define decrement __decrement_D2A
#define diff __diff_D2A
#define dtoa_result __dtoa_result_D2A
#define gethex __gethex_D2A
#define hexdig __hexdig_D2A
#define hexdig_init_D2A __hexdig_init_D2A
#define hexnan __hexnan_D2A
#define hi0bits hi0bits_D2A
#define hi0bits_D2A __hi0bits_D2A
#define i2b __i2b_D2A
#define increment increment_D2A
#define increment_D2A __increment_D2A
#define lo0bits __lo0bits_D2A
#define lshift __lshift_D2A
#define match __match_D2A
#define mult __mult_D2A
#define multadd __multadd_D2A
#define pow5mult __pow5mult_D2A
#define quorem __quorem_D2A
#define ratio __ratio_D2A
#define rshift __rshift_D2A
#define s2b __s2b_D2A
#define set_ones __set_ones_D2A
#define strcp __strcp_D2A
#define sum __sum_D2A
#define tens __tens_D2A
#define tinytens __tinytens_D2A
#define trailz __trailz_D2A
#define ulp __ulp_D2A
#define d_QNAN0 0x0
#define d_QNAN1 0x7ff80000
#define NAN_WORD0 d_QNAN1
#define NAN_WORD1 d_QNAN0
#define _0 1
#define _1 0
#define INFNAN_CHECK
#define SI 0
#define Pack_32
#define ULbits 32
#define kshift 5
#define kmask 31
#define ALL_ON 0xffffffff
#define Kmax 9
#define Exp_shift 20
#define Exp_shift1 20
#define Exp_msk1 0x100000
#define Exp_msk11 0x100000
#define Exp_mask 0x7ff00000
#define P 53
#define Bias 1023
#define Emin (-1022)
#define Exp_1 0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask 0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask 0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14
#define Flt_Rounds FLT_ROUNDS
#define IEEE_Arith
#define Scale_Bit 0x10
#define n_bigtens 5
#define word0(x) (x)->L[1]
#define word1(x) (x)->L[0]
#define dval(x) (x)->d
#define Storeinc(a,b,c) (((unsigned short *)a)[1] = (unsigned short)b, ((unsigned short *)a)[0] = (unsigned short)c, a++)
#define rounded_product(a,b) a *= b
#define rounded_quotient(a,b) a /= b
#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 0xffffffff
#define Bcopy(x,y) memcpy(&x->sign,&y->sign,y->wds*sizeof(ULong) + 2*sizeof(int))
typedef union { double d; ULong L[2]; } U;
enum { STRTOG_Zero=0, STRTOG_Normal=1, STRTOG_Denormal=2, STRTOG_Infinite=3, STRTOG_NaN=4, STRTOG_NaNbits=5, STRTOG_NoNumber=6, STRTOG_Retmask=7, STRTOG_Neg=0x08, STRTOG_Inexlo=0x10, STRTOG_Inexhi=0x20, STRTOG_Inexact=0x30, STRTOG_Underflow=0x40, STRTOG_Overflow=0x80 };
typedef struct { int nbits, emin, emax, rounding, sudden_underflow; } FPI;
enum { FPI_Round_zero=0, FPI_Round_near=1, FPI_Round_up=2, FPI_Round_down=3 };
struct Bigint { struct Bigint *next; int k, maxwds, sign, wds; ULong x[1]; };
typedef struct Bigint Bigint;
extern char *dtoa_result;
extern const double bigtens[], tens[], tinytens[];
extern unsigned char hexdig[];
extern Bigint *Balloc(int);
extern void Bfree(Bigint*);
extern char *dtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve);
extern char *gdtoa(FPI *fpi, int be, ULong *bits, int *kindp, int mode, int ndigits, int *decpt, char **rve);
extern void freedtoa(char*);
extern char *nrv_alloc(char*, char **, int);
extern char *rv_alloc(int);
extern int cmp(Bigint*, Bigint*);
extern Bigint *diff(Bigint*, Bigint*);
extern Bigint *i2b(int);
extern Bigint *increment(Bigint*);
extern void decrement(Bigint*);
extern int lo0bits(ULong*);
extern Bigint *lshift(Bigint*, int);
extern Bigint *mult(Bigint*, Bigint*);
extern Bigint *multadd(Bigint*, int, int);
extern Bigint *pow5mult(Bigint*, int);
extern int quorem(Bigint*, Bigint*);
extern double ratio(Bigint*, Bigint*);
extern void rshift(Bigint*, int);
extern Bigint *s2b(const char*, int, int, ULong, int);
extern Bigint *sum(Bigint*, Bigint*);
extern int trailz(Bigint*);
extern double ulp(U*);
extern double b2d(Bigint*, int*);
extern Bigint *d2b(double, int*, int*);
extern void copybits(ULong*, int, Bigint*);
extern ULong any_on(Bigint*, int);
extern int hi0bits(ULong);
extern Bigint *set_ones(Bigint*, int);
extern int gethex(const char**, FPI*, Long*, Bigint**, int);
extern int hexnan(const char**, FPI*, ULong*);
extern void hexdig_init_D2A(void);
extern char *strcp(char*, const char*);
extern int match(const char**, char*);
#endif
