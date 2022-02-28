#pragma once

#include <string.h>


namespace utils {

	template <class CH,bool fcasesens = true>
	struct case_t
	{
		const CH* s;
		int len;

		inline static int safe_len(const char* p){
			return (p) ? strlen(p)  : 0;
		}


		inline static int safe_len(const wchar_t* p)
		{
			
			return (p) ? wcslen(p)  : 0;
		}


		case_t(const CH* _s) :s(_s), len(safe_len(s)) {};
		case_t(const CH* _s, int l) :s(_s), len(l) {};

		inline static int str_cmp_ni(const wchar_t* p, const wchar_t* ps, int cb)
		{
			if (p == ps) return 0;
			if (p == 0) return -1;
			if (ps == 0) return 1;
			return _wcsnicmp(p, ps, cb);
		}

		inline  static int str_cmp_ni(const char* p, const char* ps, int cb)
		{
			if (p == ps) return 0;
			if (p == 0) return -1;
			if (ps == 0) return 1;
			return _strnicmp(p, ps, cb);
		}


		inline static int str_cmp_n(const wchar_t* p, const wchar_t* ps, int cb)
		{
			if (p == ps) return 0;
			if (p == 0) return -1;
			if (ps == 0) return 1;
			return wcsncmp(p, ps, cb);
		}

		inline  static int str_cmp_n(const char* p, const char* ps, int cb)
		{
			if (p == ps) return 0;
			if (p == 0) return -1;
			if (ps == 0) return 1;
			return strncmp(p, ps, cb);
		}



		bool eqlen(const CH* p, int l)
		{
			int i = (fcasesens) ? str_cmp_n(s, p, l) : str_cmp_ni(s, p, l);
			return i == 0;

		}

		bool eq(const CH* p)
		{
			return (len==safe_len(p))&& eqlen(p, len);

		}

		bool eqlens(const CH* p, int l) {

			(l == -1) && (l = safe_len(p));
				return eqlen(p, l);
		}

		bool operator()(int l, const CH* s1)
		{			
			return eqlens(s1, l);
		}

		bool operator()(int l, const CH* s1, const CH* s2)
		{
			return eqlens(s1, l)|| eqlens(s2, l);
		}

		bool operator()(int l, const CH* s1, const CH* s2, const CH* s3)
		{
			return eqlens(s1, l) || eqlens(s2, l) || eqlens(s3, l);
		}


		bool operator()(const CH* s1)
		{
			return eq(s1);
		}

		bool operator()(const CH* s1, const CH* s2)
		{
			return eq(s1) || eq(s2);
		}
		bool operator()(const CH* s1, const CH* s2, const CH* s3)
		{
			return eq(s1) || eq(s2) || eq(s3);
		}

		bool operator()(const CH* s1, const CH* s2, const CH* s3, const CH* s4)
		{
			return eq(s1) || eq(s2) || eq(s3) || eq(s4);
		}
		bool operator()(const CH* s1, const CH* s2, const CH* s3, const CH* s4, const CH* s5)
		{
			return eq(s1) || eq(s2) || eq(s3) || eq(s4) || eq(s5);
		}
		bool operator()(const CH* s1, const CH* s2, const CH* s3, const CH* s4, const CH* s5, const CH* s6)
		{
			return eq(s1) || eq(s2) || eq(s3) || eq(s4) || eq(s5) || eq(s6);
		}
		bool operator()(const CH* s1, const CH* s2, const CH* s3, const CH* s4, const CH* s5, const CH* s6, const CH* s7)
		{
			return eq(s1) || eq(s2) || eq(s3) || eq(s4) || eq(s5) || eq(s6) || eq(s7);
		}

		bool operator()(const CH* s1, const CH* s2, const CH* s3, const CH* s4, const CH* s5, const CH* s6, const CH* s7, const CH* s8)
		{
			return eq(s1) || eq(s2) || eq(s3) || eq(s4) || eq(s5) || eq(s6) || eq(s7) || eq(s8);
		}

	};
} // end - namespace utils