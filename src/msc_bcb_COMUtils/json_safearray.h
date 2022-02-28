#pragma once

#include <windows.h>
#include <comdef.h>
#include <char_parsers.h>
#include <wchar_parsers.h>
#include <os_utils.h>
#include <atlenc.h>

struct SAFEARRAY_json_parser
{
	HRESULT hr;
	VARTYPE vt;
	long ind[64];
	int dim;
	SAFEARRAY* psa;
	char dimstr[640];
	bstr_t dlmt;
	bstr_t buf;
    bstr_t comment;
    bstr_t comment_name;
	unsigned mask;
	bool fquote;
	bool flinpack;
	bool fbase64;
	int actualcount;
	int offset;

	struct inner_array_t{
	SAFEARRAY sa;
	SAFEARRAYBOUND bound[63];
	} inner_array;

	int fc;
	char* pd;
	std::vector<double> floatbuf;
	v_buf<BYTE> bytebuf;


inline operator VARIANT()
{
	VARIANT r={vt|VT_ARRAY};
	r.parray=psa;
	return r;
}

inline 	wchar_t* _get_value(wchar_t*name,wchar_t*str)
 {
        wchar_t*p = wcsstr(str,name);
		if(p)
		{
			int l=wcslen(name);
			p+=l;
			while((iswspace(*p))&&(*p!=L':')&&(*p!=L'"')&&(*p!=L'\"')&&(*p!=L',')&&(*p))++p;
			if(*p!=L':') return 0;
			//while((iswspace(*(++p))));
			//if((*p==L'\'')||(*p==L'\"')) ++p;
														           
		}
		return p;
 }
inline 	wchar_t* get_value(wchar_t*name,wchar_t*str)
{
	    wchar_t* p=_get_value(name,str);
		if(!p) 
		{
           p=_get_value(v_buf<wchar_t>().printf(L"\"%s\"",name),str);
		}
		return p;
}


///*
inline 	char* _get_value(char*name,char*str)
{
	char*p = strstr(str,name);
	if(p)
	{
		int l=strlen(name);
		p+=l;
		while((isspace(*p))&&(*p!=L':')&&(*p!=L'"')&&(*p!=L'\"')&&(*p!=L',')&&(*p))++p;
		if(*p!=L':') return 0;
		//while((iswspace(*(++p))));
		//if((*p==L'\'')||(*p==L'\"')) ++p;

	}
	return p;
}
inline 	char* get_value(char*name,char*str)
{
	char* p=_get_value(name,str);
	if(!p) 
	{
		p=_get_value(v_buf<char>().printf("\"%s\"",name),str);
	}
	return p;
}

static inline bool lcheckni(char* p,char* ps,int cb,char** pptail=0)
{
	{
		if(!p) return 0;
		while(isspace(*p)) ++p;		
		if(cb<0) {cb=strlen(ps);}
		//bool f=(p==StrStrNIA(p,ps,cb)); strstrni
		bool f=StrCmpNIA(p,ps,cb)==0;
		if(f&&(pptail)) *pptail=p+cb;
		return f;
	}


}


SAFEARRAY_json_parser(char* pstr):hr(E_POINTER),actualcount(0),offset(0)
{
	memset(&inner_array,0,sizeof(inner_array));
	psa=&inner_array.sa;

	size_t cb=0,memsize=0;
	//wchar_t* pstr=bstr;
	if(!pstr) return;

	char* pdata=0;
	int dim;

	std::vector<char> buf;

	while((*pstr!='\0')&&(*pstr!='{')&&(*pstr!='}')) ++pstr;

	char* p=get_value("data",pstr);
	if(p)
	{
		*p=0;
		pdata=p+1;
		if(!(p=get_value("vt",pstr))) return;
		vt=get_double(p+1);

		if(!(p=get_value("dim",pstr))) return;

		dim=get_double(p+1);
		std::vector<int> vsizes;
		if(!(p=get_value("sizes",pstr))) return;
		safe_get_vector(p+1,vsizes,0);
		if(dim!=vsizes.size()) return;

		//for(int n=dim-1;n>=0;--n)
		//
		
		for(int n=0,nb=dim-1;n<dim;++n,--nb)
			psa->rgsabound[n].cElements=vsizes[nb];

		psa->cbElements=GetVTSize(vt);
		psa->cDims=dim;


		while((iswspace(*pdata))) ++pdata;

        
         bool fq; 
		if((fq=lcheckni(pdata,"\"base64:",-1,&pdata))||lcheckni(pdata,"base64:",-1,&pdata))
		{
			char*  ppb=pdata;
			char* pb64=ppb,*pend;	
						 						
			if(pend=strrchr(pb64,(fq)?'\"':'}'))
			{
			    *pend=0;
			     int sz64=pend-pb64;	
				int sz=Get_fuck_total_size_b(psa);
				bytebuf.resize(sz);
				psa->pvData=bytebuf.get();
				if(Base64Decode(pb64,sz64,bytebuf,&sz))
				{
					hr=S_OK;
					return;
				};



			}

		}
		else
		{
			safe_get_vector(pdata,floatbuf);
			actualcount=floatbuf.size();
		}


	}
}
//*/
	
	SAFEARRAY_json_parser(wchar_t* pstr):hr(E_POINTER),actualcount(0),offset(0)
	{
		 memset(&inner_array,0,sizeof(inner_array));
		 psa=&inner_array.sa;
		 
         size_t cb=0,memsize=0;
		 //wchar_t* pstr=bstr;
		 if(!pstr) return;

		 wchar_t* pdata=0;
		 int dim;

		std::vector<wchar_t*> buf;
		
		wchar_t* p=get_value(L"data",pstr);
		if(p)
		{
			*p=0;
		     pdata=p+1;
			 if(!(p=get_value(L"vt",pstr))) return;
			    vt=get_double(p+1);
				
			 if(!(p=get_value(L"dim",pstr))) return;
			    
					dim=get_double(p+1);
				std::vector<int> vsizes;
			 if(!(p=get_value(L"sizes",pstr))) return;
			    safe_get_vector(p+1,vsizes,0);
				if(dim!=vsizes.size()) return;
				
		//		for(int n=dim-1;n>=0;--n)
				//for(int n=0;n<dim;++n)
		//			psa->rgsabound[n].cElements=vsizes[n];

				for(int n=0,nb=dim-1;n<dim;++n,--nb)
					psa->rgsabound[n].cElements=vsizes[nb];


				psa->cbElements=GetVTSize(vt);
				psa->cDims=dim;


				while((iswspace(*pdata))) ++pdata;



				if(_wlcheckni(pdata,L"\"base64:",-1,&pdata))
				{
					bstr_t ppb=pdata;
					char* pb64=ppb,*pend;
					if(pend=strrchr(pb64,'\"'))
					{
                      *pend=0;
					  int sz64=pend-pb64;
					  int sz=Get_fuck_total_size_b(psa);
					   bytebuf.resize(sz);
					   psa->pvData=bytebuf.get();
					   if(Base64Decode(pb64,sz64,bytebuf,&sz))
					   {
                        hr=S_OK;
						return;
					   };



					}

				}
				else
				{
					safe_get_vector(pdata,floatbuf);
					actualcount=floatbuf.size();
				}

								
		}

		//dflt_v<T>::digits()[22]
		//std::vector< CH*>& args_ZZ(const CH* pzz,size_t* plength=0,std::vector<CH*>& psv=std::vector<CH*>(),int cinc=8)
		//args_ZZ(str,&cb,buf);
            

	}


	SAFEARRAY_json_parser(SAFEARRAY* _psa,bstr_t delim,unsigned _mask=0):psa(_psa),hr(E_POINTER)
	{

		
		dlmt=(delim.length())?delim:"";

		mask=0xFF&_mask;
		fquote=_mask&0x100;
		flinpack=_mask&0x200;
		fbase64=_mask&0x400;

		static bool f= SetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL,".");
		if(!psa) return ;
		hr=SafeArrayGetVartype(psa,&vt);
		if(FAILED(hr)) return;
		hr=SafeArrayLock(psa);   
		dim=SafeArrayGetDim(psa);

	};
	~SAFEARRAY_json_parser()
	{
		if(SUCCEEDED(hr))   SafeArrayUnlock(psa);
	}

inline operator HRESULT()
	{
		return hr;
	}

inline	VARIANT get_line_element(void* ptr)
	{
		HRESULT hr;
		
		if(VT_VARIANT==vt)
		{
          return *((VARIANT*)ptr);
		}
		else
		{
    		VARIANT v={vt};
			int cb=psa->cbElements;
			memcpy(&v.intVal,ptr,cb);
           return v; 
		} 
	
	};

	char* get_line_element_str(void* ptr)
	{
		VARIANT v=get_line_element(ptr);
		if(v.vt==VT_BOOL)
		{
			return (v.boolVal)?"true":"false";  
		}
		VARIANT vr={0};
		if(FAILED(VariantChangeType(&vr,&v,0,VT_BSTR)))
			return "?";
		buf.Attach(vr.bstrVal);
		return buf;
	}


	inline int dim_size(int nd)
	{
		long l=0,r=0;

		SafeArrayGetLBound(psa,nd,&l);
		SafeArrayGetUBound(psa,nd,&r);
		return r+1-l;

	}

	template <class Stream>
	inline Stream& 	write_bounds(Stream& stream)
	{
		stream<<"[";
		for(int n=1;n<=dim;n++)
		{
			stream<<dim_size(n);
			if(n<dim) stream<<",";
		}

		stream<<"]";
		return stream;
	}
	//SafeArrayPtrOfIndex(psa,ii,pp);

	inline int get_tot_size(int* pelementsize=0,int* pcount=0)
	{
		 if(psa->cDims<=0) return 0;
         LONG32 totelement=1,elsize=psa->cbElements;
		 for(int n=0;n<psa->cDims;n++)
			 totelement*=psa->rgsabound[n].cElements;
		 if(pcount) *pcount=totelement;
		 if(pelementsize) *pelementsize=elsize;
		 return totelement*elsize;
	}


//#define ATL_BASE64_FLAG_NONE	0
//#define ATL_BASE64_FLAG_NOPAD	1
//#define ATL_BASE64_FLAG_NOCRLF  2

//inline int Base64EncodeGetRequiredLength(int nSrcLen, DWORD dwFlags=ATL_BASE64_FLAG_NONE)
	/*
	inline BOOL Base64Encode(
		_In_count_(nSrcLen) const BYTE *pbSrcData,
		_In_ int nSrcLen,
		_Out_z_cap_post_count_(*pnDestLen, *pnDestLen) LPSTR szDest,
		_Inout_ int *pnDestLen,
		_In_ DWORD dwFlags = ATL_BASE64_FLAG_NONE) throw()
*/

	template <class Stream>
	bool set_bin_to_base64(Stream& stream,DWORD fb64=ATL_BASE64_FLAG_NOCRLF)//ATL_BASE64_FLAG_NOPAD|ATL_BASE64_FLAG_NOCRLF)
	{
		  if(fquote)stream<<"\"";
			stream<<"base64:";  
    	int sz=get_tot_size();
		if(!sz) return false;
	    int sz64=Base64EncodeGetRequiredLength(sz,fb64);
		char* buf=(char*)malloc(sz64+4);
		if(!buf) {hr=E_OUTOFMEMORY;return false;}
		BYTE* ptr=(BYTE*)psa->pvData;
		BOOL f=Base64Encode(ptr,sz,buf,&sz64,fb64);
		if(f) 
		{
		  if(fquote) buf[sz64++]='"';
          buf[sz64]=0;
		  stream.write(buf,sz64);
		}
		free(buf);
		return f;

	}


	template <class Stream>
	bool set_linpack_slice(Stream& stream)
	{
		if(psa->cDims<=0) return 0;
		/*
		LONG32 totelement=1,elsize=psa->cbElements;
		for(int n=0;n<psa->cDims;n++)
			totelement*=psa->rgsabound[n].cElements;
			*/
		int elsize;
		int sizeb=get_tot_size(&elsize);


		char* pbegin=(char*)psa->pvData;
		char* pend=pbegin+sizeb;
          stream<<"[";
		  char* p=pbegin,*pstr;
        
		for(;p<pend;p+=elsize)
		{
			if(p!=pbegin) stream<<",";
			pstr= get_line_element_str(p);
			stream<<pstr;	
		}

		stream<<"]";
        return true;

	}

	template <class Stream>
	bool set_slice(Stream& stream,int level=0)
	{
		bool f=true;
		long l,r,nn=dim-level;

		if(!level) memset(ind,0,sizeof(ind));

		SafeArrayGetLBound(psa,nn,&l);
		SafeArrayGetUBound(psa,nn,&r);

		stream<<"[";
		bool fdelim=(level!=dim-1)&&(dim>1);

		//if(fdelim) stream<<(char*)dlmt;

		int sz=r-l+1;   

		if(nn>1)
		{   


			for(int n=l;n<=r;n++)
			{
				//ind[level]=n;
				ind[nn-1]=n;
				//stream<<"[";
				f=set_slice(stream,level+1);
				//stream<<"]";
				//
				if(n<r)
				{	stream<<",";
				if(fdelim) stream<<(char*)dlmt;
				}
				//else stream<<"\n";

			}
		}
		else 
		{

			for(int n=l;n<=r;n++)
			{
				//ind[level]=n;
				ind[nn-1]=n;
				void *p;
				char* pstr;
				if(SUCCEEDED(SafeArrayPtrOfIndex(psa,ind,&p)))
					pstr= get_line_element_str(p);
				else pstr= "?";
				stream<<pstr;
				if(n<r) stream<<",";
			}

		}
		//if(fdelim) stream<<(char*)dlmt;
		stream<<"]";

		return f;  
	};

	template <class Stream,class T>
	inline Stream&	save_prop(Stream& s,T& t,const char *name=0,bool f=0)
	{
		if(f) s<<name<<":";    
          s<<t;
		return s;
	}

	template <class Stream>
	inline Stream&	set_name(Stream& s,const char* name,const char* dlm="")
	{
		if((mask&2)==0)
		{
		 if(fquote) s<<"\""<<name<<"\""<<":"<<dlm;
			 else s<<name<<":"<<dlm;
		}
		return s;
	}

	template <class Stream>
	inline Stream&	df(Stream& s)
	{
      if(fc++) s<<pd;
	  return s;
	};

	template <class Stream>
	inline Stream&	save(Stream& s)
	{
		fc=0;
       	bstr_t delim=L","+dlmt;
		pd=(char*)delim;
		char* pnd=(char*)dlmt;

		if((mask&1)==0)		s<<"{"<<pnd;

		if(((mask&64)==0)&&(comment.length()))
		{
			char *pn=(comment_name.length())?(char*)comment_name:"vb_type";
            set_name(df(s),pn)<<"\""<<(char*)comment<<"\""; 

    		//s<<(char*)comment_name<<"'"<<(char*)comment<<"'"<<pd; 
		}

//	
		if((mask&128)==0)
		{
			
			int elsz,sb=get_tot_size(&elsz);
			set_name(df(s),"mem_size")<<sb;
			set_name(df(s),"element_size")<<elsz;
		};
	 if((mask&32)==0)	set_name(df(s),"vt")<<vt;

      if((mask&16)==0)  set_name(df(s),"dim")<<dim;

	  if((mask&8)==0)	write_bounds(set_name(df(s),"sizes"));

      if((mask&4)==0)	
	  {
		  if(fbase64)
            set_bin_to_base64(set_name(df(s),"data"));
		  else{
		  if(flinpack) set_linpack_slice(set_name(df(s),"data"));
			else set_slice(set_name(df(s),"data"));
		  }
	  }



/*
        s<<"vt:"<<vt<<pd;
		s<<"dim:"<<dim<<pd;
		s<<"sizes:";
		write_bounds(s);
		s<<pd;
		s<<"data:"<<pnd;
		set_slice(s);
*/
	    s<<pnd;
		if((mask&1)==0) s<<"}";
		return s;
	}


};




template<
	class Stream
> inline
Stream& __cdecl operator<< (
							Stream& _s,SAFEARRAY_json_parser& sjp )
	{	
		//char         _s<<  
		//sjp.set_slice(_s); 
		return sjp.save(_s);
	};

