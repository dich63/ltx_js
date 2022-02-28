#pragma once

#include <windows.h>
#include <stdio.h>
#include <vector>
#include <list>
#include <locale>
#include <shellapi.h>
#include <strstream>
#include "char_parsers.h"
#include "Userenv.h"
#include <conio.h>

typedef  bufT<wchar_t,512> wcharbuf;

#pragma comment (lib ,"shlwapi.lib")
#pragma comment ( lib,"Userenv.lib" )

//

struct _non_copyable_
{
	_non_copyable_(){};
private:
	_non_copyable_(_non_copyable_&){};
	_non_copyable_(const _non_copyable_&){};
	void operator=(_non_copyable_&){};
	void operator=(const _non_copyable_&){};

};
typedef _non_copyable_ non_copyable_t;


inline int safe_cputs(const char* p)
{
     return (p)?_cputs(p):-1;
}

inline int safe_cputs(const wchar_t* p)
{
	 return (p)?_cputws(p):-1;
	
}


inline int safe_len(const wchar_t* p,bool fwith_last_zero=false)
{	
	int cadd=(fwith_last_zero)?1:0;
	return (p)?wcslen(p)+cadd:0;
}


inline    int safe_cmpni(const wchar_t* pd,const wchar_t* ps,int maxcb)
{
	if(pd==ps) return 0;
	if(pd==0) return -1;
	if(ps==0) return 1;
	return wcsnicmp(pd,ps,maxcb);
}
inline  const   wchar_t* safe_strchr(const wchar_t* pd,int c)
{
	if(pd)
	{
		return wcschr(pd,c);
	}
	return 0;
}


inline v_buf<wchar_t>& safe_word(const wchar_t* p ,v_buf<wchar_t>& buf=v_buf<wchar_t>())
{
	buf.v.clear();
	int cb=safe_len(p);
	if(cb)
	{
       buf.v.resize(cb+1);
	   //safe_copy(buf.get(),p,cb+1);
	   while(iswspace(*p)&&(*p)) ++p;
	   wchar_t *pd=buf;
       while((!iswspace(*p))&&(*p))
	   {
		   *pd=*p;
		   ++pd;
		   ++p;
	   }   
          
	}
	return buf;
}


inline v_buf<char>&  unicode_to_char_buf(wchar_t* pw,int cb=-1,unsigned int CP_XX=CP_UTF8,v_buf<char>& buf=v_buf<char>())
{

    buf.v.clear();
	if(!pw) return buf;
	if(cb<0){
		cb=wcslen(pw)+1;
	}

	int cba=WideCharToMultiByte(CP_XX,0,pw,cb,0,0,0,0);
	
	buf.v.resize(cba+2);
	buf.v.reserve(cba+2);
	buf.v.resize(cba);

	if(cba)
	{
		cba=WideCharToMultiByte(CP_XX,0,pw,cb,buf,cba,0,0);
		if(!cba) buf.v.clear();
	} 

	


	return buf;
};
inline v_buf<wchar_t>& char_to_unicode_buf(char* s_utf,int len=-1,unsigned int CP_XX=CP_UTF8,v_buf<wchar_t>& buf=v_buf<wchar_t>())
{
	
	    buf.v.clear();
	    if(!s_utf) return buf;
		
		if(len<0){
			len=strlen(s_utf)+1;
		}

		int cb=MultiByteToWideChar(CP_XX,0,s_utf,len,0,0);
		
		if(cb)
		{
		    buf.v.resize(cb);	
			 cb=MultiByteToWideChar(CP_XX,0,s_utf,len,buf,cb);
			 if(!cb) buf.v.clear();
		}
		
	
	
	return buf;

};



template <int FCP_XX=CP_UTF8,bool Ffzz=false>
struct char_mutator
{
  
	v_buf<char> buf;
	v_buf<wchar_t> wbuf;
	char* p;
	wchar_t* pw;
	bool fzz;
	int CP_XX;


inline char* get_pc()
	{
		if(p==0)
		{
			int cb=(fzz)?safe_len_zerozero(pw,true):-1;
           p=unicode_to_char_buf(pw,cb,CP_XX,buf).get();  
		}
		return p;
	}
inline wchar_t* get_pwc()
{
	if(pw==0)
	{
		int cb=(fzz)?safe_len_zerozero(p,true):-1;
		pw=char_to_unicode_buf(p,cb,CP_XX,wbuf).get();  
	}
	return pw;
}
char_mutator(const char* _p,bool fcopy=0,bool _fzz=Ffzz,int _FCP_XX=FCP_XX):p((char*)_p),pw(0),fzz(_fzz),CP_XX(_FCP_XX)
{
   if(fcopy)
   	   p=(fzz)? copy_zz(p,buf).get():copy_z(p,buf).get();
   
}
char_mutator(const wchar_t* _pw,bool fcopy=0,bool _fzz=Ffzz,int _FCP_XX=FCP_XX):p(0),pw((wchar_t*)_pw),fzz(_fzz),CP_XX(_FCP_XX)
{
	if(fcopy)
		   pw=(fzz)? copy_zz(pw,wbuf).get():copy_z(pw,wbuf).get();
		//pw=copy_z(pw,wbuf).get();
}

inline operator char* ()
{
 return get_pc(); 
}
inline operator wchar_t* ()
{
 return get_pwc();
}

inline int char_count()
{
    get_pc(); 
	return buf.count();
}

inline int wchar_count()
{
	get_pwc(); 
	return wbuf.count();
}


};



inline v_buf<char>&  CommandLineToArgvA(wchar_t* pw=GetCommandLineW(),v_buf<char>& psv=v_buf<char>())
{
	int nc;
	psv.v.clear();

	if(!pw)
		return psv;

	wchar_t** ppv=CommandLineToArgvW(pw,&nc);
     wchar_t** ppvs=ppv;

	if(ppv)
	{
		for(;(nc>0)&&(ppv[0][0]==0);nc--,ppv++); 
		psv.v.resize(wcslen(pw)+nc+1);
		
		char *p=psv;
		for(int n=0;n<nc;n++)
		{
			v_buf<char> buf=unicode_to_char_buf(ppv[n]);
			int siz=buf.size_b();
			memcpy(p,buf,siz);
			p+=siz;
		}
        LocalFree(ppvs);
	}
	

	return psv;
}






template <class CH ,typename Setter=fake_setter<CH>,typename LIST=CH** >
struct argv_zzbase:argv_def<CH,LIST,Setter>
{
	std::vector<CH*> ppvect;
	argv_zzbase():argv_def<CH,LIST,Setter>(0,(CH**)0){};

	size_t set_args(CH* _ppargv)
	{
		size_t sz=0;
		ppvect.clear();
		_pv_=0;
		args_ZZ<CH>(_ppargv,&sz,ppvect); 
		argc=ppvect.size();
		if(argc) _pv_=&ppvect[0]; 
		return sz;
	}
	inline CH** args_ptr()
	{
        return (CH**)_pv_;
	}
	inline bool erase(const CH* pname)
	{
		CH* pval;
		if((pname )&&((lastindex=find_named_value((CH*)pname,argc,*pargv,&pval,equsep))>=0))
		{ 
             CH* p=ppvect[make_detach(lastindex,-1)];
			for(std::vector<CH*>::iterator i=ppvect.begin();i!=ppvect.end();++i)
			if((*i)==p)
			{
                 ppvect.erase(i);
				 --argc;
				 return true;   
			}
		         
         
		} 
        return false;
	}

};


template <class CH>
struct argzz_setter
{
   const CH* name;
   argzz_setter(const CH* _name):name(_name)
	{};

   template <typename O>
   void   operator ()(O* po,char v)
   {
	   (*this)(po,int(v));
   }

   template <typename O>
   void   operator ()(O* po,unsigned char v)
   {
	   (*this)(po,unsigned int(v));
   }

	template <typename O,typename V>
	void   operator ()(O* po,V v)
	{
		///*
		O& o=*po;
		//CH sequ[2]={dflt_v<CH>::equ,0};
		CH sequ[2]={o.nv_separator(),0};
		int ind=o.lastindex_0;
		O::tstringstream stream;
		for(int n=0;n<o.argc;++n)
		{
			if(n!=ind)
				stream<<o.args(n)<<CH(0);
			else stream<<name<<sequ<<v<<CH(0);
		}
		if(ind<0) stream<<name<<sequ<<v<<CH(0);

		if(o.parent) o.parent->_flat(stream);

		  stream<<CH(0);

         o.init(stream.str().c_str(),true);
		 //*/
	};

	template <typename O,typename V>
	void   operator ()(O* po,std::vector<V>& vv)
	{
		int siz=vv.size();
		if(!siz) 
			return;

		O& o=*po;
		//CH sequ[2]={dflt_v<CH>::equ,0};
		CH sequ[2]={o.nv_separator(),0};
		int ind=o.lastindex_0;
		O::tstringstream stream;
		const CH* cdig=dflt_v<CH>::digits();
		const CH cq=cdig[17];
		const CH cl=cdig[17+2];
		const CH cr=cdig[17+3];
        stream<<cl<<vv[0];
		for(int n=1;n<siz;++n)
			stream<<cq<<vv[n];
		stream<<cr;
		O::tstring s=stream.str();

        (*this)(po,s.c_str());
	}


};




template <class CH ,typename Setter=fake_setter<CH>,typename LIST=CH** >
struct argv_zz:argv_zzbase<CH,Setter,LIST>
{
	v_buf<CH> buf;

	typedef typename argv_zz<CH,Setter,LIST> argv_t;

	argv_zz(const CH* _ppargv=0,bool fcopy=false,const CH* delim=0)
	{
		if(!safe_len(delim)) init(_ppargv,fcopy);
		else set_args_delim(_ppargv,delim);
	}

	argv_zz(const CH* _ppargv,const CH* delim)
	{
		
		set_args_delim(_ppargv,delim);
	}

	argv_zz& set_args_delim(const CH* _ppargv,const CH* delim)
	{
		ppvect.clear();
		if(!((delim)&&(*delim))) init(_ppargv,true);
		else if(_ppargv)
		{	
		_pv_=0;
		args_ZZs<CH>(delim,_ppargv,buf,ppvect); 
		argc=ppvect.size();
		if(argc) _pv_=&ppvect[0]; 
		}
		return *this;
	}

    const  CH* init(const CH* _ppargv=0,bool fcopy=false)
	{
         
		if(_ppargv)
		{
			CH* p=(CH*)_ppargv;
			if(fcopy)
			    p=copy_zz(p,buf).get();
	  		set_args(p);
		}
	     return _ppargv;
	};

 
 template <class Other>
 inline argv_zz& convert(const Other& a,int CP_XX=CP_UTF8)
 {
	 Other* t=const_cast<Other*>(&a);
	 init(char_mutator<>(t->flat_str(true),false,true,CP_XX),true);
	 return *this;
 }

template <class Other>
inline argv_zz& operator=(const Other& a)
{
		return convert(a,CP_UTF8);
}

};


template <class CH ,typename Setter=argzz_setter<CH>,typename LIST=CH** >
struct argv_zzs:argv_zz<CH,Setter,LIST>
{
	argv_zzs(const CH* _ppargv=0):argv_zz(_ppargv,true)
	{

	};

	
	argv_zzs(const argv_zzs& a):argv_zz(0,true)
	{
       convert(a,CP_UTF8);
	}


	template <class Other>
	inline argv_zzs& operator=(const Other& a)
	{
		 convert(a,CP_UTF8);
		 return *this;
	}

	
};



template <class CH ,typename Setter=argzz_setter<CH>,typename LIST=CH** >
struct argv_zz_trunc:argv_zz<CH,Setter,LIST>
{
	argv_zz_trunc(const CH* _ppargv=0,const CH delim=dflt_v<CH>::digits()[21]):argv_zz(0)
	{
		if(delim==0)
		{
			init(_ppargv,true);
			return;
		}
		std::locale loc;
       tstringstream stream;
	   CH* p=(CH*)_ppargv;
	   if(p==0) return;
	   while(*p)
	   {
		   while(std::isspace(*p,loc))
			   p++;
		   if(*p==0) break;
		   if(*p!=delim) stream<<(*p);
		   else stream<<CH(0);
		   ++p;

	   }

	    stream<<CH(0);
	   init(stream.str().c_str(),true);
	};

};

#undef GetEnvironmentStrings



template <class CH,typename LIST=CH** ,typename Setter=argzz_setter<CH> >
struct argv_env:argv_zz<CH,Setter,LIST>
{

	/*
	void env_set(char* c)	{  FreeEnvironmentStringsA((CH*)init(GetEnvironmentStrings(),true));};
	void env_set(wchar_t* c)
	{ 
		HANDLE htok=0;

	if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY|TOKEN_DUPLICATE,&htok)) 
		return ; 
      if(CreateEnvironmentBlock((void**)&c,htok,true))
	  {
            init(char_mutator<CP_THREAD_ACP,true>(c),true);
			DestroyEnvironmentBlock(c);
	  }
	  CloseHandle(htok);
		//	  FreeEnvironmentStringsW((CH*)init(GetEnvironmentStringsW(),true));
	};
	*/
      void env_init()
	  {
		  LPWCH pc=GetEnvironmentStringsW();
		  init(char_mutator<CP_THREAD_ACP,true>(pc),true);
		  FreeEnvironmentStringsW(pc);
		  
	  };	
	argv_env()
	{
		 env_init();
	 //env_set((wchar_t*)0);	
	//	env_set((char*)0);	
	}
};
template <class CH>
struct argv_env_r:argv_env<CH,CH**,fake_setter<CH>>{};


template <class CH,int CP_XXX=CP_THREAD_ACP,typename LIST=CH** ,typename Setter=argzz_setter<CH> >
struct argv_cmdline:argv_zz<CH,Setter,LIST>
{
	
	void cmdline_set(wchar_t* c)
	{ 
		int nc=0;
		wchar_t** pp=CommandLineToArgvW(c,&nc);
		{
		  argv_def<wchar_t> tmp(nc,pp);
		  convert(tmp,CP_XXX);
		}
		if(pp) LocalFree(pp);

		//	init(char_mutator<CP_XXX,true>(c),true);
		
		
		//	  FreeEnvironmentStringsW((CH*)init(GetEnvironmentStringsW(),true));
	};
	argv_cmdline(const CH* c)
	{
		cmdline_set(char_mutator<CP_XXX,true>(c));	
	}
};


template <class CH,typename LIST=CH** ,typename Setter=fake_setter<CH> >
struct argv_ini:argv_zzbase<CH,Setter,LIST>
{

	v_buf_fix<CH,32767> buf;
	HMODULE m_hm;

	  bufT<CH> get_module_name()
	{
	
		return  get_module_name(m_hm);
	};

	  v_buf<CH>& get_module_path(v_buf<CH>& buf=v_buf<CH>())
	  {
		  char_mutator<CP_THREAD_ACP> cm(get_module_name().get(),1);
		  v_buf<wchar_t> tmp(2048);
           wchar_t* p=0;
		  if(GetFullPathNameW(cm,2048,tmp,&p))
		  {
			  *p=0;
               buf.clear().cat(tmp.get()); 
		  }
		  return  buf;
	  };
	  


 static HINSTANCE get_hinstance(void* p=&get_hinstance)
 {
       HINSTANCE h=0;
	 GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|
		 GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)p,&h);
	 return h;
 }

 static  bufT<CH> get_module_name(HMODULE hm)//=get_hinstance())
 {
     bufT<CH> buf;
	 get_module_name(hm,buf,buf.size);
	 return buf;
 };

  static  int get_module_name(HMODULE hm,wchar_t* p,int size)
  {
          return  GetModuleFileNameW(hm,p,size);
  }
  static  int get_module_name(HMODULE hm,char* p,int size)
  {
	  return  GetModuleFileNameA(hm,p,size);
  }
  static  int get_pp_section(const char* sec,char* fn,char* p,int size)
  {
       return  GetPrivateProfileSectionA(sec,p,size,fn); 
  }
  static  int get_pp_section(const wchar_t* sec,wchar_t* fn,wchar_t* p,int size)
  {
      return  GetPrivateProfileSectionW(sec,p,size,fn); 
  }


	   //std::vector<CH*> ppvect;
          argv_ini(HMODULE hm=get_hinstance()):m_hm(hm){};
         argv_ini(const CH* section,const CH* ext,HMODULE hm=get_hinstance())
		 {
              init(section,ext,hm);
		 }


     argv_ini& init_from_file(const CH* section,const CH* file)
		{
			                  
			   size_t sz;	   
			   
                 sz=get_pp_section(section,(CH*)file,buf,buf.count());
               if(sz==0) return *this;			   
			   CH *p0=buf;
			   sz=safe_len_zerozero(buf.get());
			   buf.v.resize(sz+2);
			   CH *p2=buf;
			   sz=set_args(buf);			   
			   return *this;

		}



     argv_ini& init(const CH* section,const CH* ext=char_mutator<>(L".ini"),HMODULE hm=get_hinstance())
		{
			   m_hm=hm; 
               bufT<CH,1024> mn;
			   CH* p=mn;
			   size_t sz;
			   if(hm!=HMODULE(-1))
			   {
    			   if(get_module_name(hm,mn,mn.count)==0) return *this;
                 sz=safe_len(p);
	 		   } 
			   else sz=0;

			   safe_copy(p+sz,ext,safe_len(ext));
			   return init_from_file(section,p);
			   /*
			   
                 sz=get_pp_section(section,p,buf,buf.count());
               if(sz==0) return *this;
			   
			   CH *p0=buf;
			   sz=safe_len_zerozero(buf.get());
			   buf.v.resize(sz+2);
			   CH *p2=buf;

			   sz=set_args(buf);
			   
			   
			   return *this;
			   */

			   /*
			    args_ZZ<CH>(buf,&sz,ppvect); 
				buf.v.resize(sz+2);
				argc=ppvect.size();
				if(argc) _pv_=&ppvect[0]; 
				*/

		}
  
};

template <class CH,typename LIST=CH** ,typename Setter=fake_setter<CH> >
struct argv_reg:argv_zz<CH,Setter,LIST>
{
        

inline LONG reg_enum_value(
						          HKEY hKey,
						          DWORD dwIndex,
						          char* lpValueName,
						          LPDWORD lpcchValueName,
						          LPDWORD lpReserved,
							      LPDWORD lpType,
						 	      LPBYTE lpData,
						          LPDWORD lpcbData
						 )
{
	return RegEnumValueA(hKey,dwIndex,lpValueName,lpcchValueName,lpReserved,lpType,lpData,lpcbData); 
}
inline LONG reg_enum_value(
						   HKEY hKey,
						   DWORD dwIndex,
						   wchar_t* lpValueName,
						   LPDWORD lpcchValueName,
						   LPDWORD lpReserved,
						   LPDWORD lpType,
						   LPBYTE lpData,
						   LPDWORD lpcbData
						   )
{
	return RegEnumValueW(hKey,dwIndex,lpValueName,lpcchValueName,lpReserved,lpType,lpData,lpcbData); 
}

inline LONG reg_open_key_ex( HKEY hKey,const wchar_t* lpSubKey,DWORD ulOptions,REGSAM samDesired,HKEY* pkey)
{
 return RegOpenKeyExW(hKey,lpSubKey,ulOptions,samDesired,pkey);
}

inline LONG reg_open_key_ex( HKEY hKey,const char* lpSubKey,DWORD ulOptions,REGSAM samDesired,HKEY* pkey)
{
	return RegOpenKeyExA(hKey,lpSubKey,ulOptions,samDesired,pkey);
}

       argv_reg(){};
	   argv_reg(const CH* subkey,HKEY hkroot=HKEY_CURRENT_USER )
	   {
		   init(subkey,hkroot);
	   }
   argv_reg& init(const CH* subkey,HKEY hkroot=HKEY_CURRENT_USER )
	   {
		   tstringstream buf_stream;
		   v_buf<CH> name(32760/sizeof(CH));
		   v_buf<BYTE>  value(32760);

           HKEY hk=0;
		  if(ERROR_SUCCESS!= reg_open_key_ex(hkroot,subkey,0,KEY_READ,&hk))
			  return *this;
           HRESULT hr;
          CH sequ[2]={dflt_v<CH>::equ,0};
		  for(int i=0;;++i)
		  {
              DWORD cbn=name.count(),cbv=value.size_b(),typs=0;  
            hr=reg_enum_value(hk,i,name.get(),&cbn,0,&typs,value.get(),&cbv);
			CH* pname=name;
            CH* pvalue=(CH* )value.get();
            if(ERROR_SUCCESS!=hr) break; 
			name[cbn]=0;
			if((typs==REG_SZ)||(typs==REG_EXPAND_SZ))
			{
               buf_stream<<name.get()<<sequ;
			   buf_stream.write((CH*)value.get(),cbv/sizeof(CH));
			}
		  }

		  RegCloseKey(hk);
		  buf_stream<<CH(0);
		    argv_zz<CH>::init((CH*)buf_stream.str().c_str(),true);
			return *this;
  	  	   };

};


template <class CH,typename LIST=CH** ,typename Setter=fake_setter<CH> >
struct argv_triplet:argv_env<CH,CH**,fake_setter<CH> >
{
      argv_ini<CH,LIST,Setter> argini;
	  argv_reg<CH,LIST,Setter> argreg;

	  argv_triplet(const CH* subkey,const CH* section,HKEY hkroot=HKEY_CURRENT_USER)
	  {
        init(subkey,section,hkroot);
	  }
 argv_triplet&		  init (const CH* subkey,const  CH* section,HKEY hkroot=HKEY_CURRENT_USER)
	  {
         
		 argini.init(section,char_mutator<>(L".ini"));
		 argreg.init(subkey,hkroot);
		 this->parent=&argini;
		 argini.parent=&argreg;		 
		 return *this;
	  }



};

template <typename F>
struct RectT
{
	F left;
	F top;
	F right;
	F bottom;
	operator RECT()
	{
		RECT r={left,top,right,bottom};
		return r;
	}
     RectT(RECT r)
	 {
		 left=r.left;
		 top=r.top;
		 right=r.right;
		 bottom=r.bottom;
	 }
	 RectT(F l=F(),F t=F(),F r=F(),F b=F()):left(l),top(t),right(r),bottom(b){};

};


template <class CH>
struct s_parser_t
{
	typedef  unsigned int uint32_t;
	typedef char char_t;



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


	inline static int str_cmp_ni(const wchar_t* p,const wchar_t* ps,int cb)
	{
		if(p==ps) return 0;
		if(p==0) return -1;
		if(ps==0) return 1;
		return _wcsnicmp(p,ps,cb);
	}

	inline  static int str_cmp_ni(const char_t* p,const char_t* ps,int cb)
	{
		if(p==ps) return 0;
		if(p==0) return -1;
		if(ps==0) return 1;
		return _strnicmp(p,ps,cb);
	}


	inline static int str_cmp_n(const wchar_t* p,const wchar_t* ps,int cb)
	{
		if(p==ps) return 0;
		if(p==0) return -1;
		if(ps==0) return 1;
		return wcsncmp(p,ps,cb);
	}

	inline  static int str_cmp_n(const char_t* p,const char_t* ps,int cb)
	{
		if(p==ps) return 0;
		if(p==0) return -1;
		if(ps==0) return 1;
		return strncmp(p,ps,cb);
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

	v_buf<CH>& get_cache(int siz=0)
	{
		cache_list.push_front(v_buf<CH>());	 
		v_buf<CH>& buf=cache_list.front();
		if(siz) buf.resize(siz);
		return buf;
	}

	v_buf<CH>& get_cache(const CH* str)
	{
		return get_cache().cat(str);
	}


	template <class CX>
	v_buf<CH>& get_cache_encode(const CX* str,int CP_XX=CP_UTF8)
	{
		return get_cache().cat((CH*)char_mutator<>(str,0,0,CP_XX));
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
		v_buf<CH>& buf=get_cache();
		return _trim(buf.cat(p).get());
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
			else if(*s==ket)
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
        return undecorate(undecorate(trim(str),L'\'',L'\''),L'\"',L'\"');
	}

	char* undecorateQQ(const char* str)
	{
		return undecorate(undecorate(trim(str),L'\'',L'\''),L'\"',L'\"');
	}


	template <bool fcasesens=true>
	struct case_t
	{
		const CH* s;
		int len;
		case_t(const CH* _s):s(_s),len(safe_len(s)){};
		case_t(const CH* _s,int l):s(_s),len(l){};

		bool eqlen(const CH* p,int l)
		{
			int i=(fcasesens)?str_cmp_n(s,p,l):str_cmp_ni(s,p,l);
			return i==0;

		}

		bool eq(const CH* p)
		{
			return eqlen(p,len);

		}



		bool operator()(int l,const CH* s1)
		{
			return eqlen(s1,l);
		}




		bool operator()(const CH* s1)
		{
			return eq(s1);
		}

		bool operator()(const CH* s1,const CH* s2)
		{
			return eq(s1)||eq(s2);
		}
		bool operator()(const CH* s1,const CH* s2,const CH* s3)
		{
			return eq(s1)||eq(s2)||eq(s3);
		}

		bool operator()(const CH* s1,const CH* s2,const CH* s3,const CH* s4)
		{
			return eq(s1)||eq(s2)||eq(s3)||eq(s4);
		}


	};


  std::list< v_buf<CH> > cache_list;

};