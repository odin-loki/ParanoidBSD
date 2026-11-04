module;

#include <cfloat>
#include <cmath>
#include <cstdint>

export module pbsd.lib.msun.src.b0306;

namespace pbsd::lib_msun_src::b0306::detail {

typedef union {
	float value;
	unsigned int word;
} ieee_float_shape_type;

#define GET_FLOAT_WORD(i, d)					\
	do {							\
		ieee_float_shape_type gf_u;			\
		gf_u.value = (d);				\
		(i) = gf_u.word;				\
	} while (0)

#if LDBL_MANT_DIG == 64
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
#define	LDBL_NBIT	0x80000000u
#define	LDBL_MANH_SIZE	32
#elif LDBL_MANT_DIG == 113
union IEEEl2bits {
	long double	e;
	struct {
		unsigned long	manl	:64;
		unsigned long	manh	:48;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
	} bits;
	struct {
		unsigned long	manl	:64;
		unsigned long	manh	:48;
		unsigned int	expsign	:16;
	} xbits;
};
#define	LDBL_NBIT	0x8000000000000000ull
#define	LDBL_MANH_SIZE	49
#else
#error "Unsupported long double format"
#endif

#define	BIAS		(LDBL_MAX_EXP - 1)
#define	MANH_SIZE	LDBL_MANH_SIZE
#define	ASIN_LINEAR	(LDBL_MANT_DIG == 64 ? (BIAS - 32) : (BIAS - 56))
#define	THRESH		((0xe666666666666666ULL>>(64-(MANH_SIZE-1)))|LDBL_NBIT)

#if LDBL_MANT_DIG == 64
static const long double
pS0 =  1.66666666666666666631e-01L,
pS1 = -4.16313987993683104320e-01L,
pS2 =  3.69068046323246813704e-01L,
pS3 = -1.36213932016738603108e-01L,
pS4 =  1.78324189708471965733e-02L,
pS5 = -2.19216428382605211588e-04L,
pS6 = -7.10526623669075243183e-06L,
qS1 = -2.94788392796209867269e+00L,
qS2 =  3.27309890266528636716e+00L,
qS3 = -1.68285799854822427013e+00L,
qS4 =  3.90699412641738801874e-01L,
qS5 = -3.14365703596053263322e-02L;

static const long double atanhi[] = {
	 4.63647609000806116202e-01L,
	 7.85398163397448309628e-01L,
	 9.82793723247329067960e-01L,
	 1.57079632679489661926e+00L,
};

static const long double atanlo[] = {
	 1.18469937025062860669e-20L,
	-1.25413940316708300586e-20L,
	 2.55232234165405176172e-20L,
	-2.50827880633416601173e-20L,
};

static inline long double
P(long double x)
{
	return (x * (pS0 + x * (pS1 + x * (pS2 + x * (pS3 + x * \
		(pS4 + x * (pS5 + x * pS6)))))));
}

static inline long double
Q(long double x)
{
	return (1.0 + x * (qS1 + x * (qS2 + x * (qS3 + x * (qS4 + x * qS5)))));
}
#elif LDBL_MANT_DIG == 113
static const long double
pS0 =  1.66666666666666666666666666666700314e-01L,
pS1 = -7.32816946414566252574527475428622708e-01L,
pS2 =  1.34215708714992334609030036562143589e+00L,
pS3 = -1.32483151677116409805070261790752040e+00L,
pS4 =  7.61206183613632558824485341162121989e-01L,
pS5 = -2.56165783329023486777386833928147375e-01L,
pS6 =  4.80718586374448793411019434585413855e-02L,
pS7 = -4.42523267167024279410230886239774718e-03L,
pS8 =  1.44551535183911458253205638280410064e-04L,
pS9 = -2.10558957916600254061591040482706179e-07L,
qS1 = -4.84690167848739751544716485245697428e+00L,
qS2 =  9.96619113536172610135016921140206980e+00L,
qS3 = -1.13177895428973036660836798461641458e+01L,
qS4 =  7.74004374389488266169304117714658761e+00L,
qS5 = -3.25871986053534084709023539900339905e+00L,
qS6 =  8.27830318881232209752469022352928864e-01L,
qS7 = -1.18768052702942805423330715206348004e-01L,
qS8 =  8.32600764660522313269101537926539470e-03L,
qS9 = -1.99407384882605586705979504567947007e-04L;

static const long double atanhi[] = {
	 4.63647609000806116214256231461214397e-01L,
	 7.85398163397448309615660845819875699e-01L,
	 9.82793723247329067985710611014666038e-01L,
	 1.57079632679489661923132169163975140e+00L,
};

static const long double atanlo[] = {
	 4.89509642257333492668618435220297706e-36L,
	 2.16795253253094525619926100651083806e-35L,
	-2.31288434538183565909319952098066272e-35L,
	 4.33590506506189051239852201302167613e-35L,
};

static inline long double
P(long double x)
{
	return (x * (pS0 + x * (pS1 + x * (pS2 + x * (pS3 + x * \
		(pS4 + x * (pS5 + x * (pS6 + x * (pS7 + x * (pS8 + x * \
		pS9))))))))));
}

static inline long double
Q(long double x)
{
	return (1.0 + x * (qS1 + x * (qS2 + x * (qS3 + x * (qS4 + x * \
		(qS5 + x * (qS6 + x * (qS7 + x * (qS8 + x * qS9)))))))));
}
#endif

#define	pio2_hi	atanhi[3]
#define	pio2_lo	atanlo[3]
#define	pio4_hi	atanhi[1]

} // namespace pbsd::lib_msun_src::b0306::detail

export namespace pbsd::lib_msun_src::b0306 {

using std::int16_t;
using std::int32_t;

/* FreeBSD: head/lib/msun/src/e_asin.c 176451 2008-02-22 02:30:36Z das */
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
 * See comments in e_asin.c.
 * Converted to long double by David Schultz <das@FreeBSD.ORG>.
 */

namespace {

static const long double
one =  1.00000000000000000000e+00,
huge = 1.000e+300;

} // namespace

export long double
asinl(long double x)
{
	detail::IEEEl2bits u;
	long double t=0.0,w,p,q,c,r,s;
	int16_t expsign, expt;
	u.e = x;
	expsign = u.xbits.expsign;
	expt = expsign & 0x7fff;
	if(expt >= detail::BIAS) {		/* |x|>= 1 */
		if(expt==detail::BIAS && ((u.bits.manh&~detail::LDBL_NBIT)|u.bits.manl)==0)
		    /* asin(1)=+-pi/2 with inexact */
		    return x*detail::pio2_hi+x*detail::pio2_lo;	
	    return (x-x)/(x-x);		/* asin(|x|>1) is NaN */   
	} else if (expt<detail::BIAS-1) {	/* |x|<0.5 */
	    if(expt<detail::ASIN_LINEAR) {	/* if |x| is small, asinl(x)=x */
		if(huge+x>one) return x;/* return x with inexact if x!=0*/
	    }
	    t = x*x;
	    p = detail::P(t);
	    q = detail::Q(t);
	    w = p/q;
	    return x+x*w;
	}
	/* 1> |x|>= 0.5 */
	w = one-fabsl(x);
	t = w*0.5;
	p = detail::P(t);
	q = detail::Q(t);
	s = sqrtl(t);
	if(u.bits.manh>=detail::THRESH) { 	/* if |x| is close to 1 */
	    w = p/q;
	    t = detail::pio2_hi-(2.0*(s+s*w)-detail::pio2_lo);
	} else {
	    u.e = s;
	    u.bits.manl = 0;
	    w = u.e;
	    c  = (t-w*w)/(s+w);
	    r  = p/q;
	    p  = 2.0*s*r-(detail::pio2_lo-2.0*c);
	    q  = detail::pio4_hi-2.0*w;
	    t  = detail::pio4_hi-(p-q);
	}    
	if(expsign>0) return t; else return -t;    
}

/* e_asinf.c -- float version of e_asin.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

namespace {

static const float
asinf_one =  1.0000000000e+00, /* 0x3F800000 */
asinf_huge =  1.000e+30;

/*
 * The coefficients for the rational approximation were generated over
 *  0x1p-12f <= x <= 0.5f.  The maximum error satisfies log2(e) < -30.084.
 */
static const float
pS0 =  1.66666672e-01f, /* 0x3e2aaaab */
pS1 = -1.19510300e-01f, /* 0xbdf4c1d1 */
pS2 =  5.47002675e-03f, /* 0x3bb33de9 */
qS1 = -1.16706085e+00f, /* 0xbf956240 */
qS2 =  2.90115148e-01f; /* 0x3e9489f9 */

static const double
pio2 =  1.570796326794896558e+00;

} // namespace

export float
asinf(float x)
{
	double s;
	float t,w,p,q;
	int32_t hx,ix;
	detail::GET_FLOAT_WORD(hx,x);
	ix = hx&0x7fffffff;
	if(ix>=0x3f800000) {		/* |x| >= 1 */
	    if(ix==0x3f800000)		/* |x| == 1 */
		return x*pio2;		/* asin(+-1) = +-pi/2 with inexact */
	    return (x-x)/(x-x);		/* asin(|x|>1) is NaN */
	} else if (ix<0x3f000000) {	/* |x|<0.5 */
	    if(ix<0x39800000) {		/* |x| < 2**-12 */
		if(asinf_huge+x>asinf_one) return x;/* return x with inexact if x!=0*/
	    }
	    t = x*x;
	    p = t*(pS0+t*(pS1+t*pS2));
	    q = asinf_one+t*(qS1+t*qS2);
	    w = p/q;
	    return x+x*w;
	}
	/* 1> |x|>= 0.5 */
	w = asinf_one-fabsf(x);
	t = w*(float)0.5;
	p = t*(pS0+t*(pS1+t*pS2));
	q = asinf_one+t*(qS1+t*qS2);
	s = sqrt(t);
	w = p/q;
	t = pio2-2.0*(s+s*w);
	if(hx>0) return t; else return -t;
}

} // namespace pbsd::lib_msun_src::b0306
