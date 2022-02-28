#pragma once

#include <Shlwapi.h>

template <int VERS = 0>
struct http_utils_t {
	struct httpget_t
	{
		HRESULT hr, hrCI;
		IXMLHttpRequest* httprq;
		VARIANT vres;
		BSTR bres;
		long status;


		static VARIANT no_param()
		{
			VARIANT vo = { VT_ERROR };
			vo.scode = DISP_E_PARAMNOTFOUND;
			return vo;
		}

		static 	bool loop(HWND hwnd = 0)
		{
			MSG msg;
			while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
			{
				if ((msg.message == WM_QUIT)) return true;

				TranslateMessage(&msg);
				DispatchMessage(&msg);

			}
			return false;
		};



		httpget_t() :httprq(0), bres(0), status(0), hrCI(E_FAIL) {}

		HRESULT init(const wchar_t* url, const wchar_t* user = L"", const wchar_t* psw = L"", const wchar_t*method = L"GET")
		{
			vres.vt = VT_EMPTY;
			hrCI = CoInitializeEx(0, COINIT_APARTMENTTHREADED);// COINIT_MULTITHREADED);
															   //hrCI=CoInitializeEx(0,COINIT_MULTITHREADED);

			CLSID clsid;
			hr = CLSIDFromProgID(L"Msxml2.XMLHTTP", &clsid);
			if (FAILED(hr)) return hr;
			//hr=CoCreateInstance(clsid,NULL,CLSCTX_ALL,__uuidof(IXMLHttpRequest),(void**)&httprq);
			loop();
			hr = CoCreateInstance(__uuidof(XMLHTTPRequest), NULL, CLSCTX_ALL, __uuidof(IXMLHttpRequest), (void**)&httprq);
			loop();
			if (FAILED(hr)) return hr;

			VARIANT vo = { VT_NULL }, vb = { VT_BOOL };
			vo.scode = DISP_E_PARAMNOTFOUND;
			vb.boolVal = VARIANT_TRUE;

			//bstr_t bsc=L"GET";
			bstr_t bsc = method;
			bstr_t burl;

			if ((url[0] == L'f') || (url[0] == L'F'))
				burl = url;
			else
				burl = v_buf<wchar_t>().printf(L"%s?c11=%d", url, GetTickCount());

			variant_t vu = user ? user : L"";
			variant_t vp = psw ? psw : L"";;
			hr = httprq->open(bsc, burl, vb, vu, vp);
			loop();
			if (FAILED(hr)) return hr;
			//hr=httprq->setRequestHeader(bstr_t(L"Cache-Control"),bstr_t(L"no-cache"));
			//hr=httprq->setRequestHeader(bstr_t(L"If-Modified-Since"), bstr_t(L"Sat, 1 Jan 2000 00:00:00 GMT"));
			//if(FAILED(hr)) return ;
			//SysFreeString(burl);
			//SysFreeString(bsc);

			//hr=httprq->setRequestHeader(bstr_t(L"Accept-Encoding"),bstr_t(L""));
			//loop();
			//if(FAILED(hr)) return ;
			//hr=httprq->open(bstr_t(L"GET"),bstr_t(url),vb,vo,vo);
			loop();
			hr = httprq->send(vo);
			loop();

			//hr = httprq->get_status(&status);
			return hr;

		}

		inline operator bool() {
			return S_OK == hr;
		}

		inline long abort() {
			HRESULT hr0 = hr;
			if (!hr0) {
				hr0 = httprq->abort();
			}
			return hr0;
		}
		inline long get_state() {
			long st = -1;
			HRESULT hr0;
			if (!hr) {
				loop();
				if (hr0 = httprq->get_readyState(&st))
					return -1;
			}
			return st;
		}
		inline long get_status() {
			long st = -1;
			HRESULT hr0;
			if (!hr) {
				loop();
				if (hr0 = httprq->get_status(&st))
					return -1;
			}
			return st;
		}
		~httpget_t()
		{
			SysFreeString(bres);
			if (vres.vt&VT_ARRAY) SafeArrayUnlock(vres.parray);
			VariantClear(&vres);
			if (httprq) httprq->Release();

			if (SUCCEEDED(hrCI)) CoUninitialize();
		}

		inline 	operator wchar_t*()
		{
			HRESULT hrl;
			if (FAILED(hr)) return 0;
			SysFreeString(bres);
			hrl = httprq->get_responseText(&bres);
			if (SUCCEEDED(hrl)) return bres;
			else return 0;
		}

		template <class F>
		HRESULT	get_buffer(F** pbuf, int* pcb)
		{
			HRESULT hrl;
			if (FAILED(hr)) return hr;

			hr = httprq->get_status(&status);
			if (FAILED(hr) || ((status != 200) && (status != 0))) return -1;
			VariantClear(&vres);
			hrl = httprq->get_responseBody(&vres);
			if (FAILED(hrl)) return hrl;
			long ii = 0;
			hrl = SafeArrayLock(vres.parray);
			if (FAILED(hrl)) return hrl;
			hrl = SafeArrayPtrOfIndex(vres.parray, &ii, (void**)pbuf);
			if (FAILED(hrl)) return hrl;
			*pcb = vres.parray->rgsabound[0].cElements;
			return hrl;
		}

		inline static int save_to_file(const wchar_t* fn, char* buf, int cb) {
			int cbw;
			FILE *hf = _wfopen(fn, L"wb");
			if (!hf) return -1;
			cbw = fwrite(buf, 1, cb, hf);
			fclose(hf);
			return cbw;
		};

		inline int save_to_file(const wchar_t* fn)
		{
			int cb, cbw;
			char* buf = 0;
			if (FAILED(get_buffer(&buf, &cb))) return -1;
			cbw = save_to_file(fn, buf, cb);
			return cbw;
			/*
			FILE *hf=_wfopen(fn,L"wb");
			if(!hf) return -1;
			cbw=fwrite(buf,1,cb,hf);
			fclose(hf);
			return cbw;
			*/
		}


	};

	struct url_us_pw_t {
		wchar_t* url;
		wchar_t* usr;
		wchar_t* pw;
	};


	struct http_group_t {


		std::vector< httpget_t> gets;
		std::vector< long> statuses;
		int n, index;
		bool fbad_url, fbad_host;

		http_group_t(url_us_pw_t* urls = 0) :fbad_url(false), fbad_host(false), index(-1) {
			if (urls)
				init(urls);
		}

		void abort() {
			if (gets.size()) {
				for (int i = 0; i < gets.size(); i++)
					gets[i].abort();
				gets.clear();
			}
		}

		void init(url_us_pw_t* urls) {
			index = -1;
			n = 0;
			abort();
			while (urls[n].url) n++;
			gets.resize(n);
			statuses.resize(n);
			for (int i = 0; i < n; i++)
			{
				url_us_pw_t& u = urls[i];
				gets[i].init(u.url, u.usr, u.pw);
			}
		}

		template <class Functor>
		int load(Functor& f, double tio = 10000) {
			int st = -1;

			ULONGLONG dt, t0 = GetTickCount64();

			fbad_url = fbad_host = false;

			while (1) {

				//st
				bool ftio = tio>=0;
				bool fbreak;



				for (int i = 0; i < n; i++) {
					httpget_t& g = gets[i];
					int r = g.get_state();


					fbreak = r == 4;



					if (fbreak) {
						if (st = f(g))
						{
							index = i;
							goto exts;
						}

					}

				}
				dt = GetTickCount64() - t0;
				bool fhostok = false, furlok = false;
				for (int i = 0; i < n; i++) {
					httpget_t& g = gets[i];
					int r = g.get_state();
					//ftio |= (r < 3);
					//ftio = true;
					if (r == 4) {
						int status;
						status = statuses[i] = g.get_status();
						status /= 100;
						if (!status)
						{
							fhostok = true;
							furlok = true;
						}
						else if (status == 4) {
							fhostok = fhostok || true;
						}
						else if (status == 12)
						{
							furlok = furlok || true;
						}
					}
				}

				fbad_host = !fhostok;
				fbad_url = !furlok;

				if (ftio && (dt >= tio)&&(tio>=0)) {
					st = 0;
					goto exts;
				}

				httpget_t::loop();
				SleepEx(100, TRUE);
				httpget_t::loop();
			}
		exts:

			abort();

			return  st;
		}

	};
	/*
	struct http_group_once_t :http_group_t {
	std::vector< long> statuses;
	httpget_t* current;
	http_group_once_t(url_us_pw_t* urls = 0) :status(0),http_group_t(urls), current(NULL){

	statuses.resize(n);

	}

	httpget_t*  load( DWORD tio = 20000) {
	current = NULL;
	int st;
	st=load([&](httpget_t& g) {
	if (g.get_status() == 200)
	{
	current = &g;
	return 200;
	}
	return 0;
	}
	, tio);
	if (st == 200) {
	abort();
	}
	else
	for (int i = 0; i < n; i++)
	statuses[i] = gets[i].get_status();


	return current;
	}

	};
	*/

	struct dll_downloader_t {

		HMODULE hlib;
		HRESULT hr;
		http_group_t  group;
		double tio;
		const  wchar_t * dest;

		inline bool fbad_url() {
			return group.fbad_url;
		}
		inline bool fbad_host() {
			return group.fbad_host;
		}
		dll_downloader_t(url_us_pw_t*purl, const wchar_t * pdest, double _tio = 35000)
			:group(purl), dest(pdest), hlib(0), tio(_tio) {}

		bool operator()(httpget_t& g) {
			hr = 0;
			int sts = g.get_status();
			if ((sts == 200) || (sts == 0)) {
				UINT u = GetErrorMode();
				if (g.save_to_file(dest) > 0) {
					SetErrorMode(0x8007);
					if (!(hlib = LoadLibraryW(dest))) {
						hr = GetLastError();
					}
					SetErrorMode(u);
					return hlib;

				}

			}

			return false;
		}
		HMODULE load() {
			hlib = 0;
			group.load(*this, tio);
			return hlib;
		}
	};


};
/*
typedef http_utils_t<>::url_us_pw_t url_us_pw_t;
typedef http_utils_t<>::http_group_t http_group_t;
typedef http_utils_t<>::httpget_t httpget_t;
*/
