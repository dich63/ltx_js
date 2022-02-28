#pragma once
// #include "moniker_parsers.h"
#include "ltx_js.h"
#include "ltx_call_helper.h"
#include "dispatch_call_helpers.h"
#include "wchar_parsers.h"
#include "ipc_utils.h"
#include "Shlwapi.h"
#include "buf_fm.h"

#define  bind_args_update(parser,vn,typv) { (vn)=parser.bind_args(L###vn).def<typv>((vn));}
#define  bind_args_update_hexbin(parser,vn,typv) { (vn)=parser.bind_args(L###vn,true).def<typv>((vn));}
template <class CH=wchar_t>
struct moniker_parser_t
{


	typedef std::basic_string<CH, std::char_traits<CH>,std::allocator<CH> > tstring;
	typedef  unsigned int uint32_t;

	typedef char char_t;
	typedef ipc_utils::result_t<variant_t> arg_result_t;





	enum ch:CH{
		zerro=0
	};

	

	struct variant_holder_t
	{
		variant_t v;		
	};

	struct handle_holder_t{
		HANDLE h;
		handle_holder_t():h(NULL){};
		~handle_holder_t(){
			if(h&&(h!=INVALID_HANDLE_VALUE))
				::CloseHandle(h);
		}

	};



	struct bindObject_t:invoker_base_t<bindObject_t,variant_t,HRESULT,VARIANT>
	{
       const wchar_t* moniker_str_;
	   REFIID riid_;
	   void** ppObj_;	   
	   bindObject_t(const wchar_t* moniker_str,REFIID riid,void** ppObj)
		   :moniker_str_(moniker_str),riid_(riid),ppObj_(ppObj){};

	   inline 	HRESULT invoke_proc(int argc,argh_t* argv){
		   HRESULT hr;
		   return hr=bindObjectWithArgsEx(moniker_str_,riid_,ppObj_,true,argc,argv);
	   } 

	   inline operator HRESULT(){ return invoke_proc(0,0); };

		//bindObjectWithArgsEx(const wchar_t* moniker_str,REFIID riid,void** ppObj,bool freverse=true,int argc=0,VARIANT* argv=0);
	};


	static bindObject_t bindObject(const wchar_t* moniker_str,REFIID riid,void** ppObj)
	{
		return bindObject_t(moniker_str,riid,ppObj);
	}

	template <class I>
	static bindObject_t bindObject(const wchar_t* moniker_str, I** ppObj)
	{
		return bindObject_t(moniker_str, __uuidof(I), (void**)ppObj);
	}
/*

	struct cmpi_t{
		cmpi_t(const wchar_t* pstr):p(pstr){}
		inline		bool operator()(const CH* s)
		{
			return safe_strstri(p,s);
		}
		const CH* p;
	};

	struct cmp_t{
		cmp_t(const wchar_t* pstr):p(pstr){}
		inline		bool operator()(const CH* s)
		{
			return safe_strstr(p,s);
		}
		const CH* p;
	};

*/

	inline static bool _spin(volatile LONG* pl,int c1,int toMs)
	{
		for(int c=0;c<c1;c++)
		{
			if(InterlockedCompareExchange(pl,0,0))
				return true;
			Sleep(toMs);
		}
		return false;
	};


	inline static bstr_t bstr_alloc(size_t cb,wchar_t* pstr=NULL)
	{

		return bstr_t(::SysAllocStringLen(pstr,cb),false);
	}

	template <int WOW>
	inline static bstr_t reg_value_ex(HKEY hk,_bstr_t value, bstr_t subkey,bstr_t dfltval)
	{
		//KEY_WOW64_64KEY //KEY_WOW64_32KEY

		DWORD keyWOW=0;
		if(WOW==32) keyWOW=KEY_WOW64_32KEY;
		else if(WOW==64) keyWOW=KEY_WOW64_64KEY;

		HKEY the_hk=NULL;
		HRESULT hr;

		if(subkey.length()!=0)
		{		 
			if(ERROR_SUCCESS!=(hr=RegOpenKeyExW(hk, subkey,0,KEY_QUERY_VALUE|keyWOW,&the_hk)) )
		 {
			 return  (wchar_t*)dfltval;
		 }


			hk=the_hk;
		} 
		unsigned long cb=2048,ct=0;
		bstr_t res=moniker_parser_t<>::bstr_alloc(cb);
		wchar_t* buf=res; 
		cb*=sizeof(wchar_t); 
		if(ERROR_SUCCESS!=RegQueryValueExW(hk,value,NULL,&ct,(LPBYTE)buf,&cb))
			buf=dfltval;

		if(the_hk) RegCloseKey(the_hk);

		return buf;

	};



	inline static int DecodeBase64Char(unsigned int ch) 
	{
		// returns -1 if the character is invalid
		// or should be skipped
		// otherwise, returns the 6-bit code for the character
		// from the encoding table
		if (ch >= 'A' && ch <= 'Z')
			return ch - 'A' + 0;	// 0 range starts at 'A'
		if (ch >= 'a' && ch <= 'z')
			return ch - 'a' + 26;	// 26 range starts at 'a'
		if (ch >= '0' && ch <= '9')
			return ch - '0' + 52;	// 52 range starts at '0'
		if (ch == '+')
			return 62;
		if (ch == '/')
			return 63;
		return -1;
	}

	template <class CC>
	inline static BOOL Base64Decode(const CC* szSrc, int nSrcLen, BYTE *pbDest, int *pnDestLen) 
	{
		if(!nSrcLen) 
			return TRUE;

		if (szSrc == NULL || pnDestLen == NULL)
		{
			return FALSE;
		}

		const CC* szSrcEnd = szSrc + nSrcLen;
		int nWritten = 0;

		BOOL bOverflow = (pbDest == NULL) ? TRUE : FALSE;

		while (szSrc < szSrcEnd &&(*szSrc) != 0)
		{
			DWORD dwCurr = 0;
			int i;
			int nBits = 0;
			for (i=0; i<4; i++)
			{
				if (szSrc >= szSrcEnd)
					break;
				int nCh = DecodeBase64Char(*szSrc);
				szSrc++;
				if (nCh == -1)
				{
					// skip this char
					i--;
					continue;
				}
				dwCurr <<= 6;
				dwCurr |= nCh;
				nBits += 6;
			}

			if(!bOverflow && nWritten + (nBits/8) > (*pnDestLen))
				bOverflow = TRUE;

			// dwCurr has the 3 bytes to write to the output buffer
			// left to right
			dwCurr <<= 24-nBits;
			for (i=0; i<nBits/8; i++)
			{
				if(!bOverflow)
				{
					*pbDest = (BYTE) ((dwCurr & 0x00ff0000) >> 16);
					pbDest++;
				}
				dwCurr <<= 8;
				nWritten++;
			}

		}

		*pnDestLen = nWritten;

		if(bOverflow)
		{
			return FALSE;
		}

		return TRUE;
	}


	template <class T,class CC>
	inline static BOOL base64decode_to(T& o,const CC* szSrc, int nSrcLen = -1) {

		int dlen = sizeof(o);
		BYTE * pd = (BYTE *)&o;
		
		(nSrcLen == -1) && (nSrcLen = safe_len(szSrc));

		::memset(pd, 0, dlen);
		return Base64Decode(szSrc, nSrcLen, pd, &dlen);		

	}


	template <class CH>
	inline static size_t Base64DecodeLength(const CH* szSrc, size_t nSrcLen=-1) 
	{
		(nSrcLen==-1)&&(nSrcLen=safe_len(szSrc));

		if (szSrc == NULL )
		{
			return 0;
		}

		const CH* szSrcEnd = szSrc + nSrcLen;
		size_t nWritten = 0;

		

		while (szSrc < szSrcEnd &&(*szSrc) != 0)
		{
			DWORD dwCurr = 0;
			int i;
			int nBits = 0;
			for (i=0; i<4; i++)
			{
				if (szSrc >= szSrcEnd)
					break;
				int nCh = DecodeBase64Char(*szSrc);
				szSrc++;
				if (nCh == -1)
				{
					// skip this char
					i--;
					continue;
				}
				dwCurr <<= 6;
				dwCurr |= nCh;
				nBits += 6;
			}

			
			// dwCurr has the 3 bytes to write to the output buffer
			// left to right
			dwCurr <<= 24-nBits;
			for (i=0; i<nBits/8; i++)
			{
			
				dwCurr <<= 8;
				nWritten++;
			}

		}

		

		return nWritten;
	}




	static const char* base64_encoding_table()
	{
		static const char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
			'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
			'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
			'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
			'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
			'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
			'w', 'x', 'y', 'z', '0', '1', '2', '3',
			'4', '5', '6', '7', '8', '9', '+', '/'};
		return encoding_table;

	};

	static const wchar_t* base64_encoding_tableW()
	{
		static const wchar_t encoding_table[] = {L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H',
			L'I', L'J', L'K', L'L', L'M', L'N', L'O', L'P',
			L'Q', L'R', L'S', L'T', L'U', L'V', L'W', L'X',
			L'Y', L'Z', L'a', L'b', L'c', L'd', L'e', L'f',
			L'g', L'h', L'i', L'j', L'k', L'l', L'm', L'n',
			L'o', L'p', L'q', L'r', L's', L't', L'u', L'v',
			L'w', L'x', L'y', L'z', L'0', L'1', L'2', L'3',
			L'4', L'5', L'6', L'7', L'8', L'9', L'+', L'/'};
		return encoding_table;

	};


	static   bstr_t base64_encodeW(const unsigned char *data,
		size_t input_length,
		size_t *poutput_length=0,const unsigned char *prefix=0) { 	

			const wchar_t* encoding_table=base64_encoding_tableW();
			size_t tmp;
			size_t &output_length=(poutput_length)? *poutput_length:tmp;

			int mod_table[] = {0, 2, 1};

			output_length = 4 * ((input_length + 2) / 3);

			
            int ladd=safe_len((char *)prefix);

			bstr_t bencoded_data( SysAllocStringLen(0,output_length+ladd),false);
			wchar_t* pw=bencoded_data;
			if(ladd)
			{	
                
			  char_mutator<CP_UTF8> cm((char *)prefix);
			  memcpy(pw,(wchar_t*)cm,2*ladd);

			}
			wchar_t* encoded_data=pw+ladd;
			 

			if (encoded_data == NULL) return bstr_t();

			for (int i = 0, j = 0; i < input_length;) {

				uint32_t octet_a = i < input_length ? data[i++] : 0;
				uint32_t octet_b = i < input_length ? data[i++] : 0;
				uint32_t octet_c = i < input_length ? data[i++] : 0;

				uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

				encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
				encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
				encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
				encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
			}

			for (int i = 0; i < mod_table[input_length % 3]; i++)
				encoded_data[output_length - 1 - i] = L'=';
			
			
			return bencoded_data;

	};

	static   HRESULT base64_encodeV(VARIANT* pV,const unsigned char *data,
		size_t input_length,
		size_t *poutput_length = 0, const unsigned char *prefix = 0) {
		OLE_CHECK_PTR(pV);
		VARIANT v = { VT_BSTR };
		v.bstrVal = base64_encodeW(data, input_length, poutput_length, prefix).Detach();
		*pV = v;
		return S_OK;
	}

	static   bstr_t base64_encode(const unsigned char *data,
		size_t input_length,
		size_t *output_length=0) { 



			const char* encoding_table=base64_encoding_table();
			size_t tmp;
			if(!output_length) output_length=&tmp;

			int mod_table[] = {0, 2, 1};

			*output_length = 4 * ((input_length + 2) / 3);

			v_buf<char> buf((*output_length)+4);

			char *encoded_data =buf.get();

			if (encoded_data == NULL) return bstr_t();

			for (int i = 0, j = 0; i < input_length;) {

				uint32_t octet_a = i < input_length ? data[i++] : 0;
				uint32_t octet_b = i < input_length ? data[i++] : 0;
				uint32_t octet_c = i < input_length ? data[i++] : 0;

				uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

				encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
				encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
				encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
				encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
			}

			for (int i = 0; i < mod_table[input_length % 3]; i++)
				encoded_data[*output_length - 1 - i] = '=';
           
			return encoded_data;
		   
	};



	static   bstr_t base64_encode(HGLOBAL hg ,bool fobjref=true) 
	{ 
		unsigned char *data;
		ipc_utils::CHGlobal_t< unsigned char > h(hg);
		data=h;
		bstr_t res=base64_encode(data,h.sizeb());
		if(fobjref&&res.length()) 	return bstr_t("objref:")+res+bstr_t(":");
		else return res;

	}


	static HGLOBAL base64_decode(bstr_t datastr,						
		size_t *output_length=0) {

			size_t tmp;
			if(!output_length) output_length=&tmp;
			size_t input_length;


			//moniker_parser_t<char> mpc;

			v_buf<char> buf;
			buf.cat(datastr);
			if(buf.size_b()==0) return NULL;

			char *data=moniker_parser_t<char>::_trim(buf.get()),*ptail;


			if(!data) return NULL;

			if(moniker_parser_t<char>::_lcheckni(data,"objref:",7,&ptail))
			{
				char *p;
				//input_length=safe_len(ptail);
				//if(input_length==0) return NULL;
				if(p=strchr(ptail,':'))
					input_length=int(p)-int(ptail);
				else input_length=safe_len(ptail);
				data=ptail;

				//if(ptail[input_length-1]==':')

			}
			else input_length=safe_len(data);




			if(input_length==0) return NULL;


			if (input_length % 4 != 0) return NULL;

			*output_length = input_length / 4 * 3;
			if (data[input_length - 1] == '=') (*output_length)--;
			if (data[input_length - 2] == '=') (*output_length)--;



			ipc_utils::CHGlobal_t< unsigned char > hg(*output_length);
			unsigned char *decoded_data =hg;


			if(Base64Decode(data,input_length,decoded_data,(int*)output_length))
				return hg.detach();
			else return NULL;



			/*
			const char* encoding_table=base64_encoding_table();

			char decoding_table[256];
			for (int i = 0; i < 256; i++)
			decoding_table[(unsigned char) encoding_table[i]] = i;


			if (decoded_data == NULL) return NULL;

			for (int i = 0, j = 0; i < input_length;) {

			uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
			uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
			uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
			uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

			uint32_t triple = (sextet_a << 3 * 6)
			+ (sextet_b << 2 * 6)
			+ (sextet_c << 1 * 6)
			+ (sextet_d << 0 * 6);

			if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
			if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
			if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
			}
			return hg.detach();
			*/


	}





	static inline bool is_space(char_t c)
	{
		return  isspace( static_cast<unsigned short>(c));
	}
	static inline bool is_space(wchar_t c)
	{
		return  iswspace( static_cast<unsigned short>(c));
	}


	static  CH*  _ltrim(const CH* p) 
	{
		if(p)  while(is_space((*p)))p++;  
		return (CH*)p;
	};

	static  CH*  _rtrim(CH* p)
	{
		if(p)
		{
			CH* pr=p+safe_len(p); 
			while((pr!=p)&&is_space( *(--pr))) *pr=0;  
		}
		return p;
	};


	inline static int str_cmp_ni(const wchar_t* p,const wchar_t* ps,int cb)
	{
		return _wcsnicmp(p,ps,cb);
	}

	inline  static int str_cmp_ni(const char_t* p,const char_t* ps,int cb)
	{
		return _strnicmp(p,ps,cb);
	}




	static  bool  _lcheckni(const CH* pd,const CH* ps,int cb=-1,CH** pptail=0)
	{
		if(!pd) return 0;
		CH* p=_ltrim(pd);
		if(cb<0) {cb=safe_len(ps);}

		bool f=str_cmp_ni(p,ps,cb)==0;
		if(f&&(pptail)) *pptail=p+cb;
		return f;
	}

	static CH*   _lskipi(const CH* p,const CH* ps,int cb=-1,bool fltrim=0)
	{
		//CH* p=pc;
		if(p)
		{
			p=safe_strstri(p,ps);  
			if(!p) return 0;
			if(cb<0) {cb=safe_len(ps);}
			p+=cb;
		}
		return  (fltrim)?(CH*)_ltrim(p):(CH*)p;
	}


	static  CH*  _trim(CH* p)
	{
		return _ltrim(_rtrim(p));
	};

	static wchar_t* to_upper(const wchar_t* str,v_buf<wchar_t>& buf=v_buf<wchar_t>())
	{
		if(!str) return 0;
		wchar_t* p=buf.cat(str).get();
		return (p)?_wcsupr(p):L"";

	}
	static wchar_t* extract_moniker(const wchar_t* str,wchar_t** pptail=0,bool fupper=true,v_buf<wchar_t>& buf=v_buf<wchar_t>())
	{
		if(!str) return 0;
		wchar_t* p,*ptmp;
		int cb;
		wchar_t*& ptail=(pptail)?*pptail:ptmp;



		if(p=moniker_parser_t<wchar_t>::_lskipi(str,L":",1))
			ptail=p;

		p=buf.clear().cat(str,p).get();
		if(!p) return 0;

		if((cb=buf.count())>0)	p[cb-1]=0;

		return (fupper)?_wcsupr(p):p;

	}

	static int parse_str_len(const wchar_t* str)
	{
		if(_lcheckni((wchar_t*)str,L":z:",3))
			return safe_len_zerozero(str);
		else return safe_len(str);
	}

	static int parse_str_len(const char_t* str)
	{
		if(_lcheckni((char_t*)str,":z:",3))
			return safe_len_zerozero(str);
		else return safe_len(str);
	}




	static argv_zz<wchar_t> & parse_args(const wchar_t* parse_string ,argv_zz<wchar_t>& args=argv_zz<wchar_t>())
	{
		const wchar_t* ptail;

		ptail=_lskipi(parse_string,L":",1,1);
		int l;

		if(l=safe_len(ptail))
		{
			wchar_t c[2]=L";";
			if((l>2)&&(ptail[1]==L':'))
		 {
			 if(ptail[0]==L'z') c[0]=L'\0';
			 else if (ptail[0]==L'n') c[0]=L'\n';
			 else c[0]=ptail[0];
			 ptail+=2;
		 }

			args.set_args_delim(ptail,c);	 

		}

		return args;
	}

	inline static  bool file_exists(wchar_t* path)
	{
		if(!path) return false;
		DWORD attr = GetFileAttributesW(path);
		return (attr != INVALID_FILE_ATTRIBUTES);	
	}


   inline static HRESULT getProcAddress(external_callbacks_t* pec,const char_t* procname,void** pproc)
   {
	   
	   OLE_CHECK_PTR(pec&&pproc);

	   HRESULT hr;
	   void * pcalback=0;
	   char *p=(char *)procname,*pn;

	   pn=moniker_parser_t<char>::_lskipi(p,"::",2);
	   if(pn) pn[-2]=0;

	   int l=safe_len(p);


	   while(pec->name)
	   {
		   if(0==safe_cmpni(pec->name,p,l))
		   {
			   if(pn&&(pec->attr_struct&pec->attr))
			   {

				   external_callbacks_t* pecnew=(external_callbacks_t*)pec->proc;

                 return hr=getProcAddress(pecnew,pn,pproc);    
			   }
			   else   pcalback=pec->proc;
			   break;
		   }

		   ++pec;
	   }
	   if(!pcalback) 
		   return hr=HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);  

	   *pproc=pcalback;

	   return S_OK;

   }

	inline static HRESULT getProcAddress(HMODULE hm,const char_t* procname,void** pproc)
	{ 
		HRESULT hr;
		char_t* proc=(char_t*)procname;
		void * pcalback;
		char *p;

		if(p=moniker_parser_t<char>::_lskipi(proc,"::",2))
		{
			p[-2]=0;
			int l=safe_len(p);
			if(!l) 
				return hr=HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);  

			if(pcalback=GetProcAddress(hm,proc))
			{

				external_callbacks_t* pec=(external_callbacks_t*)ipc_utils::make_detach(pcalback);
				OLE_CHECK_PTR(pec);

				hr=getProcAddress(pec,p,pproc);    
				return hr;

             /*
				while(pec->name)
				{
					if(0==safe_cmpni(pec->name,p,l))
					{
						pcalback=pec->proc;
						break;
					}

					++pec;
				}
				if(!pcalback) 
				return hr=HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);  
				*/
			
			}
			else 
				return hr=HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND); 

			

			//


		}
		else{
			pcalback=GetProcAddress(hm,proc);
			if(!pcalback) 
				return hr=HRESULT_FROM_WIN32(GetLastError());
		}

		if(pproc) *pproc=pcalback;

		return S_OK;

	}


	variant_t& get_variant_cache(VARTYPE vt=VT_EMPTY)
	{
		_cache_variant_list_.push_front(variant_holder_t());	 				
		variant_t& v=_cache_variant_list_.front().v;
		*((VARIANT*)(&v))=VARIANT();
		v.vt=vt;
			return v;
	}



	v_buf<CH>& get_cache(int siz=0)
	{
		_cache_list_.push_front(v_buf<CH>());	 
		v_buf<CH>& buf=_cache_list_.front();
		if(siz) buf.resize(siz);
		return buf;
	}

	v_buf<CH>& get_cache(const CH* str)
	{
		return get_cache().cat(str);
	}

	v_buf<CH>& get_cache(bstr_t bstr)
	{
		size_t len
		_cache_list_.push_front(v_buf<CH>());
		v_buf<CH>& buf = _cache_list_.front();
		if (len = bstr.length()) {
			CH* p = (CH*)bstr;
			buf.resize(len);
			memcpy(buf.get(), p, len);
		}

		return buf;
	}

	

	v_buf<CH>& get_cache(const CH* b,const CH* e)
	{
		return get_cache().cat(b,e);
	}

	v_buf<CH>& printf(const CH * fmt ,...)
	{
		va_list argptr;
		va_start(argptr, fmt);	
		return get_cache().vprintf(fmt,argptr);
	}



	CH* trim(const CH* p)
	{		
		return _trim(get_cache(p).get());
	}

	CH* between(const CH* str,const CH bra,const CH ket,CH** ptail=0)
	{
		  if(!str) return 0;
          CH* s0=get_cache(str);
          CH* s=s0;
		  CH* tmp; 
		  CH* pb=0,*pe=s;
		  CH*& tail=(ptail)?*ptail:tmp;
		  tail=(CH*)str;

		  int stack_braket=0;
		  while((*s)){
			  
			  if(*(s++)==bra)
			  {
				  
				  
				  if((stack_braket++)==0)
					  pb=s;	  				   

			  }
			  //else
			  if(*s==ket)
			  { 
				  ;
				  if((--stack_braket)==0)
				  {
					  *(s++)=0;
						  break;						   
				  }
				  else if(stack_braket<0)
					     return 0;

			  }			  
		  }

		  if(pb) tail+=std::distance(s0,s);
		  return pb;

	}


	CH* undecorate(const CH* str,const CH bra=dflt_v<CH>::digits()[22],const CH ket=dflt_v<CH>::digits()[22])
	{
		return get_cache().undecorate(str,bra,ket);
	}

	CH* decorate(const CH* str,const CH bra=dflt_v<CH>::digits()[22],const CH ket=dflt_v<CH>::digits()[22])
	{
		v_buf<CH> buf;		
		CH* p=(buf.undecorate(str,bra,ket));
		return get_cache().cat(bra).cat(p).cat(ket);
	}

	wchar_t* undecorateQQ(const wchar_t* str)
	{
		return undecorate(undecorate(str,L'\'',L'\''),L'\"',L'\"');
	}

	char* undecorateQQ(const char* str)
	{
		return undecorate(undecorate(str,L'\'',L'\''),L'\"',L'\"');
	}

	wchar_t* expand_env(const wchar_t* s)
	{
		if(s)
		{
			v_buf<wchar_t>& buf=get_cache();
			int cb=ExpandEnvironmentStringsW(s,NULL,0)+2;
			buf.resize(cb);
			wchar_t* p=buf;
			ExpandEnvironmentStringsW(s,p,cb);
			return p;
		}
		return 0;
	}
	wchar_t* back_slash_set(const wchar_t* s,bool fcopy=true)
	{
		wchar_t* p,*p0;
		if(s)
		{
			if(fcopy)
            p0=get_cache(s);
				/*
			{
				v_buf<wchar_t>& buf=get_cache();
				p0=buf.cat(s);
			}*/
			else p0=(wchar_t*)s;

			p=p0;
			while(p=wcschr(p,L'/')) *p=L'\\';          
			return p0; 
		}
		return 0; 
	}

	//ERROR_FILE_NOT_FOUND
	//ERROR_PATH_NOT_FOUND

	//_lskipi(const CH* p,const CH* ps,int cb=-1,bool fltrim=0)
	std::pair<CH,CH> split_step(const CH* p,const CH* ps,int cb=-1,bool fltrim=false)
	{
		CH* pt;
		if(cb<0) {cb=safe_len(ps);}
		if(pt=_lskipi(p,ps,cb))
		{
			//int cc=INT_PTR(pt)-INT_PTR(p)-cb;
			pt=get_cache(p,pt-cb);
			if(fltrim) return make_pair(trim(pt),trim(p));
			else make_pair(pt,p);			 
		}
		else return make_pair((CH*)0,(CH*)p);
	}

	std::pair<wchar_t,wchar_t> extract_section(const wchar_t* pstr,const wchar_t* section_def)
	{

		std::pair<wchar_t,wchar_t> pp=split_step(pstr,L"::",2,true);
		if(pp.first)
		{
			pp.first=trim(undecorate(pp.first,L"[",L"]"));

			if(pp.first)
		 {
			 if(pp.first[0]==L"*") pp.first=section_def;
			 else pp.first=0;
		 }

		}	 	 
		return pp;
	}


	argv_zzs<wchar_t>& _get_argv_file_ini(wchar_t* section,argv_zzs<wchar_t>& args=argv_zzs<wchar_t>())
	{
		wchar_t* psection;
		wchar_t* linkf=args[L"@"];
		if(linkf)	{
			std::pair<wchar_t,wchar_t> pp=extract_section(linkf,section);
			argv_zzs tmp;
			wchar_t* pfullfn;

			if(SUCCEEDED(expand_file_path(pp.second,&pfullfn,1)))
			{
				if(pp.first) {
					section=pp.first;
					argv_ini<wchar_t> t;
					tmp=t.init_from_file(pp.first,pfullfn);
				}
				else{
					argv_file<wchar_t> t(pfullfn);
					tmp=t;
				}
				_get_argv_file_ini(section,tmp);

				tmp.parent=args;
				argv_zzs t=tmp;
				args=t;

			}

			//if(pp.first)  tmp.init_from_file(pp.first,)

		}

		return args;
	}

	argv_zzs<wchar_t>& get_argv_ini(const wchar_t* section,argv_zzs<wchar_t>& args=argv_zzs<wchar_t>())
	{
		argv_ini<wchar_t> argini(section,L".ini");	
		args=argini;
		return _get_argv_file_ini(( wchar_t*)section,args);	
	}

	HRESULT expand_stars(const wchar_t* fnc,wchar_t** pfex,bool fpath_only=false)
	{
		HRESULT hr=S_OK;
		wchar_t* ptail,*fn=(wchar_t*)fnc;
		bool fversion=false;

		if(_lcheckni(fn,L"*",1,&ptail))
		{
			HMODULE hm=0;
			v_buf<wchar_t>& buf=get_cache();
			if(ptail[0]==L'*'){ 
				ptail++;

			if(!fpath_only)	
			{
			    int xx=_wtoi(ptail);				
				if((xx==32)||(xx==64))
				{
                   fversion=(sizeof(ptail)!=(xx/8));
				   ptail+=2;
				   if(fversion)
				   {
					   wchar_t* m=get_alter_version_module_path();

					   m=buf.cat(m);
					   if(ptail[0])
					   {
						   m=buf.cat(L"\\..\\").cat(ptail);

					   }
					   *pfex=m;
					   return S_OK;

				   }
				   

				}


			}
				
				hm=ipc_utils::module_t<>::__getHinstance();
				
			}        
			if((!fpath_only)&&(ptail[0]==0))
				buf.cat(ipc_utils::module_t<>::module_name(hm,&hr));
			else  buf.cat(ipc_utils::module_t<>::module_path(hm,&hr)).cat(ptail);
			if(FAILED(hr)) return hr;

			fn=buf;
		}

		*pfex=fn;
		return hr;

	};



	HRESULT expand_starts_path(const wchar_t* pathc,wchar_t** ppathex)
	{
		HRESULT hr=S_OK;
		if(!pathc) return hr;

		wchar_t* ptail,*path=(wchar_t*)pathc;
		argv_zz<wchar_t> pathes;
		pathes.set_args_delim(path,L";");	 
		if(!pathes.argc) return 0;
		argv_zz<wchar_t>::tstringstream stream;
		for(int k=0;k<pathes.argc;k++)
		{
			
				 wchar_t* p=(wchar_t*)pathes.ppvect[k];
				{OLE_CHECK_hr(expand_stars(p,&p,true));}
				if((p)&&(*p)) 
				{	stream<<p;
					stream<<L";";					
				}
			

		}
		  ;

		  *ppathex=get_cache(stream.str().c_str());		
		
		//ppathex=path;
		return hr;
	}

	wchar_t* safe_full_filename(const wchar_t* fnc)
	{
		wchar_t* pfull=L"";

		expand_file_path(fnc,&pfull);
		return pfull;
	}

	HRESULT expand_file_path(const wchar_t* fnc,wchar_t** pp_fullpathname,const wchar_t* search_pathc=0,int fcheck_exists=1,const wchar_t* ext_def=0)
	{
		HRESULT hr;
		bool fok;

		if(!pp_fullpathname) return E_POINTER;
		if(fcheck_exists==128)
		{
                *pp_fullpathname=assoc_module(fnc);

			return S_OK;
		}

		if(!fnc) return hr=HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		wchar_t* search_path=(wchar_t*) search_pathc;
		wchar_t* fn=_trim(back_slash_set(undecorate(_trim(expand_env(fnc))),false));
		wchar_t* ptail,*p;
		wchar_t*& pfullpath=*pp_fullpathname;
		pfullpath=L"";
		/*
		if(_lcheckni(fn,L"*",1,&ptail))
		{
		HMODULE hm=0;
		v_buf<wchar_t>& buf=get_cache();
		if(ptail[0]==L'*'){ 
		hm=ipc_utils::module_t<>::__getHinstance();
		ptail++;
		}        
		if(ptail[0]==0)
		buf.cat(ipc_utils::module_t<>::module_name(hm,&hr));
		else  buf.cat(ipc_utils::module_t<>::module_path(hm,&hr)).cat(ptail);
		if(FAILED(hr)) return hr;
		fn=buf;
		}
		*/ 

		OLE_CHECK_hr(expand_stars(fn,&fn));
		OLE_CHECK_hr(expand_starts_path(search_path,&search_path));

		if(!fn) return hr=HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		//bool frel=PathIsRelativeW(fn);
		//if(frel&&fcheck_exists)
		if(fcheck_exists)
		{    
			wchar_t* sp=_trim(back_slash_set(undecorate(_trim(expand_env(search_path))),false));
			v_buf<wchar_t>& buf=get_cache();
			buf.resize(4096+2);
			if(fok=SearchPathW(sp,fn,ext_def,4096,buf.get(),&p))
			{ fn=buf;hr=S_OK;}
			else return   hr=HRESULT_FROM_WIN32(GetLastError());

		}

		v_buf<wchar_t>& buf=get_cache(4096+2);
		//buf.resize(4096+2);

		if(!(fok=GetFullPathNameW(fn,4096,buf.get(),&p)))
			return   hr=HRESULT_FROM_WIN32(GetLastError());
		pfullpath=buf.get();
		if(fcheck_exists==2) p[0]=0;

		if( fcheck_exists&&(!file_exists(pfullpath)))
			return hr=HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

		if(fcheck_exists&16) 
		{
			// pfullpath=file_uuid(pfullpath,get_cache()).get();
			//

			union{
				struct {GUID gf;};
				struct {long l[4];};
			} ug;

			ug.gf=uuid_file(pfullpath,get_cache());

			//bstr_t b=base64_encode((unsigned char *)&gf,sizeof(gf));

			if(fcheck_exists&32)
		 {
			 if(fcheck_exists&2)
			 {
				 wchar_t* name=get_cache(p);
				 p[0]=0;

				 pfullpath=get_cache().printf(L"%s*%s*%x_%x_%x_%x",pfullpath,name,ug.l[0],ug.l[1],ug.l[2],ug.l[3]);

			 }
			 else 	 pfullpath=get_cache().printf(L"%s*%x_%x_%x_%x",pfullpath,ug.l[0],ug.l[1],ug.l[2],ug.l[3]);
		 }
			else pfullpath=get_cache().printf(L"%x_%x_%x_%x",ug.l[0],ug.l[1],ug.l[2],ug.l[3]);

		}


		if (fcheck_exists & 256) {

			int vnm = (fcheck_exists >> 10);
			
			pfullpath = sysfn2dosfn(pfullpath,vnm,&hr);
			OLE_CHECK_hr(hr);
			
		}

		if (fcheck_exists & 64) {
			long len;
			if (len = GetShortPathName(pfullpath, NULL, 0)) {
				wchar_t *p = get_cache(len+1);
				len = GetShortPathName(pfullpath, p, len+1);
				pfullpath = p;
			}
			if (!len)
				return hr=HRESULT_FROM_WIN32(GetLastError());

		}

		return S_OK;

	}

	HRESULT load_string(const wchar_t* fnc,bstr_t& bstr)
	{
		HRESULT hr;
		  wchar_t* fn =undecorateQQ(fnc),*fnf;
		  //expand_file_path(const wchar_t* fnc,wchar_t** pp_fullpathname,const wchar_t* search_pathc=0,int fcheck_exists=1,const wchar_t* ext_def=0)
		   OLE_CHECK_hr(expand_file_path(fn,&fnf));
		  buf_fm_T<char> bf(fnf);
		  bstr=(char*)bf;
		  return hr;
	}

	wchar_t* get_version_module_path(const wchar_t* parse_str,wchar_t** pptail=0,void *pmod=((void*)-1)){
		wchar_t* ptmp;
		if(pptail==0) pptail=&ptmp;

		int self_model=sizeof(void*),model=sizeof(void*);
		if(_lcheckni((wchar_t*)parse_str,L":64",3,pptail))
			model=8;
		else 
			if(_lcheckni((wchar_t*)parse_str,L":32",3,pptail))
				model=4;

		wow_index=model/8;

		wchar_t*  mn=get_cache((wchar_t*)ipc_utils::module_t<>::module_name(pmod));
		if(self_model==model)
			return mn;

		wchar_t* path=get_cache(4096+32),*pname=0;	  
		if(!GetFullPathNameW(mn,4096,path,&pname))
			return 0;
		wchar_t* name=get_cache(pname);
		*pname=0;
		wchar_t* dirs[2]={L"..\\Win32\\",L"..\\x64\\"};
		pname=get_cache(path).cat(dirs[model/8]).cat(name);

		if(!GetFullPathNameW(pname,4096,path,&pname))
			return 0;	  	  
		return path;

	}

	wchar_t* get_alter_version_module_path(void *pmod=((void*)-1))
	{
		return get_version_module_path((sizeof(void*)/8)?L":32":L":64",0,pmod);
	}


	v_buf<wchar_t>& getenv(const wchar_t* name,const wchar_t* dfltval=0)
	{
		int cb;

		if(name&&(cb=GetEnvironmentVariableW(name,0,0))){			 
		   v_buf<wchar_t>& r=get_cache(cb);
		   GetEnvironmentVariableW(name,(wchar_t*)r,cb);
		   return r;		 
		}
        return get_cache(( wchar_t*)dfltval);
	}

	variant_t& vgetenv(const wchar_t* name, variant_t dltval = variant_t())
	{
		int cb;
		wchar_t* r;
		variant_t& res = get_variant_cache();
		if (name && (cb = GetEnvironmentVariableW(name, 0, 0))) {
			 r = get_cache(cb);
			 if (GetEnvironmentVariableW(name, r, cb)) {
				 res = r;
				 return res;
			 }
		}

		res = dltval;
		return res;
			//cache((wchar_t*)dfltval);
	}

	template <class T>
	static inline bool equ_as(const void *p1,const void *p2){
		return *((T*)p1)==*((T*)p2);
	}

	inline wchar_t* tmp_file_name(const wchar_t* prfx=L"~za"){

		
		 wchar_t* pth,*pname;
		if(GetTempPathW(MAX_PATH,pth=get_cache(MAX_PATH))&&GetTempFileNameW(pth,prfx,0,pname=get_cache(MAX_PATH)))
			return pname;
		else return 0;			 
		
	}

	inline wchar_t* tmp_file_name2(const wchar_t* prfx = L"~za"){

		wchar_t* pth, *pname;		
		
		if (GetTempPathW(MAX_PATH, pth = get_cache(MAX_PATH))) {
			GUID g;
			CoCreateGuid(&g);
			DWORD *ii = (DWORD *)&g;

			prfx || (prfx = L"");			

			pname = get_cache().printf(L"%s\\%s%08X%08X%08X%08X.tmp", pth, prfx, ii[0], ii[1], ii[2], ii[3]);
			return pname;
		}
			
		else return 0;


	}

	inline wchar_t* expandenv(const wchar_t* str) {
		size_t cb;
		if (cb = ExpandEnvironmentStringsW(str, 0, 0)) {

			wchar_t* r = get_cache(cb);
			ExpandEnvironmentStringsW(str, r, cb);
			return r;

		}
		else return L"";
	}
	/*
	inline wchar_t* tmp_file_name2(const wchar_t* prfx = L"~za")
	{


		wchar_t* pth, *pname, *tmp;
		
		v_buf<wchar_t> b = get_cache();
		prfx || (prfx = L"");
		GUID g;
		CoCreateGuid(&g);
		DWORD *ii = (DWORD *)&g;

		tmp= getenv(L"TEMP",L"");
		b.printf(L"%s\\%s%08X%08X%08X%08X.tmp", tmp, prfx, ii[0], ii[1], ii[2], ii[3]);
		pname = b;
		
		return pname;

	}
	*/



	moniker_parser_t(const CH* parse_str=0,IBindCtx* pbctx=0)
	{
		HRESULT hr;
		if(parse_str) parse_args(parse_str,args);
		if(pbctx)
		{
			ipc_utils::smart_ptr_t<IUnknown> unk; 
			ipc_utils::bstr_c_t<64> bstrc( LTX_BINDCONTEXT_PARAMS);
			//hr=pbctx->GetObjectParam(bstr_t(LTX_BINDCONTEXT_PARAMS),unk._address());
			hr=pbctx->GetObjectParam(bstrc,unk._address());
			OLE_CHECK_VOID(hr);
			hr=unk.QueryInterface(&bind_disp.p);
		}


	}

	arg_result_t bind_args(const wchar_t* name,bool fhexbin=false)
	{
		HRESULT hr;
		int index=-1;
		
		variant_t& res=get_variant_cache();
        wchar_t* pvalue=args[name];
		
		if(!pvalue) return res;

		if(*pvalue!=L'#')		{
			bstr_t bvalue=pvalue;
			res.vt=VT_BSTR;
			res.bstrVal=bvalue.Detach();
			//return res;
		} 
		else{

			index=_wtoi(pvalue+1);
		
		

		



		if((index>0)&&bind_disp)
		{
			VARIANT v={VT_I4};
			v.intVal=index;			
			DISPPARAMS dps={&v,0,1,0};
			if(FAILED(hr=bind_disp->Invoke(0, IID_NULL, LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET|DISPATCH_METHOD,&dps,&res,0,0)))
				return res;
		}
		}

		if(fhexbin&&(res.vt==VT_BSTR))
		{
			variant_t& r=get_variant_cache(VT_UI8);
			
			wchar_t* p=_ltrim(res.bstrVal);
			int cb;
			if(!(cb=str_hex(p,r.ullVal)))
				cb=str_bin(p,r.ullVal);
			if(cb)			    
				return r;
		}

		return res;

	}

	wchar_t*  bind_args_str( wchar_t* name)
	{
		bstr_t s=bind_args(name,false).def<bstr_t>();
		return get_cache((wchar_t*)s);

	}

	operator bool(){
		return args.argc;
	}

	static void __cdecl  disp_parse( variant_t* presult, pcallback_lib_arguments_t pltx,pcallback_context_arguments_t pcca,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
	{


		moniker_parser_t mn;


	}

	inline wchar_t* assoc_module(const wchar_t* fn)
	{
		if(!fn) return 0;
		wchar_t* pext=PathFindExtensionW(fn);
		if(!pext) return 0;
		DWORD cb=2000;
		v_buf<wchar_t> vbk(cb);
		DWORD ct;
		wchar_t* buf=get_cache(int(cb));		
		wchar_t *pbk=buf;

		HRESULT hr;
			if((hr=RegGetValueW(HKEY_CLASSES_ROOT,pext,L"",RRF_RT_REG_SZ,&ct,buf,&(cb=2000)))!=ERROR_SUCCESS)
				return 0; 
			vbk.printf(L"%s\\shell\\open\\command",pbk);
		     pbk=vbk;

		if((hr=RegGetValueW(HKEY_CLASSES_ROOT,vbk,L"",RRF_RT_REG_SZ,&ct,buf,&(cb=2000)))!=ERROR_SUCCESS)
			return 0;			
		pbk=buf;

		//AssocQueryStringW(0,ASSOCSTR_COMMAND,pext,0,buf,&cb);
		return  pbk;
	}

    static bool _replacei_1(CH* s,const CH* si,const CH* di,v_buf<CH>& res)
	{
		CH* p;
        int l_si=safe_len(si);
		bool f;
		res.clear();
		if(f=(l_si&&(p=safe_strstri(s,si))))
		{
		 p[0]=0;
		  res.cat(s).cat(di).cat(p+l_si);
		}
		else  res.cat(s);
		return f;
	}

	static v_buf<CH>& _replacei(const CH* s,const CH* si,const CH* di,v_buf<CH>& res=v_buf<CH>())
	{
       v_buf<CH> buf;		
	   res.clear().cat(s);	
		CH* p;	
		while(_replacei_1(p=res,si,di,buf))	{
			res.v=buf.v;		   
		}
		return res;
	}

	CH* replacei(const CH* s,const CH* si,const CH* di)
	{
		return _replacei(s,si,di,get_cache());
	}


	HRESULT cmdline_parse(wchar_t** ppres,const  wchar_t* fnc,const wchar_t* cmdlinec=0,const wchar_t* search_pathc=0)
	{


		HRESULT hr=HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION);
		OLE_CHECK_PTR(ppres);

		 wchar_t* fn=(wchar_t*)fnc,*cmdline=(wchar_t*)cmdlinec;
		//ERROR_NO_ASSOCIATION
		v_buf<wchar_t> cmdtail,buf;
        int ib=safe_len(fn)?1:0;
		int ibc=safe_len(cmdline)?1:0;
		wchar_t** pp=0,*p,*ptail;
        int nc=-1;   
		if(ibc) pp=CommandLineToArgvW(cmdline,&nc);

		struct LFree_t {
			wchar_t** pp;
			~LFree_t(){if(pp) LocalFree(pp);}
		} LFree={pp};

		     
		
		if(!ib)
		{
           if(nc<=0) return 0;   
			fn=pp[0];
			p=fn;
            if(*cmdline==L'"')				
			 p=buf.cat(L"\"").cat(p).cat(L"\"");
			if(!_lcheckni(cmdline,p,-1,&ptail))
				 return hr;

			cmdtail.cat(_trim(ptail));


		}
		else cmdtail.cat(cmdline);

		cmdline=cmdtail;
		fn=buf.undecorate(fn);
		

			OLE_CHECK_hr(expand_file_path(fn,&fn,search_pathc));

			wchar_t* assoc_str = assoc_module(fn);
			if (!assoc_str) return hr = HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION);


			wchar_t* res;

			 res=replacei(assoc_str,L"%1",fn);
			 res=replacei(res,L"%*",cmdtail);
			 res=expand_env(res);
			 
			 *ppres=res;

			 return hr;



	}
	

	HANDLE& push_handle(HANDLE h=NULL){

		_cache_handle_list_.push_front(handle_holder_t());	 
		HANDLE& rh=_cache_handle_list_.front().h=h;
		return rh;
	}


	v_buf<wchar_t>& sysfn2dosfn(wchar_t* psysfn,int vnm= VOLUME_NAME_DOS, HRESULT*  phr = 0) {
		BOOL f;
		v_buf<wchar_t>& name = get_cache();
		HRESULT h, &hr = (phr) ? *phr : h;
		HANDLE hf;
		handle_holder_t hh;
		if (psysfn) {
			hr = S_OK;
			hh.h=hf= CreateFile(psysfn,
				GENERIC_READ,          // open for reading
				FILE_SHARE_READ,       // share for reading
				NULL,                  // default security
				OPEN_EXISTING,         // existing file only
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, // normal file
				NULL);
			if (hf != INVALID_HANDLE_VALUE) {
				wchar_t* pname = get_cache(1024);
				DWORD dw= GetFinalPathNameByHandle(hf, pname, 1024, vnm);
				if(dw>4){
					name.cat(pname + 0);
				}
				else 
					hr = HRESULT_FROM_WIN32(GetLastError());
			}
			else 
				hr = HRESULT_FROM_WIN32(GetLastError());

		}
		else hr = E_POINTER;
		return name;
	}

	v_buf<wchar_t>& get_filemap_name(void* pmap,HRESULT*  phr=0){
		BOOL f;
		HRESULT h,&hr=(phr)?*phr:h;
		
		wchar_t* dname=get_cache(1024);
		wchar_t* pdevice=get_cache(1024);
		v_buf<wchar_t>& name=get_cache();
		size_t len,lenname;
		




		wchar_t szTemp[512]=L"",szDrize[3]=L"?:",*p=szTemp;

		if(GetMappedFileNameW(GetCurrentProcess(),pmap,dname,1024)&&GetLogicalDriveStrings(511,szTemp)){			
			lenname=wcslen(dname);
			while(*p){
				*szDrize=*p;
				if(f=QueryDosDevice(szDrize,pdevice,1023)){
					len=wcslen(pdevice);
					f=(len<lenname)&&(_wcsnicmp(dname,pdevice,len)==0);
					if(f){
						name.cat(szDrize).cat(dname+len);
						break;
					}
					p+=4;

				}

			  //if(QueryDosDevice(szDrive,pdevice, MAX_PATH)){	   }

			}
			hr=S_OK;

		}
		else hr=HRESULT_FROM_WIN32(GetLastError());
		wchar_t* pname=name;
		return name;
	}

	v_buf<wchar_t>& get_filemap_name_by_handle(HANDLE hmap,HRESULT*  phr=0){
		BOOL f;
		HRESULT h,&hr=(phr)?*phr:h;
		struct mvp_t{
			void* p;
			~mvp_t(){
				UnmapViewOfFile(p); 
			}
		 
		} mvp={MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 1)};
		if(mvp.p){
			return get_filemap_name(mvp.p,&hr);
		}
		else hr=HRESULT_FROM_WIN32(GetLastError());
		return get_cache();
	}

	static HANDLE open_info_handle(wchar_t* pfn		
		,DWORD daccess=GENERIC_READ//FILE_READ_ATTRIBUTES|DELETE
		,DWORD sharemode=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE
		,DWORD crds=OPEN_EXISTING
		,DWORD fattr=FILE_FLAG_BACKUP_SEMANTICS|FILE_ATTRIBUTE_NORMAL
		){

		
		
        /*
		const DWORD daccess=FILE_READ_ATTRIBUTES,//|FILE_READ_EA,
			sharemode=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			crds=OPEN_EXISTING;
			//fattr= FILE_FLAG_BACKUP_SEMANTICS ;//FILE_ATTRIBUTE_NORMAL;
			*/

		
			
		if(!pfn)
			return SetLastError(HRESULT_FROM_WIN32(E_HANDLE)),INVALID_HANDLE_VALUE;

		HANDLE h;

		h=::CreateFileW(pfn,daccess,sharemode,NULL,crds,fattr,0);		

		return h;

	}


	  bool cmp_file_id(HANDLE h1,HANDLE h2,HRESULT* phr=0){

		bool f;
		HRESULT t,&hr=(phr)?*phr:t;
		BY_HANDLE_FILE_INFORMATION  hfi1,hfi2; 

		/*
		const DWORD NSZ=1024;
		wchar_t *fn1=get_cache(NSZ),*fn2=get_cache(NSZ);
		f=GetFinalPathNameByHandle(h1,fn1,NSZ-1,VOLUME_NAME_GUID);
		 hr=HRESULT_FROM_WIN32(GetLastError());
		f=GetFinalPathNameByHandle(h2,fn2,NSZ-1,VOLUME_NAME_GUID);
		*/


		if((INVALID_HANDLE_VALUE==h1)||(INVALID_HANDLE_VALUE==h2))
		 return hr=HRESULT_FROM_WIN32(E_HANDLE),false;


            
		if(!(GetFileInformationByHandle(h1,&hfi1)))
		return hr=HRESULT_FROM_WIN32(GetLastError()),false;
		if(!(GetFileInformationByHandle(h2,&hfi2)))
			return hr=HRESULT_FROM_WIN32(GetLastError()),false;

		f = (hfi1.nFileIndexLow == hfi2.nFileIndexLow) && (hfi1.nFileIndexHigh == hfi2.nFileIndexHigh);
		f = f && (hfi1.dwVolumeSerialNumber == hfi2.dwVolumeSerialNumber);
		if (f) {
			hr = S_OK;
		}

		return f;
	}

	bool cmp_file_id(HANDLE h1,wchar_t* fn2,HRESULT* phr=0){
		HRESULT t,&hr=(phr)?*phr:t;
		HANDLE h2=open_info_handle(fn2);
		if(INVALID_HANDLE_VALUE==h2)
			return hr=HRESULT_FROM_WIN32(GetLastError()),false;
		else
			return cmp_file_id(h1,push_handle(h2),phr);
	}

	bool cmp_file_id(wchar_t* fn1,wchar_t* fn2,HRESULT* phr=0){
		HRESULT t,&hr=(phr)?*phr:t;
		HANDLE h1=open_info_handle(fn1);
		if(INVALID_HANDLE_VALUE==h1)
			return hr=HRESULT_FROM_WIN32(GetLastError()),false;
		else
			return cmp_file_id(push_handle(h1),fn2,phr);
	}

	variant_t& to_unicode(char * p,size_t len,int cp_xx=CP_UTF8,HRESULT* phr=0){
		HRESULT t,&hr=(phr)?*phr:t;



		variant_t& result=get_variant_cache(VT_BSTR);
		if(!len)
			return result;
		if(cp_xx==-1)
		{
			if(result.bstrVal=SysAllocStringByteLen(0,len)){
				::memcpy(result.bstrVal,p,len);
			}
			else
			 hr=HRESULT_FROM_WIN32(GetLastError());
			


			return result;
			
		}
		int cb=MultiByteToWideChar(cp_xx,0,p,len,0,0);

		if((cb)&&(result.bstrVal=SysAllocStringLen(0,cb))){
			int cb2=MultiByteToWideChar(cp_xx,0,p,len,result.bstrVal,cb);
			if(cb2!=cb){
				hr=E_INVALIDARG;	
			}	

		}
		else hr=HRESULT_FROM_WIN32(GetLastError());
		return result;
	}

	char* from_unicode(BSTR  pw,size_t wlen=-1,size_t* plen=0,int cp_xx=CP_UTF8,HRESULT* phr=0){
		HRESULT t,&hr=(phr)?*phr:t;
		size_t l,&len=(plen)?*plen:l;

		if(wlen==-1){			
			if(!(wlen=SysStringLen(pw)))			 
				return NULL;
		}


		

		if(!(len=WideCharToMultiByte(cp_xx,0,pw,wlen,0,0,0,0)))			 
			 return hr=HRESULT_FROM_WIN32(GetLastError()),NULL;
		


		char* p=(char*)get_cache((len+1)/2).get();
		if(!p)
			return hr=E_OUTOFMEMORY,NULL;


		
		if(!(len=WideCharToMultiByte(cp_xx,0,pw,wlen,p,len,0,0)))			 
			return hr=HRESULT_FROM_WIN32(GetLastError()),NULL;	

		
		return p;
	}








//static	HRESULT bindObject()

	ipc_utils::smart_ptr_t<IDispatch> bind_disp;

	argv_zz<CH> args;

    std::list< handle_holder_t > _cache_handle_list_;
	std::list< v_buf<CH> > _cache_list_;
	std::list< variant_holder_t > _cache_variant_list_;
	int wow_index;
	
	private:
		struct bcbparams_t {
			HWND hwin;
			WCHAR *path;
		};

		static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
		{
			WCHAR path[2 * MAX_PATH];

			if (uMsg == BFFM_SELCHANGED) {
				if (lParam&&::SHGetPathFromIDListW(LPITEMIDLIST(lParam), path)) {					
					SetWindowTextW(hwnd, path);
				}
			}

			if (uMsg == BFFM_INITIALIZED)
			{				
				bcbparams_t* params = (bcbparams_t*)lpData;
				RECT r = {};
				::GetWindowRect(params->hwin, &r);
				::SetWindowPos(hwnd,HWND_TOP,
					r.left + 10,
					r.top + 10,
					0,0, SWP_NOSIZE);

				:: SendMessageW(hwnd, BFFM_SETSELECTION, TRUE, LPARAM(params->path));
			}
			return 0;
		}
    public:

	static bstr_t BrowseForFolder(HWND hwin, bstr_t title, bstr_t initialPath, UINT flags=-1) {

			WCHAR path[2*MAX_PATH];
			bstr_t res;
			

			BROWSEINFOW bi = { 0 };

			wchar_t* path_param = initialPath;
			bcbparams_t bparams = {hwin,path_param};
			bi.hwndOwner = hwin;
			bi.lpszTitle = title;
			bi.ulFlags = (flags==-1)?( BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE):flags;

			bi.lpfn = &BrowseCallbackProc;
			bi.lParam = (LPARAM)&bparams;			
			bi.pszDisplayName = path;

			LPITEMIDLIST pidl = ::SHBrowseForFolderW(&bi);
			
			if (pidl) {

				if (SHGetPathFromIDList(pidl, path)) {
					res = path;
				}

				::CoTaskMemFree(pidl);
			}

			return res;
		}




};