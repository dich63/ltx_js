#pragma once
//csv_io.h
#include <stdio.h>
#include <complex>
#include <list>

template <class Pool>
struct csv_XY_t {

	struct buf_t {
		char buf[256];
	};

	typedef Pool pool_t;	


	static bool scan_line(buf_t& line  , std::complex<double>* x, std::complex<double>* y) {

		constexpr std::complex<double> j = { 0.0,1.0 };
		double a, b, c, d;
		char buf[256];

		{
			const char* ps = line.buf;
			char* pd = buf;
			while (*ps) {
				if (!isspace(*ps)) {
					*(pd++) = *ps;
				}
				++ps;
			};
			*pd = '\0';

		}


		int n = sscanf(buf, "%lf%lfi,%lf%lfi", &a, &b, &c, &d);

		if (n == 4) {
			*x = a + j * b;
			*y = c + j * d;		
		}	 
			
		
		return (n == 4);
	};


	csv_XY_t(pool_t& _pool) :pool(_pool) {};


	size_t save(FILE* hf,const char* prefix="dSigX, dSigY") {
		
		char buf[256];

		if (prefix) {
			fprintf(hf,"%s\n", prefix);
		}

		for (auto n = 0; n < N; ++n) {

			const char* sx,*sy;
			auto xr = X[n].real();
			auto xi = X[n].imag();
			auto yr = Y[n].real();
			auto yi = Y[n].imag();

			if (xi > 0) {
				sx = "+";
			}
			else {
				xi = -xi;
				sx = "-";
			}

			if (yi > 0) {
				sy = "+";
			}
			else {
				yi = -yi;
				sy = "-";
			}

			fprintf(hf," %.15Lg%s%.15gi , %.15g%s%.15gi\n",xr,sx,xi,yr,sy,yi);

		}
	
		return N;
	}

	size_t load(FILE *hf){

		std::list<buf_t> bufs;
		buf_t b;

		//fgets(b.buf, 256, hf);
		while (fgets(b.buf, 256, hf)) {
			bufs.push_back(b);
		}
		size_t NL = bufs.size();
		size_t nr=0;

		pool.alloc(NL, &X);
		pool.alloc(NL, &Y);

		std::complex<double> *x = X, *y = Y;

		
		for (auto it = bufs.begin(); it != bufs.end();++it) {

			if (scan_line(*it, &x[nr], &y[nr])) {
				nr++;
			}
		}

		return N=nr;
	}

	size_t load(const wchar_t *fn) {
		
		FILE* hf = _wfopen(fn, L"r");
		size_t n=load(hf);
		fclose(hf);
		return n;
	}
	size_t load(const char* fn) {

		FILE* hf = fopen(fn, "r");
		size_t n = load(hf);
		fclose(hf);
		return n;
	}


	size_t save(const wchar_t* fn) {

		FILE* hf = _wfopen(fn, L"w");
		size_t n = save(hf);
		fclose(hf);
		return n;
	}


	size_t save(const char* fn) {

		FILE* hf = fopen(fn, "w");
		size_t n = save(hf);
		fclose(hf);
		return n;
	}




	size_t N;
	std::complex<double>* X, * Y;

	pool_t& pool;
};