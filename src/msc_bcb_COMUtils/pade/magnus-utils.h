#pragma once
#include "./utils.h"
#include "math.h"


//Integrate[Integrate[x*Exp[x], {x, 0, y}], {y, 0, t}]
template <class FLOAT>
FLOAT texp_2_1(FLOAT x) {
	const FLOAT f2 = 2.0;
	FLOAT r;
	auto x3 = x*x*x;
	if (_abs(x) > FLOAT(1e-2)) {
		 r = (f2 + x + exp(x)*(x - f2)) / x3;
	}
	else {

		auto x2 = x * x;
		r = (f2 +x)/ 12  + x2 / 40 + x3 / 180 + x3 * x / 1008+x3*x2/6720;
	}
	return r;
}

//Integrate[Exp[x], {x, 0, \[Tau]}]
template <class FLOAT>
FLOAT texp_1_0(FLOAT x) {

	const FLOAT f2 = 2.0;
	FLOAT r;
	auto x2 = x * x;
	if (_abs(x) > FLOAT(1e-4)) {

		r = (exp(x)-1) / x;
	}
	else {

		r = 1 + x / 2 + x2 / 6 + x * x2 / 24 + x2 * x2 / 120;
	}
	return r;
}

//Integrate[x*Exp[x], {x, 0, \[Tau]}]
template <class FLOAT>
FLOAT texp_1_1(FLOAT x) {
	const FLOAT f2 = 2.0;
	FLOAT r;
	auto x2 = x * x;
	if (_abs(x) > FLOAT(1e-4)) {

		r = (1 -  exp(x)*(1-x)) / x2;
	}
	else {
		
		r = 1 / f2 + x / 3 + x2 / 8 + x * x2 / 30 + x2 * x2 / 144;
	}
	return r;
}

//Integrate[x^2*Exp[x], {x, 0, \[Tau]}]
template <class FLOAT>
FLOAT texp_1_2(FLOAT x) {
	const FLOAT f2 = 2.0,f3=3.0;
	FLOAT r;
	auto x2 = x * x;
	auto x3 = x * x2;

	if (_abs(x) > FLOAT(1e-2)) {

		r = (exp(x)*(f2 +x*(x-f2))-f2) / x3;
	}
	else {

		r = 1 / f3 + x / 4 + x2 / 10 + x3 / 36 + x2 * x2 / 168 + x3 * x2 / 960;
	}
	return r;
}


//Integrate[Integrate[x*Exp[\[Gamma]*x], {x, 0, y}], {y, 0, \[Tau]}]
template <class FLOAT>
FLOAT texp_2_1(FLOAT t, FLOAT g) {
	auto t3 = t * t*t;
	return t3 * texp_2_1(g*t);

}


//Integrate[Exp[\[Gamma]*x], {x, 0, \[Tau]}]
template <class FLOAT>
FLOAT texp_1_0(FLOAT t, FLOAT g) {
	
	return t * texp_1_0(g*t);
}


//Integrate[x*Exp[\[Gamma]*x], {x, 0, \[Tau]}]
template <class FLOAT>
FLOAT texp_1_1(FLOAT t, FLOAT g) {
	auto t2 = t * t;
	return t2 * texp_1_1(g*t);
}

//Integrate[x^2*Exp[\[Gamma]*x], {x, 0, \[Tau]}]
template <class FLOAT>
FLOAT texp_1_2(FLOAT t, FLOAT g) {
	auto t3 = t * t*t;
	return t3 * texp_1_2(g*t);

}

