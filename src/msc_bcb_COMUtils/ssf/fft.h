#pragma once

#include "mkl.h"
#include "mkl_dfti.h"
#include <complex>
#include <algorithm>
#include "utils.h"

typedef std::complex<double> complex_t;

template <int _V = 0>
struct FFT_t {
    FFT_t(size_t _N) :N(_N), hfft(nullptr) {
        (st = DftiCreateDescriptor(&hfft, DFTI_DOUBLE, DFTI_COMPLEX, 1, (MKL_LONG)N)) || (st = DftiCommitDescriptor(hfft));
        sbN = double(1) / std::sqrt(double(N));
    }
    ~FFT_t() {
        DftiFreeDescriptor(&hfft);
    }

    inline long forward(complex_t* x) {
        return st || (st = DftiComputeForward(hfft, x));
    }

    inline  long backward(complex_t* x) {

        return st || (st = DftiComputeBackward(hfft, x));
    }

    inline complex_t* renorm(complex_t* x, double s) {

        std::for_each(x, x + N, [&](complex_t& v) {
            v *= s;
            });
        return x;
    }
    complex_t* operator()(complex_t* x) {
        st = forward(renorm(x, sbN));
        return x;
    }
    complex_t* i(complex_t* x) {
        st = backward(renorm(x, sbN));
        return x;
    }
    long st;
    size_t N;
    double sbN;
    DFTI_DESCRIPTOR_HANDLE hfft;
};
