#pragma once

// modified from  http://www.beedub.com/Sprite093/src/lib/c/stdlib/atof.c

// for float, double , long double [80bits] , __float128

#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#include "utils/static_constructors_0.h"




	template <class _FLoat, int _M = 15>
	struct  powersOf10_t {

		typedef  _FLoat float_t;

		_FLoat powers[_M];
		_FLoat NaN;

		powersOf10_t() {

			powers[0] = float_t(10);
			int MM = _M,SS=sizeof(NaN);
			for (int m = 1; m < _M; m++) {
				powers[m] = powers[m - 1] * powers[m - 1];
			}

			char nn[16] = { 0xFF,0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

			NaN=*((float_t*)nn);
		}

		inline operator float_t* () {
			return powers;
		}

		static inline float_t reset(float_t v,float_t* p) {
			if (p)
				*p = v;
			return v;
		}

	};



	template <class _FLoat>
	struct max_mantissa_t {
		enum {
			size = 32,
			M10 = 13,
			maxExponent = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512 + 1024 + 2048
		};
	};

	template <>
	struct max_mantissa_t<double> {
		enum {
			size = 18,
			M10 = 9,
			maxExponent = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256
		};
	};
	template <>
	struct max_mantissa_t<long double> {
		enum {
			size = 18,
			M10 = 13,
			maxExponent = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512 + 1024 + 2048
		};
	};
	template <>
	struct max_mantissa_t<float> {
		enum {
			size = 9,
			M10 = 6,
			maxExponent = 1 + 2 + 4 + 8 + 16 + 32 +64
		};
	};



	template<class _FLOAT>
	_FLOAT floatNaN(_FLOAT* pfraction = (_FLOAT*)(nullptr)) {
		auto& pws = class_initializer_T< powersOf10_t<_FLOAT, max_mantissa_t<_FLOAT>::M10 > >::get();
		return pws.reset(pws.NaN, pfraction);
	}


template<class _FLOAT>
_FLOAT stof(const char* string, _FLOAT* pfraction = (_FLOAT*)(nullptr)) {

	typedef  _FLOAT float_t;

	float_t fraction, dblExp, * d;
	
	const int maxExponent = max_mantissa_t<float_t>::maxExponent;
	const int maxdig = max_mantissa_t<float_t>::size;
	const float_t ten = float_t(10);

	//powersOf10_t<float_t> pw10;
	auto& pws = class_initializer_T< powersOf10_t<float_t, max_mantissa_t<float_t>::M10 > >::get();
	float_t* powersOf10 = pws ;

	float_t NaN = pws.NaN;
	

	uint64_t frac1=0L, frac2=0L;

	
	int sign, expSign = FALSE;

	
	
	register char* p, c;
	int exp = 0;		/* Exponent read from "EX" field. */
	int fracExp = 0;		/* Exponent that derives from the fractional
				 * part.  Under normal circumstatnces, it is
				 * the negative of the number of digits in F.
				 * However, if I is very long, the last digits
				 * of I get dropped (otherwise a long I with a
				 * large negative exponent could cause an
				 * unnecessary overflow on I alone).  In this
				 * case, fracExp is incremented one for each
				 * dropped digit.
				 */
	int mantSize;		/* Number of digits in mantissa. */
	int decPt;			/* Number of mantissa digits BEFORE decimal
				 * point.
				 */
	char* pExp;			/* Temporarily holds location of exponent
				 * in string.
				 */

				 /*
				  * Strip off leading blanks and check for a sign.
				  */

	p = (char*)string;

	while (isspace(*p)) {
		p += 1;
	}
	if (*p == '-') {
		sign = TRUE;
		p += 1;
	}
	else {
		if (*p == '+') {
			p += 1;
		}
		sign = FALSE;
	}

	if (_strnicmp(p, "nan", 3) == 0) {	

		return fraction= pws.reset(NaN, pfraction);

	}
	/*
	 * Count the number of digits in the mantissa (including the decimal
	 * point), and also locate the decimal point.
	 */

	decPt = -1;
	for (mantSize = 0; ; mantSize += 1)
	{
		c = *p;
		if (!isdigit(c)) {
			if ((c != '.') || (decPt >= 0)) {
				break;
			}
			decPt = mantSize;
		}
		p += 1;
	}

	/*
	 * Now suck up the digits in the mantissa.  Use two integers to
	 * collect 9 digits each (this is faster than using floating-point).
	 * If the mantissa has more than 18 [maxdig] digits, ignore the extras, since
	 * they can't affect the value anyway.
	 */

	pExp = p;
	p -= mantSize;
	if (decPt < 0) {
		decPt = mantSize;
	}
	else {
		mantSize -= 1;			/* One of the digits was the point. */
	}
	if (mantSize > maxdig) {
		fracExp = decPt - maxdig;
		mantSize = maxdig;
	}
	else {
		fracExp = decPt - mantSize;
	}
	if (mantSize == 0) {
		return 0.0;
	}
	else {
		
		frac1 = 0;
		for (; mantSize > 18; mantSize -= 1)
		{
			c = *p;
			p += 1;
			if (c == '.') {
				c = *p;
				p += 1;
			}
			frac1 = 10L * frac1 + uint64_t(c - '0');
		}
		frac2 = 0;
		for (; mantSize > 0; mantSize -= 1)
		{
			c = *p;
			p += 1;
			if (c == '.') {
				c = *p;
				p += 1;
			}
			frac2 = 10L * frac2 + uint64_t(c - '0');
		}
		
		if (frac1)
			fraction = float_t(1000000000000000000L) * float_t(frac1) + float_t(frac2);
		else fraction = float_t(frac2);
		
	}

	/*
	 * Skim off the exponent.
	 */

	p = pExp;
	if ((*p == 'E') || (*p == 'e')) {
		p += 1;
		if (*p == '-') {
			expSign = TRUE;
			p += 1;
		}
		else {
			if (*p == '+') {
				p += 1;
			}
			expSign = FALSE;
		}
		while (isdigit(*p)) {
			exp = exp * 10 + (*p - '0');
			p += 1;
		}
	}
	if (expSign) {
		exp = fracExp - exp;
	}
	else {
		exp = fracExp + exp;
	}

	/*
	 * Generate a floating-point number that represents the exponent.
	 * Do this by processing the exponent one bit at a time to combine
	 * many powers of 2 of 10. Then combine the exponent with the
	 * fraction.
	 */

	if (exp < 0) {
		expSign = TRUE;
		exp = -exp;
	}
	else {
		expSign = FALSE;
	}
	if (exp > maxExponent) {
		exp = maxExponent;
	}
	dblExp = float_t(1);
	for (d = powersOf10; exp != 0; exp >>= 1, d += 1) {
		if (exp & 01) {
			dblExp *= *d;
		}
	}
	if (expSign) {
		fraction /= dblExp;
	}
	else {
		fraction *= dblExp;
	}

	if (sign) {
		fraction= -fraction;
	}

	 

	 return   pws.reset(fraction, pfraction);

}

