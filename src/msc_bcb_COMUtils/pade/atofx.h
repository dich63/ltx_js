#pragma once
/* 
 * atof.c --
 *
 *	Source code for the "atof" library procedure.
 *
 * Copyright 1988 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */


#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#include "utils/static_constructors_0.h"


static int maxExponent = 511;	/* Largest possible base 10 exponent.  Any
				 * exponent larger than this will already
				 * produce underflow or overflow, so there's
				 * no need to worry about additional digits.
				 */
static double powersOf10[] = {	/* Table giving binary powers of 10.  Entry */
    10.,			/* is 10^2^i.  Used to convert decimal */
    100.,			/* exponents into floating-point numbers. */
    1.0e4,
    1.0e8,
    1.0e16,
    1.0e32,
    1.0e64,
    1.0e128,
    1.0e256		
};


template <class _FLoat,int _M=15>
struct  powersOf10_t {

	typedef  _FLoat float_t;
	
	_FLoat powers[_M] ;

	powersOf10_t() {

		powers[0] = _FLoat(10);

		for (int m = 1; m < _M; m++) {
			powers[m] = powers[m - 1] * powers[m - 1];
		}

		
	}

	inline operator float_t* () {
		return powers;
	}
};

template <class _FLoat>
struct max_mantissa_t{
	enum {size=32,
		M10 = 13,
		maxExponent = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512 + 1024 + 2048
	};
};

template <>
struct max_mantissa_t<double> {
	enum { size = 18,
		M10 = 9,
		maxExponent= 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256
	};
};
template <>
struct max_mantissa_t<long double> {
	enum { size = 18 ,
		M10 = 13,
		maxExponent = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512 + 1024 + 2048
	};
};
template <>
struct max_mantissa_t<float> {
	enum { size = 9,
		M10 = 7,
		maxExponent = 1 + 2 + 4 + 8 + 16 + 32 + 64
	};
};




template<class _FLOAT>
_FLOAT stof(const char* string, _FLOAT* = ( _FLOAT*)(nullptr)) {

	typedef  _FLOAT float_t;

	float_t fraction = float_t(-1), dblExp, * d;
	const int maxExponent = max_mantissa_t<float_t>::maxExponent;
	const int maxdig = max_mantissa_t<float_t>::size;
	const float_t ten = float_t(10);

	//powersOf10_t<float_t> pw10;
	float_t* powersOf10 = class_initializer_T< powersOf10_t<float_t> >::get();
	

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
	 * If the mantissa has more than 18 digits, ignore the extras, since
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
			fraction = float_t(1000000000000000000L) * frac1 + frac2;
		else fraction =  frac2;		
		
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
	dblExp = 1.0;
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
		return -fraction;
	}
	return fraction;

}

/*
 *----------------------------------------------------------------------
 *
 * atof --
 *
 *	This procedure converts a floating-point number from an ASCII
 *	decimal representation to internal double-precision format.
 *
 * Results:
 *	The return value is the floating-point equivalent of string.
 *	If a terminating character is found before any floating-point
 *	digits, then zero is returned.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */



double atofx(const char* stringc)
       		/* A decimal ASCII floating-point number,
				 * optionally preceded by white space.
				 * Must have form "-I.FE-X", where I is the
				 * integer part of the mantissa, F is the
				 * fractional part of the mantissa, and X
				 * is the exponent.  Either of the signs
				 * may be "+", "-", or omitted.  Either I
				 * or F may be omitted, or both.  The decimal
				 * point isn't necessary unless F is present.
				 * The "E" may actually be an "e".  E and X
				 * may both be omitted (but not just one).
				 */
{
	char* string = (char*) stringc;
    int sign, expSign = FALSE;
    double fraction=-1, dblExp, *d;
    register char *p, c;
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
    char *pExp;			/* Temporarily holds location of exponent
				 * in string.
				 */

    /*
     * Strip off leading blanks and check for a sign.
     */

    p = string;
    while (isspace(*p)) {
	p += 1;
    }
    if (*p == '-') {
	sign = 	TRUE;
	p += 1;
    } else {
	if (*p == '+') {
	    p += 1;
	}
	sign = FALSE;
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
     * If the mantissa has more than 18 digits, ignore the extras, since
     * they can't affect the value anyway.
     */
    
    pExp  = p;
    p -= mantSize;
    if (decPt < 0) {
	decPt = mantSize;
    } else {
	mantSize -= 1;			/* One of the digits was the point. */
    }
    if (mantSize > 18) {
	fracExp = decPt - 18;
	mantSize = 18;
    } else {
	fracExp = decPt - mantSize;
    }
    if (mantSize == 0) {
	return 0.0;
    } else {
	int64_t frac1, frac2;
	frac1 = 0;
	for ( ; mantSize > 9; mantSize -= 1)
	{
	    c = *p;
	    p += 1;
	    if (c == '.') {
		c = *p;
		p += 1;
	    }
	    frac1 = 10*frac1 + (c - '0');
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
	    frac2 = 10*frac2 + (c - '0');
	}
	fraction = (1.0e9 * frac1) + frac2;
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
	} else {
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
    } else {
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
    } else {
	expSign = FALSE;
    }
    if (exp > maxExponent) {
	exp = maxExponent;
    }
    dblExp = 1.0;
    for (d = powersOf10; exp != 0; exp >>= 1, d += 1) {
	if (exp & 01) {
	    dblExp *= *d;
	}
    }
    if (expSign) {
	fraction /= dblExp;
    } else {
	fraction *= dblExp;
    }

    if (sign) {
	return -fraction;
    }
    return fraction;
}