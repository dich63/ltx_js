#pragma once

#include <limits>

template <class _F>
inline _F _abs(const _F& f) {
	return (f >= 0) ? f : -f;
}

template <class FF>
inline static void set_NaN(FF& v) {
	const int64_t ii[4] = { -1LL, -1LL, -1LL, -1LL };
	v = *((FF*)ii);
}


inline static void set_NaN(double& v) {
	v = std::numeric_limits<double>::quiet_NaN();
}

inline static void set_NaN(float& v) {
	v = std::numeric_limits<float>::quiet_NaN();
}






template <template <class > class _COMPLEX, class F>
inline _COMPLEX< F> _fast_inv(_COMPLEX< F> a) {

    F r = a.real();
    F i = a.imag();
    F br = r * r + i * i;
    return { r / br,-i / br };
}


template <class F >
inline F  _fast_inv(F a) {
    return F(1) / a;
}

template <class F >
inline F  _fast0_inv(F a) {
	return F(1) / a;
}


template <template <class > class _COMPLEX, class F>
inline _COMPLEX< F> _fast2_inv(const _COMPLEX< F>& a) {

	constexpr F E = F(1);

	F r = a.real();
	F i = a.imag();
	F br =E/(r * r + i * i);
	return { r * br,-i * br };
}


template <template <class > class _COMPLEX, class F>
inline _COMPLEX< F> _fast_div(const _COMPLEX< F>& z1, const _COMPLEX< F>& z2) {

	F x1 = z1.real();
	F y1 = z1.imag();
	F x2 = z2.real();
	F y2 = z2.imag();

	F br = (x2 * x2 + y2 * y2);

	return { (x1 * x2 + y1 * y2)/br,(x2 * y1 - x1 * y2) / br };
}


template <template <class > class _COMPLEX, class F>
inline _COMPLEX< F> _fast2_div(const _COMPLEX< F>& z1,const _COMPLEX< F>& z2) {

	F x1 = z1.real();
	F y1 = z1.imag();
	F x2 = z2.real();
	F y2 = z2.imag();

	F br = F(1) / (x2 * x2 + y2 * y2);

	return {  br*(x1*x2+y1*y2), br*(x2*y1-x1*y2) };
}


template <class F >
inline F  _fast2_div(F a, F b) {
	return a/ b;
}

template <class F >
inline F  _fast_div(F a, F b) {
	return a / b;
}


template <class F >
inline F  _fast2_inv(F a) {
	return F(1) / a;
}


template <template <class > class _COMPLEX, class _FLOAT>
inline static _FLOAT mul_imag(_COMPLEX<_FLOAT>& f1, _COMPLEX<_FLOAT>& f2) {

	auto x1 = f1.real();
	auto y1 = f1.imag();
	auto x2 = f2.real();
	auto y2 = f2.imag();
	return x1 * y2 + x2 * y1;
}

template <template <class > class _COMPLEX, class _FLOAT>
inline static _FLOAT mul_real(_COMPLEX<_FLOAT>& f1, _COMPLEX<_FLOAT>& f2) {

	auto x1 = f1.real();
	auto y1 = f1.imag();
	auto x2 = f2.real();
	auto y2 = f2.imag();
	return x1 * x2 -y1 * y2;
}


template <template <class > class _COMPLEX, class _FLOAT>
inline static _FLOAT mul_imag_c(const _COMPLEX<_FLOAT>& fc, const _COMPLEX<_FLOAT>& f) {

	auto xc = fc.real();
	auto yc = fc.imag();
	auto x = f.real();
	auto y = f.imag();
	return xc * y - x * yc;
}

template <template <class > class _COMPLEX, class _FLOAT>
inline static _FLOAT sin_delta_c(const _COMPLEX<_FLOAT>& fp, const _COMPLEX<_FLOAT>& fm, const _COMPLEX<_FLOAT>& f0) {
	auto a0 = std::arg(f0);
	auto am = std::arg(fm);
	auto ap = std::arg(fp);
	return std::sin(ap - a0) + std::sin(am - a0);
}

template <template <class > class _COMPLEX>
inline static double sin_delta( _COMPLEX<double> fp,  _COMPLEX<double> fm,  _COMPLEX<double> f0) {

	return sin_delta_c(fp, fm, f0);
}


template <template <class > class _COMPLEX, class _FLOAT>
inline static _FLOAT mul_real_c(_COMPLEX<_FLOAT>& f1, _COMPLEX<_FLOAT>& f2) {

	auto x1 = f1.real();
	auto y1 = f1.imag();
	auto x2 = f2.real();
	auto y2 = f2.imag();
	return x1 * x2 + y1 * y2;
}



template <template <class > class _COMPLEX, class _FLOAT>
inline static _FLOAT abs2(const _COMPLEX<_FLOAT>& f) {
	auto x = f.real();
	auto y = f.imag();
	return x * x + y * y;
}

template <template <class > class _COMPLEX, class _FLOAT>
inline static _FLOAT absi(const _COMPLEX<_FLOAT>& f) {
	auto x = _abs(f.real());
	auto y = _abs(f.imag());
	return x > y ? x:y;
}

#define is_near_null is_nearly_null

template <template <class > class _COMPLEX, class _FLOAT>
inline static bool is_nearly_null(const _COMPLEX<_FLOAT>& f, _FLOAT eps = 1e-17) {
	return absi(f) < eps;
}

template <class _FLOAT>
inline static bool is_nearly_null(const _FLOAT& f, _FLOAT eps = 1e-17) {
	return _abs(f) < eps;
}

template <template <class > class _COMPLEX, class _FLOAT>
_FLOAT derr2e(size_t N , const _COMPLEX<_FLOAT>* x, const _COMPLEX<_FLOAT>* y) {

	_FLOAT sy=0.0, sx= 0.0, sd=0.0;
	for (auto k = 0; k < N; k++) {
		auto xr = x.real(), xi = x.imag();
		auto yr = x.real(), yi = x.imag();
		auto dr = xr - yr, di = xi - yi;
		sx += xr * xr + xi * xi;
		sy += yr * yr + yi * yi;
		sd += dr * dr + di * di;

	}

	return std::sqrt(sd/(sx+sy));

}

