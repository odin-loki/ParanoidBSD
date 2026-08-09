module;

#include <complex.h>
#include <math.h>

export module pbsd.lib.msun.src.b0312;

namespace pbsd::lib_msun_src::b0312 {

/*-
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

export float _Complex
cpowf(float _Complex a, float _Complex z)
{
	float _Complex w;
	float x, y, r, theta, absa, arga;

	x = crealf(z);
	y = cimagf(z);
	absa = cabsf (a);
	if (absa == 0.0f) {
		if (x == 0 && y == 0)
		    return (CMPLXF(1.f, 0.f));
		else
		    return (CMPLXF(0.f, 0.f));
	}
	arga = cargf (a);
	r = powf (absa, x);
	theta = x * arga;
	if (y != 0.0f) {
		r = r * expf (-y * arga);
		theta = theta + y * logf (absa);
	}
	w = CMPLXF(r * cosf (theta), r * sinf (theta));
	return (w);
}

/*-
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

export double _Complex
cpow(double _Complex a, double _Complex z)
{
	double _Complex w;
	double x, y, r, theta, absa, arga;

	x = creal (z);
	y = cimag (z);
	absa = cabs (a);
	if (absa == 0.0) {
		if (x == 0 && y == 0)
		    return (CMPLX(1., 0.));
		else
		    return (CMPLX(0., 0.));
	}
	arga = carg (a);
	r = pow (absa, x);
	theta = x * arga;
	if (y != 0.0) {
		r = r * exp (-y * arga);
		theta = theta + y * log (absa);
	}
	w = CMPLX(r * cos (theta),  r * sin (theta));
	return (w);
}

} // namespace pbsd::lib_msun_src::b0312
