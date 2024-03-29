// complex standard header

// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once
#ifndef _COMPLEX_
#define _COMPLEX_
#include <yvals_core.h>
#if _STL_COMPILER_PREPROCESSOR
#include <cmath>
#include <cstdint>
#include <sstream>
#include <ymath.h>

#pragma pack(push, _CRT_PACKING)
#pragma warning(push, _STL_WARNING_LEVEL)
#pragma warning(disable : _STL_DISABLED_WARNINGS)
_STL_DISABLE_CLANG_WARNINGS
#pragma push_macro("new")
#undef new

#ifndef _C_COMPLEX_T
#define _C_COMPLEX_T

struct _C_double_complex {
    double _Val[2];
};

struct _C_float_complex {
    float _Val[2];
};

struct _C_ldouble_complex {
    long double _Val[2];
};
#endif // _C_COMPLEX_T

// COMPLEX _Val OFFSETS
#define _RE 0
#define _IM 1

_STD_BEGIN
using _Dcomplex_value = _CSTD _C_double_complex;
using _Fcomplex_value = _CSTD _C_float_complex;
using _Lcomplex_value = _CSTD _C_ldouble_complex;

template <class _Ty>
class complex;
template <>
class complex<float>;
template <>
class complex<double>;
template <>
class complex<long double>;

// CLASS TEMPLATE _Ctraits
template <class _Ty>
class _Ctraits {
public:
    static constexpr _Ty _Flt_eps() { // get epsilon
        return numeric_limits<_Ty>::epsilon();
    }

    static constexpr _Ty _Flt_max() {
        return (numeric_limits<_Ty>::max)();
    }

    static _Ty _Cosh(_Ty _Left, _Ty _Right) { // return cosh(_Left) * _Right
        return static_cast<_Ty>(_CSTD _Cosh(static_cast<double>(_Left), static_cast<double>(_Right)));
    }

    static short _Exp(_Ty* _Pleft, _Ty _Right, short _Exponent) { // compute exp(*_Pleft) * _Right * 2 ^ _Exponent
        double _Tmp = static_cast<double>(*_Pleft);
        short _Ans  = _CSTD _Exp(&_Tmp, static_cast<double>(_Right), _Exponent);
        *_Pleft     = static_cast<_Ty>(_Tmp);
        return _Ans;
    }

    static constexpr _Ty _Infv() { // return infinity
        return numeric_limits<_Ty>::infinity();
    }

    static bool _Isinf(_Ty _Left) { // test for infinity
        const auto _Tmp  = static_cast<double>(_Left);
        const auto _Uint = _Bit_cast<uint64_t>(_Tmp);
        return (_Uint & 0x7fffffffffffffffU) == 0x7ff0000000000000U;
    }

    static _CONSTEXPR20 bool _Isnan(_Ty _Left) {
        const auto _Tmp  = static_cast<double>(_Left);
        const auto _Uint = _Bit_cast<uint64_t>(_Tmp);
        return (_Uint & 0x7fffffffffffffffU) > 0x7ff0000000000000U;
    }

    static constexpr _Ty _Nanv() { // return NaN
        return numeric_limits<_Ty>::quiet_NaN();
    }

    static _Ty _Sinh(_Ty _Left, _Ty _Right) { // return sinh(_Left) * _Right
        return static_cast<_Ty>(_CSTD _Sinh(static_cast<double>(_Left), static_cast<double>(_Right)));
    }

    static _Ty asinh(_Ty _Left) {
        constexpr _Ty _Ln2 = 0.69314718055994530941723212145817658L;

        bool _Neg = _Left < 0;
        _Ty _Ans;

        if (_Neg) {
            _Left = -_Left;
        }

        if (_Left < 2 / _Flt_eps()) {
            _Ans = log1p(_Left + _Left * _Left / (1 + sqrt(_Left * _Left + 1)));
        } else {
            _Ans = log(_Left) + _Ln2;
        }

        return static_cast<_Ty>(_Neg ? -_Ans : _Ans);
    }

    static _Ty atan2(_Ty _Yval, _Ty _Xval) { // return atan(_Yval / _Xval)
        return static_cast<_Ty>(_CSTD atan2(static_cast<double>(_Yval), static_cast<double>(_Xval)));
    }

    static _Ty cos(_Ty _Left) {
        return static_cast<_Ty>(_CSTD cos(static_cast<double>(_Left)));
    }

    static _Ty exp(_Ty _Left) {
        return static_cast<_Ty>(_CSTD exp(static_cast<double>(_Left)));
    }

    static _Ty ldexp(_Ty _Left, int _Exponent) { // return _Left * 2 ^ _Exponent
        return static_cast<_Ty>(_CSTD ldexp(static_cast<double>(_Left), _Exponent));
    }

    static _Ty log(_Ty _Left) {
        return static_cast<_Ty>(_CSTD log(static_cast<double>(_Left)));
    }

    static _Ty log1p(_Ty _Left) { // return log(1 + _Left)
        if (_Left < -1) {
            return _Nanv();
        } else if (_Left == 0) {
            return _Left;
        } else { // compute log(1 + _Left) with fixup for small _Left
            _Ty _Leftp1 = 1 + _Left;
            return log(_Leftp1) - ((_Leftp1 - 1) - _Left) / _Leftp1;
        }
    }

    static _Ty pow(_Ty _Left, _Ty _Right) {
        return static_cast<_Ty>(_CSTD pow(static_cast<double>(_Left), static_cast<double>(_Right)));
    }

    static _Ty sin(_Ty _Left) {
        return static_cast<_Ty>(_CSTD sin(static_cast<double>(_Left)));
    }

    static _Ty sqrt(_Ty _Left) {
        return static_cast<_Ty>(_CSTD sqrt(static_cast<double>(_Left)));
    }

    static _Ty tan(_Ty _Left) {
        return static_cast<_Ty>(_CSTD tan(static_cast<double>(_Left)));
    }

    static _Ty hypot(_Ty _Left, _Ty _Right) {
        return static_cast<_Ty>(_CSTD hypot(static_cast<double>(_Left), static_cast<double>(_Right)));
    }
};

// CLASS _Ctraits<long double>
template <>
class _Ctraits<long double> {
public:
    using _Ty = long double;

    static constexpr _Ty _Flt_eps() { // get epsilon
        return numeric_limits<long double>::epsilon();
    }

    static constexpr _Ty _Flt_max() {
        return (numeric_limits<long double>::max)();
    }

    static _Ty _Cosh(_Ty _Left, _Ty _Right) { // return cosh(_Left) * _Right
        return _CSTD _LCosh(_Left, _Right);
    }

    static short _Exp(_Ty* _Pleft, _Ty _Right, short _Exponent) { // compute exp(*_Pleft) * _Right * 2 ^ _Exponent
        return _CSTD _LExp(_Pleft, _Right, _Exponent);
    }

    static constexpr _Ty _Infv() { // return infinity
        return numeric_limits<long double>::infinity();
    }

    static bool _Isinf(_Ty _Left) { // test for infinity
        //const auto _Uint = _Bit_cast<uint64_t>(_Left);
        return _Left > _Left;
    }

    static _CONSTEXPR20 bool _Isnan(_Ty _Left) {
        //const auto _Uint = _Bit_cast<uint64_t>(_Left);
        return _Left != _Left;
        
    }

    static constexpr _Ty _Nanv() { // return NaN
        return numeric_limits<long double>::quiet_NaN();
    }

    static _Ty _Sinh(_Ty _Left, _Ty _Right) { // return sinh(_Left) * _Right
        return _CSTD _LSinh(_Left, _Right);
    }

    static _Ty asinh(_Ty _Left) {
        constexpr _Ty _Ln2 = 0.69314718055994530941723212145817658L;

        const bool _Neg = _Left < 0;
        _Ty _Ans;

        if (_Neg) {
            _Left = -_Left;
        }

        if (_Left < 2 / _Flt_eps()) {
            _Ans = log1p(_Left + _Left * _Left / (1 + sqrt(_Left * _Left + 1)));
        } else {
            _Ans = log(_Left) + _Ln2;
        }

        return _Neg ? -_Ans : _Ans;
    }

    static _Ty atan2(_Ty _Yval, _Ty _Xval) { // return atan(_Yval / _Xval)
        return _CSTD atan2l(_Yval, _Xval);
    }

    static _Ty cos(_Ty _Left) {
        return _CSTD cosl(_Left);
    }

    static _Ty exp(_Ty _Left) {
        return _CSTD expl(_Left);
    }

    static _Ty ldexp(_Ty _Left, int _Exponent) { // return _Left * 2 ^ _Exponent
        return _CSTD ldexpl(_Left, _Exponent);
    }

    static _Ty log(_Ty _Left) {
        return _CSTD logl(_Left);
    }

    static _Ty log1p(_Ty _Left) { // return log(1 + _Left)
        if (_Left < -1) {
            return _Nanv();
        } else if (_Left == 0) {
            return _Left;
        } else { // compute log(1 + _Left) with fixup for small _Left
            _Ty _Leftp1 = 1 + _Left;
            return log(_Leftp1) - ((_Leftp1 - 1) - _Left) / _Leftp1;
        }
    }

    static _Ty pow(_Ty _Left, _Ty _Right) {
        return _CSTD powl(_Left, _Right);
    }

    static _Ty sin(_Ty _Left) {
        return _CSTD sinl(_Left);
    }

    static _Ty sqrt(_Ty _Left) {
        return _CSTD sqrtl(_Left);
    }

    static _Ty tan(_Ty _Left) {
        return _CSTD tanl(_Left);
    }

    static _Ty hypot(_Ty _Left, _Ty _Right) {
        return _CSTD hypotl(_Left, _Right);
    }
};

// CLASS _Ctraits<double>
template <>
class _Ctraits<double> {
public:
    using _Ty = double;

    static constexpr _Ty _Flt_eps() { // get epsilon
        return numeric_limits<double>::epsilon();
    }

    static constexpr _Ty _Flt_max() {
        return (numeric_limits<double>::max)();
    }

    static _Ty _Cosh(_Ty _Left, _Ty _Right) { // return cosh(_Left) * _Right
        return _CSTD _Cosh(_Left, _Right);
    }

    static short _Exp(_Ty* _Pleft, _Ty _Right, short _Exponent) { // compute exp(*_Pleft) * _Right * 2 ^ _Exponent
        return _CSTD _Exp(_Pleft, _Right, _Exponent);
    }

    static constexpr _Ty _Infv() { // return infinity
        return numeric_limits<double>::infinity();
    }

    static bool _Isinf(_Ty _Left) { // test for infinity
        const auto _Uint = _Bit_cast<uint64_t>(_Left);
        return (_Uint & 0x7fffffffffffffffU) == 0x7ff0000000000000U;
    }

    static _CONSTEXPR20 bool _Isnan(_Ty _Left) {
        const auto _Uint = _Bit_cast<uint64_t>(_Left);
        return (_Uint & 0x7fffffffffffffffU) > 0x7ff0000000000000U;
    }

    static constexpr _Ty _Nanv() { // return NaN
        return numeric_limits<double>::quiet_NaN();
    }

    static _Ty _Sinh(_Ty _Left, _Ty _Right) { // return sinh(_Left) * _Right
        return _CSTD _Sinh(_Left, _Right);
    }

    static _Ty asinh(_Ty _Left) {
        if (_Isnan(_Left) || _Isinf(_Left) || _Left == 0) {
            return 0;
        } else { // _Left finite nonzero
            const bool _Neg = _Left < 0;
            _Ty _Ans;

            if (_Neg) {
                _Left = -_Left;
            }

            if (_Left < 2 / _Flt_eps()) {
                _Ans = log1p(_Left + _Left * _Left / (1 + sqrt(_Left * _Left + 1)));
            } else {
                _Ans = log(_Left) // _Left big, compute log(_Left+_Left)
                       + static_cast<_Ty>(0.69314718055994530941723212145817658L);
            }

            return _Neg ? -_Ans : _Ans;
        }
    }

    static _Ty atan2(_Ty _Yval, _Ty _Xval) { // return atan(_Yval / _Xval)
        return _CSTD atan2(_Yval, _Xval);
    }

    static _Ty cos(_Ty _Left) {
        return _CSTD cos(_Left);
    }

    static _Ty exp(_Ty _Left) {
        return _CSTD exp(_Left);
    }

    static _Ty ldexp(_Ty _Left, int _Exponent) { // return _Left * 2 ^ _Exponent
        return _CSTD ldexp(_Left, _Exponent);
    }

    static _Ty log(_Ty _Left) {
        return _CSTD log(_Left);
    }

    static _Ty log1p(_Ty _Left) { // return log(1 + _Left)
        if (_Isnan(_Left) || _Left == 0 || (_Isinf(_Left) && 0 < _Left)) {
            return _Left;
        } else if (_Left < -1) {
            return _Nanv();
        } else if (_Left == -1) {
            return -_Infv();
        } else if (_Left == 0) {
            return _Left;
        } else { // compute log(1 + _Left) with fixup for small _Left
            _Ty _Leftp1 = 1 + _Left;
            return log(_Leftp1) - ((_Leftp1 - 1) - _Left) / _Leftp1;
        }
    }

    static _Ty pow(_Ty _Left, _Ty _Right) {
        return _CSTD pow(_Left, _Right);
    }

    static _Ty sin(_Ty _Left) {
        return _CSTD sin(_Left);
    }

    static _Ty sqrt(_Ty _Left) {
        return _CSTD sqrt(_Left);
    }

    static _Ty tan(_Ty _Left) {
        return _CSTD tan(_Left);
    }

    static _Ty hypot(_Ty _Left, _Ty _Right) {
        return _CSTD hypot(_Left, _Right);
    }
};

// CLASS _Ctraits<float>
template <>
class _Ctraits<float> {
public:
    using _Ty = float;

    static constexpr _Ty _Flt_eps() { // get epsilon
        return numeric_limits<float>::epsilon();
    }

    static constexpr _Ty _Flt_max() {
        return (numeric_limits<float>::max)();
    }

    static _Ty _Cosh(_Ty _Left, _Ty _Right) { // return cosh(_Left) * _Right
        return _CSTD _FCosh(_Left, _Right);
    }

    static short _Exp(_Ty* _Pleft, _Ty _Right, short _Exponent) { // compute exp(*_Pleft) * _Right * 2 ^ _Exponent
        return _CSTD _FExp(_Pleft, _Right, _Exponent);
    }

    static constexpr _Ty _Infv() { // return infinity
        return numeric_limits<float>::infinity();
    }

    static bool _Isinf(_Ty _Left) { // test for infinity
        const auto _Uint = _Bit_cast<uint32_t>(_Left);
        return (_Uint & 0x7fffffffU) == 0x7f800000U;
    }

    static _CONSTEXPR20 bool _Isnan(_Ty _Left) {
        const auto _Uint = _Bit_cast<uint32_t>(_Left);
        return (_Uint & 0x7fffffffU) > 0x7f800000U;
    }

    static constexpr _Ty _Nanv() { // return NaN
        return numeric_limits<float>::quiet_NaN();
    }

    static _Ty _Sinh(_Ty _Left, _Ty _Right) { // return sinh(_Left) * _Right
        return _CSTD _FSinh(_Left, _Right);
    }

    static _Ty asinh(_Ty _Left) {
        constexpr _Ty _Ln2 = 0.69314718055994530941723212145817658F;

        const bool _Neg = _Left < 0;
        _Ty _Ans;

        if (_Neg) {
            _Left = -_Left;
        }

        if (_Left < 2 / _Flt_eps()) {
            _Ans = log1p(_Left + _Left * _Left / (1 + sqrt(_Left * _Left + 1)));
        } else {
            _Ans = log(_Left) + _Ln2;
        }

        return _Neg ? -_Ans : _Ans;
    }

    static _Ty atan2(_Ty _Yval, _Ty _Xval) { // return atan(_Yval / _Xval)
        return _CSTD atan2f(_Yval, _Xval);
    }

    static _Ty cos(_Ty _Left) {
        return _CSTD cosf(_Left);
    }

    static _Ty exp(_Ty _Left) {
        return _CSTD expf(_Left);
    }

    static _Ty ldexp(_Ty _Left, int _Exponent) { // return _Left * 2 ^ _Exponent
        return _CSTD ldexpf(_Left, _Exponent);
    }

    static _Ty log(_Ty _Left) {
        return _CSTD logf(_Left);
    }

    static _Ty log1p(_Ty _Left) { // return log(1 + _Left)
        if (_Left < -1) {
            return _Nanv();
        } else if (_Left == 0) {
            return _Left;
        } else { // compute log(1 + _Left) with fixup for small _Left
            _Ty _Leftp1 = 1 + _Left;
            return log(_Leftp1) - ((_Leftp1 - 1) - _Left) / _Leftp1;
        }
    }

    static _Ty pow(_Ty _Left, _Ty _Right) {
        return _CSTD powf(_Left, _Right);
    }

    static _Ty sin(_Ty _Left) {
        return _CSTD sinf(_Left);
    }

    static _Ty sqrt(_Ty _Left) {
        return _CSTD sqrtf(_Left);
    }

    static _Ty tan(_Ty _Left) {
        return _CSTD tanf(_Left);
    }

    static _Ty hypot(_Ty _Left, _Ty _Right) {
        return _CSTD hypotf(_Left, _Right);
    }
};

// STRUCT TEMPLATE _Complex_value
template <class _Ty>
struct _Complex_value {
    enum { _Re = 0, _Im = 1 };
    _Ty _Val[2];
};

// CLASS TEMPLATE _Complex_base
template <class _Ty, class _Valbase>
class _Complex_base : public _Valbase {
public:
    using _Myctraits = _Ctraits<_Ty>;
    using value_type = _Ty;

    constexpr _Complex_base(const _Ty& _Realval, const _Ty& _Imagval) : _Valbase{{_Realval, _Imagval}} {}

    _CONSTEXPR20 void real(const _Ty& _Right) { // set real component
        this->_Val[_RE] = _Right;
    }

    _CONSTEXPR20 void imag(const _Ty& _Right) { // set imaginary component
        this->_Val[_IM] = _Right;
    }

    _NODISCARD constexpr _Ty real() const { // return real component
        return this->_Val[_RE];
    }

    _NODISCARD constexpr _Ty imag() const { // return imaginary component
        return this->_Val[_IM];
    }

protected:
    template <class _Other>
    _CONSTEXPR20 void _Add(const complex<_Other>& _Right) {
        this->_Val[_RE] = this->_Val[_RE] + static_cast<_Ty>(_Right.real());
        this->_Val[_IM] = this->_Val[_IM] + static_cast<_Ty>(_Right.imag());
    }

    template <class _Other>
    _CONSTEXPR20 void _Sub(const complex<_Other>& _Right) {
        this->_Val[_RE] = this->_Val[_RE] - static_cast<_Ty>(_Right.real());
        this->_Val[_IM] = this->_Val[_IM] - static_cast<_Ty>(_Right.imag());
    }

    template <class _Other>
    _CONSTEXPR20 void _Mul(const complex<_Other>& _Right) {
        _Ty _Rightreal = static_cast<_Ty>(_Right.real());
        _Ty _Rightimag = static_cast<_Ty>(_Right.imag());

        _Ty _Tmp        = this->_Val[_RE] * _Rightreal - this->_Val[_IM] * _Rightimag;
        this->_Val[_IM] = this->_Val[_RE] * _Rightimag + this->_Val[_IM] * _Rightreal;
        this->_Val[_RE] = _Tmp;
    }

    template <class _Other>
    _CONSTEXPR20 void _Div(const complex<_Other>& _Right) {
        using _Myctraits = _Ctraits<_Ty>;

        _Ty _Rightreal = static_cast<_Ty>(_Right.real());
        _Ty _Rightimag = static_cast<_Ty>(_Right.imag());

        if (_Myctraits::_Isnan(_Rightreal) || _Myctraits::_Isnan(_Rightimag)) { // set NaN result
            this->_Val[_RE] = _Myctraits::_Nanv();
            this->_Val[_IM] = this->_Val[_RE];
        } else if ((_Rightimag < 0 ? -_Rightimag : +_Rightimag)
                   < (_Rightreal < 0 ? -_Rightreal : +_Rightreal)) { // |_Right.imag()| < |_Right.real()|
            _Ty _Wr = _Rightimag / _Rightreal;
            _Ty _Wd = _Rightreal + _Wr * _Rightimag;

            if (_Myctraits::_Isnan(_Wd) || _Wd == 0) { // set NaN result
                this->_Val[_RE] = _Myctraits::_Nanv();
                this->_Val[_IM] = this->_Val[_RE];
            } else { // compute representable result
                _Ty _Tmp        = (this->_Val[_RE] + this->_Val[_IM] * _Wr) / _Wd;
                this->_Val[_IM] = (this->_Val[_IM] - this->_Val[_RE] * _Wr) / _Wd;
                this->_Val[_RE] = _Tmp;
            }
        } else if (_Rightimag == 0) { // set NaN result
            this->_Val[_RE] = _Myctraits::_Nanv();
            this->_Val[_IM] = this->_Val[_RE];
        } else { // 0 < |_Right.real()| <= |_Right.imag()|
            _Ty _Wr = _Rightreal / _Rightimag;
            _Ty _Wd = _Rightimag + _Wr * _Rightreal;

            if (_Myctraits::_Isnan(_Wd) || _Wd == 0) { // set NaN result
                this->_Val[_RE] = _Myctraits::_Nanv();
                this->_Val[_IM] = this->_Val[_RE];
            } else { // compute representable result
                _Ty _Tmp        = (this->_Val[_RE] * _Wr + this->_Val[_IM]) / _Wd;
                this->_Val[_IM] = (this->_Val[_IM] * _Wr - this->_Val[_RE]) / _Wd;
                this->_Val[_RE] = _Tmp;
            }
        }
    }
};

// CLASS complex<float>
template <>
class complex<float> : public _Complex_base<float, _Fcomplex_value> {
public:
    using _Ty = float;

    constexpr explicit complex(const complex<double>&); // defined below
    constexpr explicit complex(const complex<long double>&); // defined below

    constexpr complex(const _Ty& _Realval = 0, const _Ty& _Imagval = 0)
        : _Complex_base<float, _Fcomplex_value>(_Realval, _Imagval) {}

    constexpr complex(const _Fcomplex_value& _Right)
        : _Complex_base<float, _Fcomplex_value>(_Right._Val[_RE], _Right._Val[_IM]) {}

    constexpr complex(const _Dcomplex_value& _Right)
        : _Complex_base<float, _Fcomplex_value>(
            static_cast<float>(_Right._Val[_RE]), static_cast<float>(_Right._Val[_IM])) {}

    constexpr complex(const _Lcomplex_value& _Right)
        : _Complex_base<float, _Fcomplex_value>(
            static_cast<float>(_Right._Val[_RE]), static_cast<float>(_Right._Val[_IM])) {}

    _CONSTEXPR20 complex& operator=(const _Ty& _Right) {
        _Val[_RE] = _Right;
        _Val[_IM] = 0;
        return *this;
    }

    _CONSTEXPR20 complex& operator+=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] + _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator-=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] - _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator*=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] * _Right;
        _Val[_IM] = _Val[_IM] * _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator/=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] / _Right;
        _Val[_IM] = _Val[_IM] / _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator+=(const complex& _Right) {
        this->_Add(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator-=(const complex& _Right) {
        this->_Sub(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator*=(const complex& _Right) {
        this->_Mul(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator/=(const complex& _Right) {
        this->_Div(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator=(const complex<_Other>& _Right) {
        _Val[_RE] = static_cast<_Ty>(_Right._Val[_RE]);
        _Val[_IM] = static_cast<_Ty>(_Right._Val[_IM]);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator+=(const complex<_Other>& _Right) {
        this->_Add(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator-=(const complex<_Other>& _Right) {
        this->_Sub(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator*=(const complex<_Other>& _Right) {
        this->_Mul(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator/=(const complex<_Other>& _Right) {
        this->_Div(_Right);
        return *this;
    }
};

// CLASS complex<double>
template <>
class complex<double> : public _Complex_base<double, _Dcomplex_value> {
public:
    using _Ty = double;

    constexpr complex(const complex<float>&); // defined below
    constexpr explicit complex(const complex<long double>&); // defined below

    constexpr complex(const _Ty& _Realval = 0, const _Ty& _Imagval = 0)
        : _Complex_base<double, _Dcomplex_value>(_Realval, _Imagval) {}

    constexpr complex(const _Dcomplex_value& _Right)
        : _Complex_base<double, _Dcomplex_value>(_Right._Val[_RE], _Right._Val[_IM]) {}

    constexpr complex(const _Lcomplex_value& _Right)
        : _Complex_base<double, _Dcomplex_value>(
            static_cast<double>(_Right._Val[_RE]), static_cast<double>(_Right._Val[_IM])) {}

    _CONSTEXPR20 complex& operator=(const _Ty& _Right) {
        _Val[_RE] = _Right;
        _Val[_IM] = 0;
        return *this;
    }

    _CONSTEXPR20 complex& operator+=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] + _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator-=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] - _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator*=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] * _Right;
        _Val[_IM] = _Val[_IM] * _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator/=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] / _Right;
        _Val[_IM] = _Val[_IM] / _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator+=(const complex& _Right) {
        this->_Add(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator-=(const complex& _Right) {
        this->_Sub(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator*=(const complex& _Right) {
        this->_Mul(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator/=(const complex& _Right) {
        this->_Div(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator=(const complex<_Other>& _Right) {
        _Val[_RE] = static_cast<_Ty>(_Right._Val[_RE]);
        _Val[_IM] = static_cast<_Ty>(_Right._Val[_IM]);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator+=(const complex<_Other>& _Right) {
        this->_Add(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator-=(const complex<_Other>& _Right) {
        this->_Sub(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator*=(const complex<_Other>& _Right) {
        this->_Mul(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator/=(const complex<_Other>& _Right) {
        this->_Div(_Right);
        return *this;
    }
};

// CLASS complex<long double>
template <>
class complex<long double> : public _Complex_base<long double, _Lcomplex_value> {
public:
    using _Ty = long double;

    constexpr complex(const complex<float>&); // defined below
    constexpr complex(const complex<double>&); // defined below

    constexpr complex(const _Ty& _Realval = 0, const _Ty& _Imagval = 0)
        : _Complex_base<long double, _Lcomplex_value>(_Realval, _Imagval) {}

    constexpr complex(const _Lcomplex_value& _Right)
        : _Complex_base<long double, _Lcomplex_value>(_Right._Val[_RE], _Right._Val[_IM]) {}

    _CONSTEXPR20 complex& operator=(const _Ty& _Right) {
        _Val[_RE] = _Right;
        _Val[_IM] = 0;
        return *this;
    }

    _CONSTEXPR20 complex& operator+=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] + _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator-=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] - _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator*=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] * _Right;
        _Val[_IM] = _Val[_IM] * _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator/=(const _Ty& _Right) {
        _Val[_RE] = _Val[_RE] / _Right;
        _Val[_IM] = _Val[_IM] / _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator+=(const complex& _Right) {
        this->_Add(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator-=(const complex& _Right) {
        this->_Sub(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator*=(const complex& _Right) {
        this->_Mul(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator/=(const complex& _Right) {
        this->_Div(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator=(const complex<_Other>& _Right) {
        _Val[_RE] = static_cast<_Ty>(_Right._Val[_RE]);
        _Val[_IM] = static_cast<_Ty>(_Right._Val[_IM]);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator+=(const complex<_Other>& _Right) {
        this->_Add(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator-=(const complex<_Other>& _Right) {
        this->_Sub(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator*=(const complex<_Other>& _Right) {
        this->_Mul(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator/=(const complex<_Other>& _Right) {
        this->_Div(_Right);
        return *this;
    }
};

// CONSTRUCTORS FOR complex SPECIALIZATIONS
constexpr complex<float>::complex(const complex<double>& _Right)
    : _Complex_base<float, _Fcomplex_value>(static_cast<_Ty>(_Right.real()), static_cast<_Ty>(_Right.imag())) {}

constexpr complex<float>::complex(const complex<long double>& _Right)
    : _Complex_base<float, _Fcomplex_value>(static_cast<_Ty>(_Right.real()), static_cast<_Ty>(_Right.imag())) {}

constexpr complex<double>::complex(const complex<float>& _Right)
    : _Complex_base<double, _Dcomplex_value>(_Right.real(), _Right.imag()) {}

constexpr complex<double>::complex(const complex<long double>& _Right)
    : _Complex_base<double, _Dcomplex_value>(static_cast<_Ty>(_Right.real()), static_cast<_Ty>(_Right.imag())) {}

constexpr complex<long double>::complex(const complex<float>& _Right)
    : _Complex_base<long double, _Lcomplex_value>(_Right.real(), _Right.imag()) {}

constexpr complex<long double>::complex(const complex<double>& _Right)
    : _Complex_base<long double, _Lcomplex_value>(_Right.real(), _Right.imag()) {}

// CLASS TEMPLATE complex
template <class _Ty>
class complex : public _Complex_base<_Ty, _Complex_value<_Ty>> {
public:
    using _Mybase = _Complex_base<_Ty, _Complex_value<_Ty>>;

    constexpr complex(const _Ty& _Realval = _Ty(), const _Ty& _Imagval = _Ty()) : _Mybase(_Realval, _Imagval) {}

    _CONSTEXPR20 complex& operator=(const _Ty& _Right) {
        this->_Val[_RE] = _Right;
        this->_Val[_IM] = _Ty();
        return *this;
    }

    template <class _Other>
    constexpr complex(const complex<_Other>& _Right)
        : _Mybase(static_cast<_Ty>(_Right.real()), static_cast<_Ty>(_Right.imag())) {}

    template <class _Other>
    _CONSTEXPR20 complex& operator=(const complex<_Other>& _Right) {
        this->_Val[_RE] = static_cast<_Ty>(_Right.real());
        this->_Val[_IM] = static_cast<_Ty>(_Right.imag());
        return *this;
    }

    _CONSTEXPR20 complex& operator+=(const _Ty& _Right) {
        this->_Val[_RE] = this->_Val[_RE] + _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator-=(const _Ty& _Right) {
        this->_Val[_RE] = this->_Val[_RE] - _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator*=(const _Ty& _Right) {
        this->_Val[_RE] = this->_Val[_RE] * _Right;
        this->_Val[_IM] = this->_Val[_IM] * _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator/=(const _Ty& _Right) {
        this->_Val[_RE] = this->_Val[_RE] / _Right;
        this->_Val[_IM] = this->_Val[_IM] / _Right;
        return *this;
    }

    _CONSTEXPR20 complex& operator+=(const complex& _Right) {
        this->_Add(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator-=(const complex& _Right) {
        this->_Sub(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator*=(const complex& _Right) {
        this->_Mul(_Right);
        return *this;
    }

    _CONSTEXPR20 complex& operator/=(const complex& _Right) {
        this->_Div(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator+=(const complex<_Other>& _Right) {
        this->_Add(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator-=(const complex<_Other>& _Right) {
        this->_Sub(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator*=(const complex<_Other>& _Right) {
        this->_Mul(_Right);
        return *this;
    }

    template <class _Other>
    _CONSTEXPR20 complex& operator/=(const complex<_Other>& _Right) {
        this->_Div(_Right);
        return *this;
    }
};

// FUNCTION TEMPLATE operator+
template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator+(const complex<_Ty>& _Left, const complex<_Ty>& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp += _Right;
    return _Tmp;
}

template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator+(const complex<_Ty>& _Left, const _Ty& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp.real(_Tmp.real() + _Right);
    return _Tmp;
}

template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator+(const _Ty& _Left, const complex<_Ty>& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp += _Right;
    return _Tmp;
}

// FUNCTION TEMPLATE operator-
template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator-(const complex<_Ty>& _Left, const complex<_Ty>& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp -= _Right;
    return _Tmp;
}

template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator-(const complex<_Ty>& _Left, const _Ty& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp.real(_Tmp.real() - _Right);
    return _Tmp;
}

template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator-(const _Ty& _Left, const complex<_Ty>& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp -= _Right;
    return _Tmp;
}

// FUNCTION TEMPLATE operator*
template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator*(const complex<_Ty>& _Left, const complex<_Ty>& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp *= _Right;
    return _Tmp;
}

template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator*(const complex<_Ty>& _Left, const _Ty& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp.real(_Tmp.real() * _Right);
    _Tmp.imag(_Tmp.imag() * _Right);
    return _Tmp;
}

template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator*(const _Ty& _Left, const complex<_Ty>& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp *= _Right;
    return _Tmp;
}

// FUNCTION TEMPLATE operator/
template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator/(const complex<_Ty>& _Left, const complex<_Ty>& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp /= _Right;
    return _Tmp;
}

template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator/(const complex<_Ty>& _Left, const _Ty& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp.real(_Tmp.real() / _Right);
    _Tmp.imag(_Tmp.imag() / _Right);
    return _Tmp;
}

template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator/(const _Ty& _Left, const complex<_Ty>& _Right) {
    complex<_Ty> _Tmp(_Left);
    _Tmp /= _Right;
    return _Tmp;
}

// FUNCTION TEMPLATE UNARY operator+
template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator+(const complex<_Ty>& _Left) {
    return _Left;
}

// FUNCTION TEMPLATE UNARY operator-
template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> operator-(const complex<_Ty>& _Left) {
    return complex<_Ty>(-_Left.real(), -_Left.imag());
}

// FUNCTION TEMPLATE operator==
template <class _Ty>
_NODISCARD constexpr bool operator==(const complex<_Ty>& _Left, const complex<_Ty>& _Right) {
    return _Left.real() == _Right.real() && _Left.imag() == _Right.imag();
}

template <class _Ty>
_NODISCARD constexpr bool operator==(const complex<_Ty>& _Left, const _Ty& _Right) {
    return _Left.real() == _Right && _Left.imag() == 0;
}

template <class _Ty>
_NODISCARD constexpr bool operator==(const _Ty& _Left, const complex<_Ty>& _Right) {
    return _Left == _Right.real() && 0 == _Right.imag();
}

// FUNCTION TEMPLATE operator!=
template <class _Ty>
_NODISCARD constexpr bool operator!=(const complex<_Ty>& _Left, const complex<_Ty>& _Right) {
    return !(_Left == _Right);
}

template <class _Ty>
_NODISCARD constexpr bool operator!=(const complex<_Ty>& _Left, const _Ty& _Right) {
    return !(_Left == _Right);
}

template <class _Ty>
_NODISCARD constexpr bool operator!=(const _Ty& _Left, const complex<_Ty>& _Right) {
    return !(_Left == _Right);
}

// FUNCTION TEMPLATE imag
template <class _Ty>
_NODISCARD constexpr _Ty imag(const complex<_Ty>& _Left) {
    return _Left.imag();
}

// FUNCTION TEMPLATE real
template <class _Ty>
_NODISCARD constexpr _Ty real(const complex<_Ty>& _Left) {
    return _Left.real();
}

template <class _Ty>
_NODISCARD complex<_Ty> sqrt(const complex<_Ty>& _Left);

// FUNCTION TEMPLATE abs
template <class _Ty>
_NODISCARD _Ty abs(const complex<_Ty>& _Left) {
    return _Ctraits<_Ty>::hypot(_Left.real(), _Left.imag());
}

// FUNCTION TEMPLATE acos
template <class _Ty>
_NODISCARD complex<_Ty> acos(const complex<_Ty>& _Left) {
    const _Ty _Arcbig = static_cast<_Ty>(0.25) * _Ctraits<_Ty>::sqrt(_Ctraits<_Ty>::_Flt_max());
    constexpr _Ty _Pi = static_cast<_Ty>(3.1415926535897932384626433832795029L);

    const _Ty _Re = real(_Left);
    const _Ty _Im = imag(_Left);
    _Ty _Ux;
    _Ty _Vx;

    if (_Ctraits<_Ty>::_Isnan(_Re) || _Ctraits<_Ty>::_Isnan(_Im)) { // at least one NaN
        _Ux = _Ctraits<_Ty>::_Nanv();
        _Vx = _Ux;
    } else if (_Ctraits<_Ty>::_Isinf(_Re)) { // (+/-Inf, not NaN)
        if (_Ctraits<_Ty>::_Isinf(_Im)) {
            if (_Re < 0) {
                _Ux = static_cast<_Ty>(0.75) * _Pi; // (-Inf, +/-Inf)
            } else {
                _Ux = static_cast<_Ty>(0.25) * _Pi; // (+Inf, +/-Inf)
            }
        } else if (_Re < 0) {
            _Ux = _Pi; // (-Inf, finite)
        } else {
            _Ux = 0; // (+Inf, finite)
        }
        _Vx = -_Ctraits<_Ty>::_Infv();
        if (_Im < 0) {
            _Vx = -_Vx;
        }
    } else if (_Ctraits<_Ty>::_Isinf(_Im)) { // (finite, finite)
        _Ux = static_cast<_Ty>(0.50) * _Pi; // (finite, +/-Inf)
        _Vx = -_Im;
    } else { // (finite, finite)
        const complex<_Ty> _Wx = sqrt(complex<_Ty>(1 + _Re, -_Im));
        const complex<_Ty> _Zx = sqrt(complex<_Ty>(1 - _Re, -_Im));
        const _Ty _Wr          = real(_Wx);
        const _Ty _Wi          = imag(_Wx);
        const _Ty _Zr          = real(_Zx);
        const _Ty _Zi          = imag(_Zx);
        _Ty _Alfa;
        _Ty _Beta;

        _Ux = 2 * _Ctraits<_Ty>::atan2(_Zr, _Wr);

        if (_Arcbig < _Wr) { // real parts large
            _Alfa = _Wr;
            _Beta = _Zi + _Wi * (_Zr / _Alfa);
        } else if (_Arcbig < _Wi) { // imag parts large
            _Alfa = _Wi;
            _Beta = _Wr * (_Zi / _Alfa) + _Zr;
        } else if (_Wi < -_Arcbig) { // imag part of w large negative
            _Alfa = -_Wi;
            _Beta = _Wr * (_Zi / _Alfa) - _Zr;
        } else { // shouldn't overflow
            _Alfa = 0;
            _Beta = _Wr * _Zi + _Wi * _Zr; // Im(w * z)
        }

        _Vx = _Ctraits<_Ty>::asinh(_Beta);
        if (_Alfa != 0) {
            if (0 <= _Vx) {
                _Vx += _Ctraits<_Ty>::log(_Alfa);
            } else {
                _Vx -= _Ctraits<_Ty>::log(_Alfa); // asinh(a*b) = asinh(a)+log(b)
            }
        }
    }
    return complex<_Ty>(_Ux, _Vx);
}

// FUNCTION TEMPLATE acosh
template <class _Ty>
_NODISCARD complex<_Ty> acosh(const complex<_Ty>& _Left) {
    const _Ty _Arcbig = static_cast<_Ty>(0.25) * _Ctraits<_Ty>::sqrt(_Ctraits<_Ty>::_Flt_max());
    constexpr _Ty _Pi = static_cast<_Ty>(3.1415926535897932384626433832795029L);

    const _Ty _Re = real(_Left);
    _Ty _Im       = imag(_Left);
    _Ty _Ux;
    _Ty _Vx;

    if (_Ctraits<_Ty>::_Isnan(_Re) || _Ctraits<_Ty>::_Isnan(_Im)) { // at least one NaN
        _Ux = _Ctraits<_Ty>::_Nanv();
        _Vx = _Ux;
    } else if (_Ctraits<_Ty>::_Isinf(_Re)) { // (+/-Inf, not NaN)
        _Ux = _Ctraits<_Ty>::_Infv();

        if (_Ctraits<_Ty>::_Isinf(_Im)) {
            if (_Re < 0) {
                _Vx = static_cast<_Ty>(0.75) * _Pi; // (-Inf, +/-Inf)
            } else {
                _Vx = static_cast<_Ty>(0.25) * _Pi; // (+Inf, +/-Inf)
            }
        } else if (_Re < 0) {
            _Vx = _Pi; // (-Inf, finite)
        } else {
            _Vx = 0; // (+Inf, finite)
        }

        if (_Im < 0) {
            _Vx = -_Vx;
        }
    } else if (_Ctraits<_Ty>::_Isinf(_Im)) { // (finite, +/-Inf)
        _Ux = _Ctraits<_Ty>::_Infv();
        _Vx = _Im < 0 ? -static_cast<_Ty>(0.50) * _Pi : static_cast<_Ty>(0.50) * _Pi;
    } else { // (finite, finite)
        const complex<_Ty> _Wx = sqrt(complex<_Ty>(_Re - 1, -_Im));
        const complex<_Ty> _Zx = sqrt(complex<_Ty>(_Re + 1, _Im));
        const _Ty _Wr          = real(_Wx);
        const _Ty _Wi          = imag(_Wx);
        const _Ty _Zr          = real(_Zx);
        const _Ty _Zi          = imag(_Zx);
        _Ty _Alfa;
        _Ty _Beta;

        if (_Arcbig < _Wr) { // real parts large
            _Alfa = _Wr;
            _Beta = _Zr - _Wi * (_Zi / _Alfa);
        } else if (_Arcbig < _Wi) { // imag parts large
            _Alfa = _Wi;
            _Beta = _Wr * (_Zr / _Alfa) - _Zi;
        } else if (_Wi < -_Arcbig) { // imag part of w large negative
            _Alfa = -_Wi;
            _Beta = _Wr * (_Zr / _Alfa) + _Zi;
        } else { // shouldn't overflow
            _Alfa = 0;
            _Beta = _Wr * _Zr - _Wi * _Zi; // Re(w * z)
        }

        _Ux = _Ctraits<_Ty>::asinh(_Beta);
        if (_Alfa != 0) {
            if (0 <= _Ux) {
                _Ux += _Ctraits<_Ty>::log(_Alfa);
            } else {
                _Ux -= _Ctraits<_Ty>::log(_Alfa); // asinh(a*b) = asinh(a)+log(b)
            }
        }

        bool _Neg = true;
        if (_Im < 0) {
            _Im = -_Im;
        } else {
            _Neg = false;
        }

        _Vx = 2 * _Ctraits<_Ty>::atan2(imag(sqrt(complex<_Ty>(_Re - 1, _Im))), _Zr);
        if (_Neg) {
            _Vx = -_Vx;
        }
    }
    return complex<_Ty>(_Ux, _Vx);
}

// FUNCTION TEMPLATE asinh
template <class _Ty>
_NODISCARD complex<_Ty> asinh(const complex<_Ty>& _Left) {
    const _Ty _Arcbig = static_cast<_Ty>(0.25) * _Ctraits<_Ty>::sqrt(_Ctraits<_Ty>::_Flt_max());
    constexpr _Ty _Pi = static_cast<_Ty>(3.1415926535897932384626433832795029L);

    const _Ty _Re = real(_Left);
    _Ty _Im       = imag(_Left);
    _Ty _Ux;
    _Ty _Vx;

    if (_Ctraits<_Ty>::_Isnan(_Re) || _Ctraits<_Ty>::_Isnan(_Im)) { // at least one NaN/Inf
        _Ux = _Ctraits<_Ty>::_Nanv();
        _Vx = _Ux;
    } else if (_Ctraits<_Ty>::_Isinf(_Re)) { // (+/-Inf, not NaN)
        _Ux = _Ctraits<_Ty>::_Infv();

        if (_Ctraits<_Ty>::_Isinf(_Im)) { // (+/-Inf, +/-Inf)
            _Ux = _Re;
            _Vx = _Im < 0 ? -static_cast<_Ty>(0.25) * _Pi : static_cast<_Ty>(0.25) * _Pi;
        } else { // (+/-Inf, finite)
            _Ux = _Re;
            _Vx = _Im < 0 ? -_Ty{0} : _Ty{0};
        }
    } else if (_Ctraits<_Ty>::_Isinf(_Im)) { // (finite, +/-Inf)
        _Ux = _Im;
        _Vx = _Im < 0 ? -static_cast<_Ty>(0.50) * _Pi : static_cast<_Ty>(0.50) * _Pi;
    } else { // (finite, finite)
        const complex<_Ty> _Wx = sqrt(complex<_Ty>(1 - _Im, _Re));
        const complex<_Ty> _Zx = sqrt(complex<_Ty>(1 + _Im, -_Re));
        const _Ty _Wr          = real(_Wx);
        const _Ty _Wi          = imag(_Wx);
        const _Ty _Zr          = real(_Zx);
        const _Ty _Zi          = imag(_Zx);
        _Ty _Alfa;
        _Ty _Beta;

        if (_Arcbig < _Wr) { // real parts large
            _Alfa = _Wr;
            _Beta = _Wi * (_Zr / _Alfa) - _Zi;
        } else if (_Arcbig < _Wi) { // imag parts large
            _Alfa = _Wi;
            _Beta = _Zr - _Wr * (_Zi / _Alfa);
        } else if (_Wi < -_Arcbig) { // imag part of w large negative
            _Alfa = -_Wi;
            _Beta = -_Zr - _Wr * (_Zi / _Alfa);
        } else { // shouldn't overflow
            _Alfa = 0;
            _Beta = _Wi * _Zr - _Wr * _Zi; // Im(w * conj(z))
        }

        _Ux = _Ctraits<_Ty>::asinh(_Beta);
        if (_Alfa != 0) {
            if (0 <= _Ux) {
                _Ux += _Ctraits<_Ty>::log(_Alfa);
            } else {
                _Ux -= _Ctraits<_Ty>::log(_Alfa); // asinh(a*b) = asinh(a)+log(b)
            }
        }

        _Vx = _Ctraits<_Ty>::atan2(_Im, real(_Wx * _Zx));
    }

    return complex<_Ty>(_Ux, _Vx);
}

// FUNCTION TEMPLATE asin
template <class _Ty>
_NODISCARD complex<_Ty> asin(const complex<_Ty>& _Left) {
    complex<_Ty> _Asinh = _STD asinh(complex<_Ty>(-imag(_Left), real(_Left)));

    return complex<_Ty>(imag(_Asinh), -real(_Asinh));
}

// FUNCTION TEMPLATE atanh
template <class _Ty>
_NODISCARD complex<_Ty> atanh(const complex<_Ty>& _Left) {
    const _Ty _Arcbig    = static_cast<_Ty>(0.25) * _Ctraits<_Ty>::sqrt(_Ctraits<_Ty>::_Flt_max());
    constexpr _Ty _Piby2 = static_cast<_Ty>(1.5707963267948966192313216916397514L);

    _Ty _Re = real(_Left);
    _Ty _Im = imag(_Left);
    _Ty _Ux;
    _Ty _Vx;

    if (_Ctraits<_Ty>::_Isnan(_Re) || _Ctraits<_Ty>::_Isnan(_Im)) { // at least one NaN
        _Ux = _Ctraits<_Ty>::_Nanv();
        _Vx = _Ux;
    } else if (_Ctraits<_Ty>::_Isinf(_Re)) { // (+/-Inf, not NaN)
        _Ux = _Re < 0 ? -_Ty{0} : _Ty{0};
        _Vx = _Im < 0 ? -_Piby2 : _Piby2;
    } else { // (finite, not _NaN)
        _Ty _Magim = _Im < 0 ? -_Im : _Im;
        bool _Neg  = _Re < 0;

        if (_Neg) {
            _Re = -_Re;
        } else {
            _Im = -_Im;
        }

        if (_Arcbig < _Re) { // |re| is large
            _Ty _Fx = _Im / _Re;

            _Ux = 1 / _Re / (1 + _Fx * _Fx);
            _Vx = _Im < 0 ? -_Piby2 : _Piby2;
        } else if (_Arcbig < _Magim) { // |im| is large
            _Ty _Fx = _Re / _Im;

            _Ux = _Fx / _Im / (1 + _Fx * _Fx);
            _Vx = _Im < 0 ? -_Piby2 : _Piby2;
        } else if (_Re != 1) { // |re| is small
            _Ty _Refrom1 = 1 - _Re;
            _Ty _Imeps2  = _Magim * _Magim;

            _Ux = static_cast<_Ty>(0.25) * _Ctraits<_Ty>::log1p(4 * _Re / (_Refrom1 * _Refrom1 + _Imeps2));
            _Vx = static_cast<_Ty>(0.50) * _Ctraits<_Ty>::atan2(2 * _Im, _Refrom1 * (1 + _Re) - _Imeps2);
        } else if (_Im == 0) { // {+/-1, 0)
            _Ux = _Ctraits<_Ty>::_Infv();
            _Vx = _Im;
        } else { // {+/-1, nonzero)
            _Ux = _Ctraits<_Ty>::log(
                _Ctraits<_Ty>::sqrt(_Ctraits<_Ty>::sqrt(4 + _Im * _Im)) / _Ctraits<_Ty>::sqrt(_Magim));
            _Vx = static_cast<_Ty>(0.50) * (_Piby2 + _Ctraits<_Ty>::atan2(_Magim, 2));
            if (_Im < 0) {
                _Vx = -_Vx;
            }
        }

        if (_Neg) {
            _Ux = -_Ux;
        } else {
            _Vx = -_Vx;
        }
    }
    return complex<_Ty>(_Ux, _Vx);
}

// FUNCTION TEMPLATE atan
template <class _Ty>
_NODISCARD complex<_Ty> atan(const complex<_Ty>& _Left) {
    complex<_Ty> _Atanh = _STD atanh(complex<_Ty>(-imag(_Left), real(_Left)));

    return complex<_Ty>(imag(_Atanh), -real(_Atanh));
}

// FUNCTION TEMPLATE cosh
template <class _Ty>
_NODISCARD complex<_Ty> cosh(const complex<_Ty>& _Left) {
    return complex<_Ty>(_Ctraits<_Ty>::_Cosh(real(_Left), _Ctraits<_Ty>::cos(imag(_Left))),
        _Ctraits<_Ty>::_Sinh(real(_Left), _Ctraits<_Ty>::sin(imag(_Left))));
}

// FUNCTION TEMPLATE exp
template <class _Ty>
_NODISCARD complex<_Ty> exp(const complex<_Ty>& _Left) {
    _Ty _Real(real(_Left)), _Imag(real(_Left));
    _Ctraits<_Ty>::_Exp(&_Real, _Ctraits<_Ty>::cos(imag(_Left)), 0);
    _Ctraits<_Ty>::_Exp(&_Imag, _Ctraits<_Ty>::sin(imag(_Left)), 0);
    return complex<_Ty>(_Real, _Imag);
}

// FUNCTION TEMPLATE _Fabs
template <class _Ty>
_Ty _Fabs(const complex<_Ty>& _Left, int* _Pexp) { // return magnitude and scale factor
    *_Pexp  = 0;
    _Ty _Av = real(_Left);
    _Ty _Bv = imag(_Left);

    if (_Ctraits<_Ty>::_Isinf(_Av) || _Ctraits<_Ty>::_Isinf(_Bv)) {
        return _Ctraits<_Ty>::_Infv(); // at least one component is INF
    } else if (_Ctraits<_Ty>::_Isnan(_Av)) {
        return _Av; // real component is NaN
    } else if (_Ctraits<_Ty>::_Isnan(_Bv)) {
        return _Bv; // imaginary component is NaN
    } else { // neither component is NaN or INF
        if (_Av < 0) {
            _Av = -_Av;
        }

        if (_Bv < 0) {
            _Bv = -_Bv;
        }

        if (_Av < _Bv) { // ensure that |_Bv| <= |_Av|
            _Ty _Tmp = _Av;
            _Av      = _Bv;
            _Bv      = _Tmp;
        }

        if (_Av == 0) {
            return _Av; // |0| == 0
        }

        if (1 <= _Av) {
            *_Pexp = 2;
            _Av    = _Av * static_cast<_Ty>(0.25);
            _Bv    = _Bv * static_cast<_Ty>(0.25);
        } else {
            *_Pexp = -2;
            _Av    = _Av * 4;
            _Bv    = _Bv * 4;
        }

        _Ty _Tmp = _Av - _Bv;
        if (_Tmp == _Av) {
            return _Av; // _Bv unimportant
        } else if (_Bv < _Tmp) { // use simple approximation
            const _Ty _Qv = _Av / _Bv;
            return _Av + _Bv / (_Qv + _Ctraits<_Ty>::sqrt(_Qv * _Qv + 1));
        } else { // use 1 1/2 precision to preserve bits
            constexpr _Ty _Root2            = static_cast<_Ty>(1.4142135623730950488016887242096981L);
            constexpr _Ty _Oneplusroot2high = static_cast<_Ty>(10125945.0 / 4194304.0); // exact if prec >= 24 bits
            constexpr _Ty _Oneplusroot2low  = static_cast<_Ty>(1.4341252375973918872420969807856967e-7L);

            const _Ty _Qv = _Tmp / _Bv;
            const _Ty _Rv = (_Qv + 2) * _Qv;
            const _Ty _Sv = _Rv / (_Root2 + _Ctraits<_Ty>::sqrt(_Rv + 2)) + _Oneplusroot2low + _Qv + _Oneplusroot2high;
            return _Av + _Bv / _Sv;
        }
    }
}

// FUNCTION TEMPLATE log
template <class _Ty>
_NODISCARD complex<_Ty> log(const complex<_Ty>& _Left) {
    _Ty _Theta = _Ctraits<_Ty>::atan2(imag(_Left), real(_Left)); // get phase

    if (_Ctraits<_Ty>::_Isnan(_Theta)) {
        return complex<_Ty>(_Theta, _Theta); // real or imag is NaN
    } else { // use 1 1/2 precision to preserve bits
        constexpr _Ty _Cm = static_cast<_Ty>(22713.0L / 32768.0L);
        constexpr _Ty _Cl = static_cast<_Ty>(1.4286068203094172321214581765680755e-6L);
        int _Leftexp;
        _Ty _Rho = _Fabs(_Left, &_Leftexp); // get magnitude and scale factor

        _Ty _Leftn = static_cast<_Ty>(_Leftexp);

        _Ty _Real;
        if (_Rho == 0) {
            _Real = -_Ctraits<_Ty>::_Infv(); // log(0) == -INF
        } else if (_Ctraits<_Ty>::_Isinf(_Rho)) {
            _Real = _Rho; // log(INF) == INF
        } else {
            _Real = _Ctraits<_Ty>::log(_Rho) + _Leftn * _Cl + _Leftn * _Cm;
        }

        return complex<_Ty>(_Real, _Theta);
    }
}

// FUNCTION TEMPLATE pow
template <class _Ty>
complex<_Ty> _Pow(const _Ty& _Left, const _Ty& _Right) {
    if (0 <= _Left) {
        return _Ctraits<_Ty>::pow(_Left, _Right);
    } else {
        return exp(_Right * log(complex<_Ty>(_Left)));
    }
}

template <class _Ty>
_NODISCARD complex<_Ty> pow(const complex<_Ty>& _Left, const _Ty& _Right) {
    if (imag(_Left) == 0) {
        return _Pow(real(_Left), _Right);
    } else {
        return exp(_Right * log(_Left));
    }
}

template <class _Ty>
_NODISCARD complex<_Ty> pow(const _Ty& _Left, const complex<_Ty>& _Right) {
    if (imag(_Right) == 0) {
        return _Pow(_Left, real(_Right));
    } else if (0 < _Left) {
        return exp(_Right * _Ctraits<_Ty>::log(_Left));
    } else {
        return exp(_Right * log(complex<_Ty>(_Left)));
    }
}

template <class _Ty>
_NODISCARD complex<_Ty> pow(const complex<_Ty>& _Left, const complex<_Ty>& _Right) {
    if (imag(_Left) == 0) {
        return pow(real(_Left), _Right);
    } else {
        return exp(_Right * log(_Left));
    }
}

// FUNCTION TEMPLATE sinh
template <class _Ty>
_NODISCARD complex<_Ty> sinh(const complex<_Ty>& _Left) {
    return complex<_Ty>(_Ctraits<_Ty>::_Sinh(real(_Left), _Ctraits<_Ty>::cos(imag(_Left))),
        _Ctraits<_Ty>::_Cosh(real(_Left), _Ctraits<_Ty>::sin(imag(_Left))));
}

// FUNCTION TEMPLATE sqrt
template <class _Ty>
_NODISCARD complex<_Ty> sqrt(const complex<_Ty>& _Left) {
    int _Leftexp;
    _Ty _Rho = _Fabs(_Left, &_Leftexp); // get magnitude and scale factor

    if (_Leftexp == 0) {
        return complex<_Ty>(_Rho, _Rho); // argument is zero, INF, or NaN
    } else { // compute in safest quadrant
        _Ty _Realmag = _Ctraits<_Ty>::ldexp(real(_Left) < 0 ? -real(_Left) : real(_Left), -_Leftexp);
        _Rho         = _Ctraits<_Ty>::ldexp(_Ctraits<_Ty>::sqrt(2 * (_Realmag + _Rho)), _Leftexp / 2 - 1);

        if (0 <= real(_Left)) {
            return complex<_Ty>(_Rho, imag(_Left) / (2 * _Rho));
        } else if (imag(_Left) < 0) {
            return complex<_Ty>(-imag(_Left) / (2 * _Rho), -_Rho);
        } else {
            return complex<_Ty>(imag(_Left) / (2 * _Rho), _Rho);
        }
    }
}

// FUNCTION TEMPLATE tanh
template <class _Ty>
_NODISCARD complex<_Ty> tanh(const complex<_Ty>& _Left) {
    _Ty _Tv = _Ctraits<_Ty>::tan(imag(_Left));
    _Ty _Sv = _Ctraits<_Ty>::_Sinh(real(_Left), _Ty{1});
    _Ty _Bv = _Sv * (_Ty{1} + _Tv * _Tv);
    _Ty _Dv = _Ty{1} + _Bv * _Sv;

    if (_Ctraits<_Ty>::_Isinf(_Dv)) {
        _Ty _Real;

        if (_Sv < _Ty{0}) {
            _Real = -_Ty{1};
        } else {
            _Real = _Ty{1};
        }

        return complex<_Ty>(_Real, _Tv * _Ty{0});
    }

    return complex<_Ty>((_Ctraits<_Ty>::sqrt(_Ty{1} + _Sv * _Sv)) * _Bv / _Dv, _Tv / _Dv);
}

// FUNCTION TEMPLATE arg
template <class _Ty>
_NODISCARD _Ty arg(const complex<_Ty>& _Left) { // return phase angle of complex as real
    return _Ctraits<_Ty>::atan2(imag(_Left), real(_Left));
}

// FUNCTION TEMPLATE conj
template <class _Ty>
_NODISCARD _CONSTEXPR20 complex<_Ty> conj(const complex<_Ty>& _Left) { // return complex conjugate
    return complex<_Ty>(real(_Left), -imag(_Left));
}

// FUNCTION TEMPLATE proj
template <class _Ty>
_NODISCARD complex<_Ty> proj(const complex<_Ty>& _Left) { // return complex projection
    if (_Ctraits<_Ty>::_Isinf(real(_Left)) || _Ctraits<_Ty>::_Isinf(imag(_Left))) {
        _Ty _Imag;
        if (imag(_Left) < 0) {
            _Imag = -_Ty{0};
        } else {
            _Imag = _Ty{0};
        }

        return complex<_Ty>(_Ctraits<_Ty>::_Infv(), _Imag);
    }

    return _Left;
}

// FUNCTION TEMPLATE cos
template <class _Ty>
_NODISCARD complex<_Ty> cos(const complex<_Ty>& _Left) {
    return complex<_Ty>(_Ctraits<_Ty>::_Cosh(imag(_Left), _Ctraits<_Ty>::cos(real(_Left))),
        -_Ctraits<_Ty>::_Sinh(imag(_Left), _Ctraits<_Ty>::sin(real(_Left))));
}

// FUNCTION TEMPLATE log10
template <class _Ty>
_NODISCARD complex<_Ty> log10(const complex<_Ty>& _Left) {
    return log(_Left) * static_cast<_Ty>(0.43429448190325182765112891891660508L);
}

// FUNCTION TEMPLATE norm
template <class _Ty>
_NODISCARD _CONSTEXPR20 _Ty norm(const complex<_Ty>& _Left) { // return squared magnitude
    return real(_Left) * real(_Left) + imag(_Left) * imag(_Left);
}

// FUNCTION TEMPLATE polar
template <class _Ty>
_NODISCARD complex<_Ty> polar(const _Ty& _Rho, const _Ty& _Theta) { // return _Rho * exp(i * _Theta) as complex
    return complex<_Ty>(_Rho * _Ctraits<_Ty>::cos(_Theta), _Rho * _Ctraits<_Ty>::sin(_Theta));
}

template <class _Ty>
_NODISCARD complex<_Ty> polar(const _Ty& _Rho) { // return _Rho * exp(i * 0) as complex
    return complex<_Ty>(_Rho, _Ty{0});
}

// FUNCTION TEMPLATE sin
template <class _Ty>
_NODISCARD complex<_Ty> sin(const complex<_Ty>& _Left) {
    return complex<_Ty>(_Ctraits<_Ty>::_Cosh(imag(_Left), _Ctraits<_Ty>::sin(real(_Left))),
        _Ctraits<_Ty>::_Sinh(imag(_Left), _Ctraits<_Ty>::cos(real(_Left))));
}

// FUNCTION TEMPLATE tan
template <class _Ty>
_NODISCARD complex<_Ty> tan(const complex<_Ty>& _Left) {
    complex<_Ty> _Zv(tanh(complex<_Ty>(-imag(_Left), real(_Left))));
    return complex<_Ty>(imag(_Zv), -real(_Zv));
}

// ADDITIONAL OVERLOADS
template <class _Ty>
using _Upgrade_to_double = conditional_t<is_integral_v<_Ty>, double, _Ty>;

template <class _Ty, enable_if_t<is_arithmetic_v<_Ty>, int> = 0>
_NODISCARD _Upgrade_to_double<_Ty> arg(_Ty _Left) {
    using _Upgraded = _Upgrade_to_double<_Ty>;
    const auto _Val = static_cast<_Upgraded>(_Left);

    return _Ctraits<_Upgraded>::atan2(0, _Val);
}

template <class _Ty, enable_if_t<is_arithmetic_v<_Ty>, int> = 0>
_NODISCARD _CONSTEXPR20 _Upgrade_to_double<_Ty> imag(_Ty) {
    return 0;
}

template <class _Ty, enable_if_t<is_arithmetic_v<_Ty>, int> = 0>
_NODISCARD _CONSTEXPR20 _Upgrade_to_double<_Ty> real(_Ty _Left) {
    using _Upgraded = _Upgrade_to_double<_Ty>;
    const auto _Val = static_cast<_Upgraded>(_Left);

    return _Val;
}

template <class _Ty, enable_if_t<is_arithmetic_v<_Ty>, int> = 0>
_NODISCARD _CONSTEXPR20 _Upgrade_to_double<_Ty> norm(_Ty _Left) {
    using _Upgraded = _Upgrade_to_double<_Ty>;
    const auto _Val = static_cast<_Upgraded>(_Left);

    return _Val * _Val;
}

template <class _Ty, enable_if_t<is_arithmetic_v<_Ty>, int> = 0>
_NODISCARD _CONSTEXPR20 complex<_Upgrade_to_double<_Ty>> conj(_Ty _Left) {
    using _Upgraded = _Upgrade_to_double<_Ty>;
    const auto _Val = static_cast<_Upgraded>(_Left);

    return complex<_Upgraded>(_Val, -_Upgraded{0});
}

template <class _Ty, enable_if_t<is_arithmetic_v<_Ty>, int> = 0>
_NODISCARD complex<_Upgrade_to_double<_Ty>> proj(_Ty _Left) {
    using _Upgraded = _Upgrade_to_double<_Ty>;
    const auto _Val = static_cast<_Upgraded>(_Left);

    if (_Ctraits<_Upgraded>::_Isinf(_Val)) {
        // C11 7.3.9.5/2: "z projects to z except that all complex infinities [...]
        // project to positive infinity on the real axis."
        return complex<_Upgraded>(_Ctraits<_Upgraded>::_Infv(), 0);
    }

    return complex<_Upgraded>(_Val, 0);
}

// FUNCTION TEMPLATE pow
template <class _Ty1, class _Ty2>
_NODISCARD complex<_Common_float_type_t<_Ty1, _Ty2>> pow(const complex<_Ty1>& _Left, const complex<_Ty2>& _Right) {
    using type = complex<_Common_float_type_t<_Ty1, _Ty2>>;
    return _STD pow(type(_Left), type(_Right));
}

template <class _Ty1, class _Ty2, enable_if_t<!is_integral_v<_Ty2>, int> = 0>
_NODISCARD complex<_Common_float_type_t<_Ty1, _Ty2>> pow(const complex<_Ty1>& _Left, const _Ty2& _Right) {
    using type = complex<_Common_float_type_t<_Ty1, _Ty2>>;
    return _STD pow(type(_Left), type(_Right));
}

template <class _Ty1, class _Ty2, enable_if_t<!is_integral_v<_Ty1> && is_integral_v<_Ty2>, int> = 0>
_NODISCARD complex<_Common_float_type_t<_Ty1, _Ty2>> pow(const complex<_Ty1>& _Left, const _Ty2 _Right) {
    using type = complex<_Common_float_type_t<_Ty1, _Ty2>>;

    type _Tmp   = _Left;
    auto _Count = static_cast<make_unsigned_t<_Ty2>>(_Right);

    if (_Right < 0) {
        _Count = 0 - _Count; // safe negation as unsigned
    }

    for (type _Zv(1);; _Tmp *= _Tmp) { // fold in _Left ^ (2 ^ _Count) as needed
        if ((_Count & 1) != 0) {
            _Zv *= _Tmp;
        }

        if ((_Count >>= 1) == 0) {
            return _Right < 0 ? type(1) / _Zv : _Zv;
        }
    }
}

template <class _Ty1, class _Ty2, enable_if_t<is_integral_v<_Ty1> && is_integral_v<_Ty2>, int> = 0>
_NODISCARD complex<_Ty1> pow(const complex<_Ty1>& _Left, _Ty2 _Right) {
    // raise Gaussian integer to an integer power
    using type = complex<_Ty1>;

    type _Ans = type(1, 0);

    if (_Right < 0) {
        _Ans = type(0, 0); // ignore 1/type(0, 0) error
    } else if (0 < _Right) { // raise to a positive power
        for (type _Factor = _Left;; _Factor *= _Factor) { // fold in _Left^(2^N))
            if ((_Right & 1) != 0) {
                _Ans *= _Factor;
            }

            if ((_Right >>= 1) == 0) {
                break;
            }
        }
    }
    return _Ans;
}

template <class _Ty1, class _Ty2>
_NODISCARD complex<_Common_float_type_t<_Ty1, _Ty2>> pow(const _Ty1& _Left, const complex<_Ty2>& _Right) {
    using type = complex<_Common_float_type_t<_Ty1, _Ty2>>;
    return _STD pow(type(_Left), type(_Right));
}

// FUNCTION TEMPLATE operator>>
template <class _Ty, class _Elem, class _Tr>
basic_istream<_Elem, _Tr>& operator>>(basic_istream<_Elem, _Tr>& _Istr, complex<_Ty>& _Right) {
    const ctype<_Elem>& _Ctype_fac = _STD use_facet<ctype<_Elem>>(_Istr.getloc());
    _Elem _Ch                      = 0;
    long double _Real              = 0;
    long double _Imag              = 0;

    if (_Istr >> _Ch && _Ch != _Ctype_fac.widen('(')) { // no leading '(', treat as real only
        _Istr.putback(_Ch);
        _Istr >> _Real;
        _Imag = 0;
    } else if (_Istr >> _Real >> _Ch && _Ch != _Ctype_fac.widen(',')) {
        if (_Ch == _Ctype_fac.widen(')')) {
            _Imag = 0; // (real)
        } else { // no trailing ')' after real, treat as bad field
            _Istr.putback(_Ch);
            _Istr.setstate(ios_base::failbit);
        }
    } else if (_Istr >> _Imag >> _Ch && _Ch != _Ctype_fac.widen(')')) { // no imag or trailing ')', treat as bad field
        _Istr.putback(_Ch);
        _Istr.setstate(ios_base::failbit);
    }

    if (!_Istr.fail()) { // store valid result
        _Ty _Tyreal(static_cast<_Ty>(_Real)), _Tyimag(static_cast<_Ty>(_Imag));
        _Right = complex<_Ty>(_Tyreal, _Tyimag);
    }
    return _Istr;
}

// FUNCTION TEMPLATE operator<<
template <class _Ty, class _Elem, class _Tr>
basic_ostream<_Elem, _Tr>& operator<<(basic_ostream<_Elem, _Tr>& _Ostr, const complex<_Ty>& _Right) {
    const ctype<_Elem>& _Ctype_fac = _STD use_facet<ctype<_Elem>>(_Ostr.getloc());
    basic_ostringstream<_Elem, _Tr, allocator<_Elem>> _Sstr;

    _Sstr.flags(_Ostr.flags());
    _Sstr.imbue(_Ostr.getloc());
    _Sstr.precision(_Ostr.precision());
    _Sstr << _Ctype_fac.widen('(') << real(_Right) << _Ctype_fac.widen(',') << imag(_Right) << _Ctype_fac.widen(')');

    return _Ostr << _Sstr.str();
}

// complex LITERALS
inline namespace literals {
    inline namespace complex_literals {
        _NODISCARD constexpr complex<long double> operator""il(long double _Val) {
            return complex<long double>(0.0L, static_cast<long double>(_Val));
        }

        _NODISCARD constexpr complex<long double> operator""il(unsigned long long _Val) {
            return complex<long double>(0.0L, static_cast<long double>(_Val));
        }

        _NODISCARD constexpr complex<double> operator""i(long double _Val) {
            return complex<double>(0.0, static_cast<double>(_Val));
        }

        _NODISCARD constexpr complex<double> operator""i(unsigned long long _Val) {
            return complex<double>(0.0, static_cast<double>(_Val));
        }

        _NODISCARD constexpr complex<float> operator""if(long double _Val) {
            return complex<float>(0.0f, static_cast<float>(_Val));
        }

        _NODISCARD constexpr complex<float> operator""if(unsigned long long _Val) {
            return complex<float>(0.0f, static_cast<float>(_Val));
        }
    } // namespace complex_literals
} // namespace literals

_STD_END

#undef _RE
#undef _IM

#pragma pop_macro("new")
_STL_RESTORE_CLANG_WARNINGS
#pragma warning(pop)
#pragma pack(pop)
#endif // _STL_COMPILER_PREPROCESSOR
#endif // _COMPLEX_
