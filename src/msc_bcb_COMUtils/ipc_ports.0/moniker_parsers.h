#pragma once
// #include "moniker_parsers.h"
#include "ltx_call_helper.h"
#include "wchar_parsers.h"
#include "ipc_utils.h"
#include "Shlwapi.h"

template <class CH=wchar_t>
struct moniker_parser_t
{


	typedef std::basic_string<CH, std::char_traits<CH>,std::allocator<CH> > tstring;
	typedef  unsigned int uint32_t;

	typedef char char_t;

	enum ch:CH{
          zerro=0
	};

	

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

	inline static BOOL Base64Decode(LPCSTR szSrc, int nSrcLen, BYTE *pbDest, int *pnDestLen) 
	{

		if (szSrc == NULL || pnDestLen == NULL)
		{
			return FALSE;
		}

		LPCSTR szSrcEnd = szSrc + nSrcLen;
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
	  return  iswspace( static_cast<unsigned short>(c));
  }
  static inline bool is_space(wchar_t c)
  {
	  return  isspace( static_cast<unsigned short>(c));
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


 inline static int str_cmp_ni(wchar_t* p,wchar_t* ps,int cb)
 {
       return _wcsnicmp(p,ps,cb);
 }

inline  static int str_cmp_ni(char_t* p,char_t* ps,int cb)
 {
	 return _strnicmp(p,ps,cb);
 }

 

 static  bool  _lcheckni(CH* p,CH* ps,int cb=-1,CH** pptail=0)
 {
	 if(!p) return 0;
	 p=_ltrim(p);
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

		}
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


v_buf<CH>& get_cache(int siz=0)
{
	 cache_list.push_front(v_buf<wchar_t>());	 
	 v_buf<wchar_t>& buf=cache_list.front();
	 if(siz) buf.resize(siz);
	 return buf;
}

v_buf<CH>& get_cache(CH* str)
{
    return get_cache().cat(str);
}

v_buf<CH>& get_cache(CH* b,CH* e)
{
	return get_cache().cat(b,e);
}


CH* trim(const CH* p)
{
 v_buf<CH>& buf=get_cache();
  return _trim(buf.cat(p).get());
}

CH* undecorate(const CH* str,const CH bra=dflt_v<CH>::digits()[22],const CH ket=dflt_v<CH>::digits()[22])
{
 return get_cache().undecorate(str,bra,ket);
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
		{
         v_buf<wchar_t>& buf=get_cache();
		 p0=buf.cat(s);
		}
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

HRESULT expand_starts(const wchar_t* fnc,wchar_t** pfex,bool fpath_only=false)
{
	   HRESULT hr=S_OK;
      wchar_t* ptail,*fn=(wchar_t*)fnc;

	  if(_lcheckni(fn,L"*",1,&ptail))
	  {
		  HMODULE hm=0;
		  v_buf<wchar_t>& buf=get_cache();
		  if(ptail[0]==L'*'){ 
			  hm=ipc_utils::module_t<>::__getHinstance();
			  ptail++;
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
     wchar_t* ptail,*path=(wchar_t*)pathc;
	  while(_lcheckni(path,L"*",1,&ptail))
    	  {OLE_CHECK_hr(expand_starts(path,&path,true));}
	  *ppathex=path;
    return hr;
}

HRESULT expand_file_path(const wchar_t* fnc,wchar_t** pp_fullpathname,const wchar_t* search_pathc=0,int fcheck_exists=1)
{
	HRESULT hr;
	bool fok;
	if(!pp_fullpathname) return E_POINTER;

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

	   OLE_CHECK_hr(expand_starts(fn,&fn));
	   OLE_CHECK_hr(expand_starts_path(search_path,&search_path));

       if(!fn) return hr=HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	bool frel=PathIsRelativeW(fn);
	if(frel)
	{    
        wchar_t* sp=_trim(back_slash_set(undecorate(_trim(expand_env(search_path))),false));
         v_buf<wchar_t>& buf=get_cache();
		 buf.resize(4096+2);
		 if(fok=SearchPathW(sp,fn,NULL,4096,buf.get(),&p))
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


	return S_OK;

}

wchar_t* get_version_module_path(const wchar_t* parse_str,wchar_t** pptail=0)
{
       wchar_t* ptmp;
	   if(pptail==0) pptail=&ptmp;

	int self_model=sizeof(void*),model=sizeof(void*);
	if(_lcheckni((wchar_t*)parse_str,L":64",3,pptail))
		 model=8;
	else 
		if(_lcheckni((wchar_t*)parse_str,L":32",3,pptail))
			model=4;

	  wchar_t*  mn=get_cache((wchar_t*)module_t<>::module_name());
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
wchar_t* get_alter_version_module_path()
{
	return get_version_module_path((sizeof(void*)/8)?L":32":L":64");
}



moniker_parser_t(const CH* parse_str=0)
{
	if(parse_str) parse_args(parse_str,args);
}

operator bool(){
	return args.argc;
}

static void __cdecl  disp_parse( variant_t* presult, pcallback_lib_arguments_t pltx,pcallback_context_arguments_t pcca,int argc,variant_t* argv,int arglast,HRESULT& hr,void*,unsigned flags)
{
	moniker_parser_t mn;
      

}
 
argv_zz<CH> args;

std::list< v_buf<CH> > cache_list;

};