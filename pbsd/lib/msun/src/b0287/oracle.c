/*
 * oracle.c -- reference implementation for PBSD batch b0287.
 *
 * The original HardenedBSD sources from lib/msun/src are concatenated below.
 * Every exported function has been renamed with a "ref_" prefix; the function
 * bodies are otherwise byte-for-byte unmodified.  Supporting material needed
 * by the originals is reproduced verbatim.
 *
 * This file is the specification.  Do not modify any function body.
 */

#include <endian.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

/* ---- lib/msun/src/math_private.h (excerpt) ----------------------------- */

typedef union
{
  double value;
  struct
  {
#if __BYTE_ORDER == __BIG_ENDIAN
    u_int32_t msw;
    u_int32_t lsw;
#else
    u_int32_t lsw;
    u_int32_t msw;
#endif
  } parts;
} ieee_double_shape_type;

#define EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
  ieee_double_shape_type ew_u;					\
  ew_u.value = (d);						\
  (ix0) = ew_u.parts.msw;					\
  (ix1) = ew_u.parts.lsw;					\
} while (0)

#define GET_HIGH_WORD(i,d)					\
do {								\
  ieee_double_shape_type gh_u;					\
  gh_u.value = (d);						\
  (i) = gh_u.parts.msw;						\
} while (0)

#define SET_HIGH_WORD(d,v)					\
do {								\
  ieee_double_shape_type sh_u;					\
  sh_u.value = (d);						\
  sh_u.parts.msw = (v);						\
  (d) = sh_u.value;						\
} while (0)

#define SET_LOW_WORD(d,v)					\
do {								\
  ieee_double_shape_type sl_u;					\
  sl_u.value = (d);						\
  sl_u.parts.lsw = (v);						\
  (d) = sl_u.value;						\
} while (0)

#define INSERT_WORDS(d,ix0,ix1)					\
do {								\
  ieee_double_shape_type iw_u;					\
  iw_u.parts.msw = (ix0);					\
  iw_u.parts.lsw = (ix1);					\
  (d) = iw_u.value;						\
} while (0)

#define	nan_mix(x, y)		(nan_mix_op((x), (y), +))
#define	nan_mix_op(x, y, op)	(((x) + 0.0L) op ((y) + 0))

static inline int32_t
subnormal_ilogb(int32_t hi, int32_t lo)
{
	int32_t j;
	uint32_t i;

	j = -1022;
	if (hi == 0) {
	    j -= 21;
	    i = (uint32_t)lo;
	} else
	    i = (uint32_t)hi << 11;

	for (; i < 0x7fffffff; i <<= 1) j -= 1;

	return (j);
}

/* ---- lib/msun/src/k_log.h ---------------------------------------------- */

static const double
Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

static inline double
k_log1p(double f)
{
	double hfsq,s,z,R,w,t1,t2;

 	s = f/(2.0+f);
	z = s*s;
	w = z*z;
	t1= w*(Lg2+w*(Lg4+w*Lg6));
	t2= z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7)));
	R = t2+t1;
	hfsq=0.5*f*f;
	return s*(hfsq+R);
}

/* ---- lib/libc/amd64/_fpmath.h ------------------------------------------ */

union IEEEl2bits {
	long double	e;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:32;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
		unsigned int	junkl	:16;
		unsigned int	junkh	:32;
	} bits;
	struct {
		unsigned long	man	:64;
		unsigned int	expsign	:16;
		unsigned long	junk	:48;
	} xbits;
};

#define	LDBL_NBIT	0x80000000
#define	LDBL_MANH_SIZE	32
#define	LDBL_MANL_SIZE	32

/* ======================================================================== */
/* lib/msun/src/s_remquo.c                                                  */
/* ======================================================================== */

/*-
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */



static const double Zero_remquo[] = {0.0, -0.0,};

/*
 * Return the IEEE remainder and set *quo to the last n bits of the
 * quotient, rounded to the nearest integer.  We choose n=31 because
 * we wind up computing all the integer bits of the quotient anyway as
 * a side-effect of computing the remainder by the shift and subtract
 * method.  In practice, this is far more bits than are needed to use
 * remquo in reduction algorithms.
 */
double
ref_remquo(double x, double y, int *quo)
{
	int32_t hx,hy,hz,ix,iy,n,sx;
	u_int32_t lx,ly,lz,q,sxy;

	EXTRACT_WORDS(hx,lx,x);
	EXTRACT_WORDS(hy,ly,y);
	sxy = (hx ^ hy) & 0x80000000;
	sx = hx&0x80000000;		/* sign of x */
	hx ^=sx;		/* |x| */
	hy &= 0x7fffffff;	/* |y| */

    /* purge off exception values */
	if((hy|ly)==0||(hx>=0x7ff00000)||	/* y=0,or x not finite */
	  ((hy|((ly|-ly)>>31))>0x7ff00000))	/* or y is NaN */
	    return nan_mix_op(x, y, *)/nan_mix_op(x, y, *);
	if(hx<=hy) {
	    if((hx<hy)||(lx<ly)) {
		q = 0;
		goto fixup;	/* |x|<|y| return x or x-y */
	    }
	    if(lx==ly) {
		*quo = (sxy ? -1 : 1);
		return Zero_remquo[(u_int32_t)sx>>31];	/* |x|=|y| return x*0*/
	    }
	}

    /* determine ix = ilogb(x) */
	if(hx<0x00100000)
	    ix = subnormal_ilogb(hx, lx);
	else
	    ix = (hx>>20)-1023;

    /* determine iy = ilogb(y) */
	if(hy<0x00100000)
	    iy = subnormal_ilogb(hy, ly);
	else
	    iy = (hy>>20)-1023;

    /* set up {hx,lx}, {hy,ly} and align y to x */
	if(ix >= -1022)
	    hx = 0x00100000|(0x000fffff&hx);
	else {		/* subnormal x, shift x to normal */
	    n = -1022-ix;
	    if(n<=31) {
	        hx = (hx<<n)|(lx>>(32-n));
	        lx <<= n;
	    } else {
		hx = lx<<(n-32);
		lx = 0;
	    }
	}
	if(iy >= -1022)
	    hy = 0x00100000|(0x000fffff&hy);
	else {		/* subnormal y, shift y to normal */
	    n = -1022-iy;
	    if(n<=31) {
	        hy = (hy<<n)|(ly>>(32-n));
	        ly <<= n;
	    } else {
		hy = ly<<(n-32);
		ly = 0;
	    }
	}

    /* fix point fmod */
	n = ix - iy;
	q = 0;
	while(n--) {
	    hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	    if(hz<0){hx = hx+hx+(lx>>31); lx = lx+lx;}
	    else {hx = hz+hz+(lz>>31); lx = lz+lz; q++;}
	    q <<= 1;
	}
	hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	if(hz>=0) {hx=hz;lx=lz;q++;}

    /* convert back to floating value and restore the sign */
	if((hx|lx)==0) {			/* return sign(x)*0 */
	    q &= 0x7fffffff;
	    *quo = (sxy ? -q : q);
	    return Zero_remquo[(u_int32_t)sx>>31];
	}
	while(hx<0x00100000) {		/* normalize x */
	    hx = hx+hx+(lx>>31); lx = lx+lx;
	    iy -= 1;
	}
	if(iy>= -1022) {	/* normalize output */
	    hx = ((hx-0x00100000)|((iy+1023)<<20));
	} else {		/* subnormal output */
	    n = -1022 - iy;
	    if(n<=20) {
		lx = (lx>>n)|((u_int32_t)hx<<(32-n));
		hx >>= n;
	    } else if (n<=31) {
		lx = (hx<<(32-n))|(lx>>n); hx = 0;
	    } else {
		lx = hx>>(n-32); hx = 0;
	    }
	}
fixup:
	INSERT_WORDS(x,hx,lx);
	y = fabs(y);
	if (y < 0x1p-1021) {
	    if (x+x>y || (x+x==y && (q & 1))) {
		q++;
		x-=y;
	    }
	} else if (x>0.5*y || (x==0.5*y && (q & 1))) {
	    q++;
	    x-=y;
	}
	GET_HIGH_WORD(hx,x);
	SET_HIGH_WORD(x,hx^sx);
	q &= 0x7fffffff;
	*quo = (sxy ? -q : q);
	return x;
}

/* ======================================================================== */
/* lib/msun/src/e_log2.c                                                    */
/* ======================================================================== */


/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/*
 * Return the base 2 logarithm of x.  See e_log.c and k_log.h for most
 * comments.
 *
 * This reduces x to {k, 1+f} exactly as in e_log.c, then calls the kernel,
 * then does the combining and scaling steps
 *    log2(x) = (f - 0.5*f*f + k_log1p(f)) / ln2 + k
 * in not-quite-routine extra precision.
 */



static const double
two54      =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
ivln2hi    =  1.44269504072144627571e+00, /* 0x3ff71547, 0x65200000 */
ivln2lo    =  1.67517131648865118353e-10; /* 0x3de705fc, 0x2eefa200 */

static const double zero_log2   =  0.0;
static volatile double vzero_log2 = 0.0;

double
ref_log2(double x)
{
	double f,hfsq,hi,lo,r,val_hi,val_lo,w,y;
	int32_t i,k,hx;
	u_int32_t lx;

	EXTRACT_WORDS(hx,lx,x);

	k=0;
	if (hx < 0x00100000) {			/* x < 2**-1022  */
	    if (((hx&0x7fffffff)|lx)==0)
		return -two54/vzero_log2;		/* log(+-0)=-inf */
	    if (hx<0) return (x-x)/zero_log2;	/* log(-#) = NaN */
	    k -= 54; x *= two54; /* subnormal number, scale up x */
	    GET_HIGH_WORD(hx,x);
	}
	if (hx >= 0x7ff00000) return x+x;
	if (hx == 0x3ff00000 && lx == 0)
	    return zero_log2;			/* log(1) = +0 */
	k += (hx>>20)-1023;
	hx &= 0x000fffff;
	i = (hx+0x95f64)&0x100000;
	SET_HIGH_WORD(x,hx|(i^0x3ff00000));	/* normalize x or x/2 */
	k += (i>>20);
	y = (double)k;
	f = x - 1.0;
	hfsq = 0.5*f*f;
	r = k_log1p(f);

	/*
	 * f-hfsq must (for args near 1) be evaluated in extra precision
	 * to avoid a large cancellation when x is near sqrt(2) or 1/sqrt(2).
	 * This is fairly efficient since f-hfsq only depends on f, so can
	 * be evaluated in parallel with R.  Not combining hfsq with R also
	 * keeps R small (though not as small as a true `lo' term would be),
	 * so that extra precision is not needed for terms involving R.
	 *
	 * Compiler bugs involving extra precision used to break Dekker's
	 * theorem for spitting f-hfsq as hi+lo, unless double_t was used
	 * or the multi-precision calculations were avoided when double_t
	 * has extra precision.  These problems are now automatically
	 * avoided as a side effect of the optimization of combining the
	 * Dekker splitting step with the clear-low-bits step.
	 *
	 * y must (for args near sqrt(2) and 1/sqrt(2)) be added in extra
	 * precision to avoid a very large cancellation when x is very near
	 * these values.  Unlike the above cancellations, this problem is
	 * specific to base 2.  It is strange that adding +-1 is so much
	 * harder than adding +-ln2 or +-log10_2.
	 *
	 * This uses Dekker's theorem to normalize y+val_hi, so the
	 * compiler bugs are back in some configurations, sigh.  And I
	 * don't want to used double_t to avoid them, since that gives a
	 * pessimization and the support for avoiding the pessimization
	 * is not yet available.
	 *
	 * The multi-precision calculations for the multiplications are
	 * routine.
	 */
	hi = f - hfsq;
	SET_LOW_WORD(hi,0);
	lo = (f - hi) - hfsq + r;
	val_hi = hi*ivln2hi;
	val_lo = (lo+hi)*ivln2lo + lo*ivln2hi;

	/* spadd(val_hi, val_lo, y), except for not using double_t: */
	w = y + val_hi;
	val_lo += (y - w) + val_hi;
	val_hi = w;

	return val_lo + val_hi;
}

/* ======================================================================== */
/* lib/msun/src/e_atan2.c                                                   */
/* ======================================================================== */


/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

/* atan2(y,x)
 * Method :
 *	1. Reduce y to positive by atan2(y,x)=-atan2(-y,x).
 *	2. Reduce x to positive by (if x and y are unexceptional): 
 *		ARG (x+iy) = arctan(y/x)   	   ... if x > 0,
 *		ARG (x+iy) = pi - arctan[y/(-x)]   ... if x < 0,
 *
 * Special cases:
 *
 *	ATAN2((anything), NaN ) is NaN;
 *	ATAN2(NAN , (anything) ) is NaN;
 *	ATAN2(+-0, +(anything but NaN)) is +-0  ;
 *	ATAN2(+-0, -(anything but NaN)) is +-pi ;
 *	ATAN2(+-(anything but 0 and NaN), 0) is +-pi/2;
 *	ATAN2(+-(anything but INF and NaN), +INF) is +-0 ;
 *	ATAN2(+-(anything but INF and NaN), -INF) is +-pi;
 *	ATAN2(+-INF,+INF ) is +-pi/4 ;
 *	ATAN2(+-INF,-INF ) is +-3pi/4;
 *	ATAN2(+-INF, (anything but,0,NaN, and INF)) is +-pi/2;
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following 
 * constants. The decimal values may be used, provided that the 
 * compiler will convert from decimal to binary accurately enough 
 * to produce the hexadecimal values shown.
 */



static volatile double
tiny_atan2  = 1.0e-300;
static const double
zero_atan2  = 0.0,
pi_o_4  = 7.8539816339744827900E-01, /* 0x3FE921FB, 0x54442D18 */
pi_o_2  = 1.5707963267948965580E+00, /* 0x3FF921FB, 0x54442D18 */
pi_atan2      = 3.1415926535897931160E+00; /* 0x400921FB, 0x54442D18 */
static volatile double
pi_lo_atan2   = 1.2246467991473531772E-16; /* 0x3CA1A626, 0x33145C07 */

double
ref_atan2(double y, double x)
{
	double z;
	int32_t k,m,hx,hy,ix,iy;
	u_int32_t lx,ly;

	EXTRACT_WORDS(hx,lx,x);
	ix = hx&0x7fffffff;
	EXTRACT_WORDS(hy,ly,y);
	iy = hy&0x7fffffff;
	if(((ix|((lx|-lx)>>31))>0x7ff00000)||
	   ((iy|((ly|-ly)>>31))>0x7ff00000))	/* x or y is NaN */
	    return nan_mix(x, y);
	if(hx==0x3ff00000&&lx==0) return atan(y);   /* x=1.0 */
	m = ((hy>>31)&1)|((hx>>30)&2);	/* 2*sign(x)+sign(y) */

    /* when y = 0 */
	if((iy|ly)==0) {
	    switch(m) {
		case 0: 
		case 1: return y; 	/* atan(+-0,+anything)=+-0 */
		case 2: return  pi_atan2_atan2+tiny_atan2;/* atan(+0,-anything) = pi */
		case 3: return -pi_atan2-tiny_atan2;/* atan(-0,-anything) =-pi */
	    }
	}
    /* when x = 0 */
	if((ix|lx)==0) return (hy<0)?  -pi_o_2-tiny_atan2: pi_o_2+tiny_atan2;
	    
    /* when x is INF */
	if(ix==0x7ff00000) {
	    if(iy==0x7ff00000) {
		switch(m) {
		    case 0: return  pi_atan2_o_4+tiny_atan2;/* atan(+INF,+INF) */
		    case 1: return -pi_atan2_o_4-tiny_atan2;/* atan(-INF,+INF) */
		    case 2: return  3.0*pi_o_4+tiny_atan2;/*atan(+INF,-INF)*/
		    case 3: return -3.0*pi_o_4-tiny_atan2;/*atan(-INF,-INF)*/
		}
	    } else {
		switch(m) {
		    case 0: return  zero_atan2  ;	/* atan(+...,+INF) */
		    case 1: return -zero_atan2  ;	/* atan(-...,+INF) */
		    case 2: return  pi_atan2_atan2+tiny_atan2  ;	/* atan(+...,-INF) */
		    case 3: return -pi_atan2-tiny_atan2  ;	/* atan(-...,-INF) */
		}
	    }
	}
    /* when y is INF */
	if(iy==0x7ff00000) return (hy<0)? -pi_o_2-tiny_atan2: pi_o_2+tiny_atan2;

    /* compute y/x */
	k = (iy-ix)>>20;
	if(k > 60) {		 	/* |y/x| >  2**60 */
	    z=pi_o_2+0.5*pi_lo_atan2;
	    m&=1;
	}
	else if(hx<0&&k<-60) z=0.0; 	/* 0 > |y|/x > -2**-60 */
	else z=atan(fabs(y/x));		/* safe to do y/x */
	switch (m) {
	    case 0: return       z  ;	/* atan(+,+) */
	    case 1: return      -z  ;	/* atan(-,+) */
	    case 2: return  pi_atan2_atan2-(z-pi_lo_atan2);/* atan(+,-) */
	    default: /* case 3 */
	    	    return  (z-pi_lo_atan2)-pi_atan2;/* atan(-,-) */
	}
}

/* ======================================================================== */
/* lib/msun/src/e_fmodl.c                                                   */
/* ======================================================================== */

/*-
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */



#define	BIAS (LDBL_MAX_EXP - 1)

#if LDBL_MANL_SIZE > 32
typedef	uint64_t manl_t;
#else
typedef	uint32_t manl_t;
#endif

#if LDBL_MANH_SIZE > 32
typedef	uint64_t manh_t;
#else
typedef	uint32_t manh_t;
#endif

/*
 * These macros add and remove an explicit integer bit in front of the
 * fractional mantissa, if the architecture doesn't have such a bit by
 * default already.
 */
#ifdef LDBL_IMPLICIT_NBIT
#define	SET_NBIT(hx)	((hx) | (1ULL << LDBL_MANH_SIZE))
#define	HFRAC_BITS	LDBL_MANH_SIZE
#else
#define	SET_NBIT(hx)	(hx)
#define	HFRAC_BITS	(LDBL_MANH_SIZE - 1)
#endif

#define	MANL_SHIFT	(LDBL_MANL_SIZE - 1)

static const long double one_fmodl = 1.0, Zero_fmodl[] = {0.0, -0.0,};

/*
 * fmodl(x,y)
 * Return x mod y in exact arithmetic
 * Method: shift and subtract
 *
 * Assumptions:
 * - The low part of the mantissa fits in a manl_t exactly.
 * - The high part of the mantissa fits in an int64_t with enough room
 *   for an explicit integer bit in front of the fractional bits.
 */
long double
ref_fmodl(long double x, long double y)
{
	union IEEEl2bits ux, uy;
	int64_t hx,hz;	/* We need a carry bit even if LDBL_MANH_SIZE is 32. */
	manh_t hy;
	manl_t lx,ly,lz;
	int ix,iy,n,sx;

	ux.e = x;
	uy.e = y;
	sx = ux.bits.sign;

    /* purge off exception values */
	if((uy.bits.exp|uy.bits.manh|uy.bits.manl)==0 || /* y=0 */
	   (ux.bits.exp == BIAS + LDBL_MAX_EXP) ||	 /* or x not finite */
	   (uy.bits.exp == BIAS + LDBL_MAX_EXP &&
	    ((uy.bits.manh&~LDBL_NBIT)|uy.bits.manl)!=0)) /* or y is NaN */
	    return nan_mix_op(x, y, *)/nan_mix_op(x, y, *);
	if(ux.bits.exp<=uy.bits.exp) {
	    if((ux.bits.exp<uy.bits.exp) ||
	       (ux.bits.manh<=uy.bits.manh &&
		(ux.bits.manh<uy.bits.manh ||
		 ux.bits.manl<uy.bits.manl))) {
		return x;		/* |x|<|y| return x or x-y */
	    }
	    if(ux.bits.manh==uy.bits.manh && ux.bits.manl==uy.bits.manl) {
		return Zero_fmodl[sx];	/* |x|=|y| return x*0*/
	    }
	}

    /* determine ix = ilogb(x) */
	if(ux.bits.exp == 0) {	/* subnormal x */
	    ux.e *= 0x1.0p512;
	    ix = ux.bits.exp - (BIAS + 512);
	} else {
	    ix = ux.bits.exp - BIAS;
	}

    /* determine iy = ilogb(y) */
	if(uy.bits.exp == 0) {	/* subnormal y */
	    uy.e *= 0x1.0p512;
	    iy = uy.bits.exp - (BIAS + 512);
	} else {
	    iy = uy.bits.exp - BIAS;
	}

    /* set up {hx,lx}, {hy,ly} and align y to x */
	hx = SET_NBIT(ux.bits.manh);
	hy = SET_NBIT(uy.bits.manh);
	lx = ux.bits.manl;
	ly = uy.bits.manl;

    /* fix point fmod */
	n = ix - iy;

	while(n--) {
	    hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	    if(hz<0){hx = hx+hx+(lx>>MANL_SHIFT); lx = lx+lx;}
	    else {
		if ((hz|lz)==0)		/* return sign(x)*0 */
		    return Zero_fmodl[sx];
		hx = hz+hz+(lz>>MANL_SHIFT); lx = lz+lz;
	    }
	}
	hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	if(hz>=0) {hx=hz;lx=lz;}

    /* convert back to floating value and restore the sign */
	if((hx|lx)==0)			/* return sign(x)*0 */
	    return Zero_fmodl[sx];
	while(hx<(1ULL<<HFRAC_BITS)) {	/* normalize x */
	    hx = hx+hx+(lx>>MANL_SHIFT); lx = lx+lx;
	    iy -= 1;
	}
	ux.bits.manh = hx; /* The mantissa is truncated here if needed. */
	ux.bits.manl = lx;
	if (iy < LDBL_MIN_EXP) {
	    ux.bits.exp = iy + (BIAS + 512);
	    ux.e *= 0x1p-512;
	} else {
	    ux.bits.exp = iy + BIAS;
	}
	x = ux.e * one_fmodl_fmodl;		/* create necessary signal */
	return x;		/* exact output */
}