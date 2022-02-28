#pragma once

#include "windows.h"
//#include "os_utils.h"
#include "dispex.h"
#include <comdef.h>
//#include <atlbase.h>
//#include <atlcom.h>
//#include <atldef.h>

//#include "ipc.h"
#include <vector>
#include <algorithm>

#include "../pp/ppcall.h"
#include "../com/com_utils.h"


#define JS_LTX_DBG L":dbg:1"


#define JS_LTX_STA  L"dich-ltx:sta:script"
#define JS_LTX_STD      L"dich-ltx:both:script"
#define JS_LTX_SRV  L"dich-ltx-srv:both:script"
#define JS_LTX_FREE  L"dich-ltx:free:script"

#define JS_LTX_STA_DBG JS_LTX_STA JS_LTX_DBG
#define JS_LTX_STD_DBG JS_LTX_STD JS_LTX_DBG
#define JS_LTX_SRV_DBG JS_LTX_SRV JS_LTX_DBG

#define CHECK_FAIL_(hr,a) if(FAILED(hr=(a))) \
	return hr;
#define CHECK_FAIL_hr(a)  CHECK_FAIL_(hr,a)

//#define COMINIT_F LifeTimeX_Dispatch_base<NULL_DispatchHolder>::COMInit ccccc2222(COINIT_MULTITHREADED);
//#define COMINIT_A LifeTimeX_Dispatch_base<NULL_DispatchHolder>::COMInit ccccc222(COINIT_APARTMENTTHREADED);

#define COMINIT_F LifeTimeX_Dispatch_base::COMInit ccccc2222(COINIT_MULTITHREADED);
#define COMINIT_A LifeTimeX_Dispatch_base::COMInit ccccc222(COINIT_APARTMENTTHREADED);


struct COMInit_tls_blank {};



template<typename MultiThreadHolder, typename DP, typename DPEx>
struct DispatchHolderT
{
	typedef typename DP DispPtrType;
	typedef typename DPEx DispPtrTypeEx;
	//typedef typename MultiThreadHolder HolderType;
	struct safeHolderType
	{

		inline 	safeHolderType& operator=(IDispatch* p)
		{
			m_th=p;
			return *this;
		}
		inline HRESULT CopyTo(IDispatch** pp) const 
		{
			return m_th.CopyTo(pp);
		}
		safeHolderType(){};

inline	safeHolderType& operator=(const safeHolderType& git)
		{
              if(DWORD(git.m_th))   m_th=git.m_th;
			  return *this;
		}
		safeHolderType(const safeHolderType& git)
		{
			//safe_assign(&m_th,&git.m_th);
			if(DWORD(git.m_th))  m_th=git.m_th;
		}
		MultiThreadHolder m_th;
	};

	//typedef typename safeHolderType HolderType;
	typedef typename MultiThreadHolder HolderType;

inline static bool safe_assign(HolderType* pd,HolderType const * ps)
{
	//if((ps)&&DWORD(*ps))
	if((ps))
	{
		*pd=*ps;
     return true;
	}
	else return false; 
}

};

//typedef DispatchHolderT<CComGITPtr<IDispatch>,CComPtr<IDispatch>,CComPtr<IDispatchEx> > ATL_DispatchHolder;
typedef DispatchHolderT<com_GIT_ptr<IDispatch,_com_error>,com_ptr<IDispatch>,com_ptr<IDispatchEx> > com_DispatchHolder;
typedef DispatchHolderT<int,int,int> NULL_DispatchHolder;

//typename ltx_script::variant_type bind(char* parsestr,char* ltx_mode=JS_LTX_STD)
#define DISPGIT ATL_DispatchHolder
#define DISPGIT com_DispatchHolder
//
template<typename ltx_script>
typename ltx_script::variant_type bind(typename ltx_script::string_type parsestr,typename ltx_script::string_type ltx_mode=JS_LTX_STD)
{
	ltx_script ltxscr(ltx_mode);
	return ltxscr(ltx_script::string_type(L"ltx_bind('")+parsestr+L"')");
}



template<typename ltx_script, typename N >
typename ltx_script::variant_type wrap_callback( N proc,void* context=0,int calltype=3)
{
	ltx_script ltxscr(JS_LTX_STD);
	ltx_script::variant_type wrap_o=ltxscr("ltx_bind('proc_wrap')($0,$1,$2)",ltx_script::IntPtr(proc),ltx_script::IntPtr(context),calltype);
	return wrap_o;
}

#ifdef _MSC_VER

template<typename ltx_script, typename N >
typename ltx_script::variant_type wrap_method( N proc,void* pthis) // MS VS only!!!!
{
  return wrap_callback<ltx_script>(proc,pthis,0);
}
#endif

//template<class DispatchHolder=DISPGIT>
struct LifeTimeX_Dispatch_base
{
/*
	typedef typename DispatchHolder::HolderType DispatchHolderType;
	typedef typename DispatchHolder::DispPtrType DispatchPtrType;
	typedef typename DispatchHolder::DispPtrTypeEx DispatchPtrTypeEx;
*/

	class COMInit
	{
	public:
		static void tss_free(COMInit* p)
		{
             if(p) delete p;       
		}
		HRESULT m_hr;
		COMInit(DWORD dw= COINIT_MULTITHREADED /*COINIT_APARTMENTTHREADED*/ )
		{
			m_hr=CoInitializeEx(NULL,dw);
		}
		inline HRESULT reinit(DWORD dw= COINIT_MULTITHREADED)
		{
			if(FAILED(m_hr)) m_hr=CoInitializeEx(NULL,dw);
			return m_hr;
		}
		~COMInit()
		{
			//
			if(SUCCEEDED(m_hr))
				CoUninitialize();
		}
		inline operator HRESULT() { return m_hr;}
	};


	static HRESULT com_thread_initialize(DWORD dw= COINIT_MULTITHREADED)
	{

		struct ltx_proc_table_type
		{
			typedef  LONG (  __cdecl *PEXTERNAL_CONTROLLER_PROC)(bool fconnect,long strong_lock_count); 
			void* (__stdcall *ltx_on_exit_thread)(void* proc,void *ptr);
			void *rsv1,*rsv2;
			void* ( __stdcall *ltx_SetHardExternalControllerMonitor)(PEXTERNAL_CONTROLLER_PROC pp,LONG flags);
			ULONG ( __stdcall *ltx_StrongLockObjectsCount)();
			HRESULT (__stdcall *ltx_TlsArgs)(IDispatch** ppdisp);
		};


		COMInit* pci=(COMInit*) tls_var<COMInit_tls_blank>();
		if(!pci)
		{
			pci= new  COMInit(dw);

			if(SUCCEEDED(*pci)) 
			{
				HRESULT hr;
				HMODULE hm;	
				//LifeTimeX_Scriptor_T<com_variant,com_string,com_exception>
				com_ptr<IUnknown>  script;
				hr=::CoGetObject(JS_LTX_STD,0,__uuidof(IUnknown),(void**)&script.p);

				if(FAILED(hr)) return delete pci,hr;
				::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN,"LifeTimeX.dll",&hm);// immortal DLL
				//int itable=(int)ltxscr(L"ltx_bind('global').export_table_int_ptr");
				void* p=GetProcAddress(hm,"ltx_proc_table");
				ltx_proc_table_type * ptt=*((ltx_proc_table_type **)p);

				if(!ptt) return delete pci,E_POINTER;
				//
				(ptt->ltx_on_exit_thread)(&COMInit::tss_free,pci);   

				tls_var<COMInit_tls_blank>()=pci;
				script.Release();

			}
		}

		return  pci->reinit(dw);
	}



	template<typename N>
	static	inline int IntPtr(N pr)
	{
		union
		{
			N t;
			int k;
		} u;
		u.t=pr;
		return u.k;
	};
	template<typename N>
	static	inline int IntPtrEncode(N pr)
	{
		void* p=(void*) IntPtr(pr);
		return (int)EncodePointer(p);
	}


	class ExcepInfo :
		public EXCEPINFO
	{

	public:
		ExcepInfo()
		{

			memset( this, 0, sizeof( *this ) );

		}
#define __BSTR_cpy(a) if(a) bstr##a=::SysAllocString(a);

		ExcepInfo(BSTR Description,BSTR Source=0,BSTR HelpFile=0)
		{
			memset( this, 0, sizeof( *this ) );
			__BSTR_cpy(Description);
			__BSTR_cpy(Source);
			__BSTR_cpy(HelpFile);

		}

		~ExcepInfo()
		{
			Clear();
		}

		void Clear()
		{
			::SysFreeString(bstrSource);
			::SysFreeString(bstrDescription);
			::SysFreeString(bstrHelpFile);

			memset(this, 0, sizeof(*this));
		}
	};

	struct ErrorHelper
	{
		EXCEPINFO& m_ei;
		GUID m_iid;
		ErrorHelper(EXCEPINFO& ei,GUID iid=GUID_NULL ):m_ei(ei),m_iid(iid){};

		template <class I>
		inline HRESULT	get_error_info(I** ppIntf)
		{	HRESULT hr;
		if(!ppIntf) return E_POINTER;
		//CComPtr<IErrorInfo> ei;
		IErrorInfo* ei=0;
		CHECK_FAIL_hr(get_error_info(&ei));
		hr=ei->QueryInterface(__uuidof(I),(void**)ppIntf);
		if(ei) ei->Release();
		return hr;
		}

		inline HRESULT	get_error_info(IErrorInfo** ppIntf,bool fandset=true)
		{ 
			HRESULT hr;

			if(!ppIntf) return E_POINTER;

			//CComPtr<ICreateErrorInfo> cei;
			ICreateErrorInfo* cei;
			CHECK_FAIL_hr(CreateErrorInfo(&cei));

			(cei->SetGUID(m_iid));
			(cei->SetSource(m_ei.bstrSource));
			(cei->SetDescription(m_ei.bstrDescription));
			(cei->SetHelpFile(m_ei.bstrHelpFile));
			(cei->SetHelpContext(m_ei.dwHelpContext));
			hr=cei->QueryInterface(__uuidof(IErrorInfo),(void**)ppIntf);
			if((fandset)&&(SUCCEEDED(hr))) SetErrorInfo(0,*ppIntf);
			if(cei) cei->Release();
			return hr;
		}



	};

/*
	inline bool  IsComPresents(bool fchekonly=1,DWORD appart=COINIT_MULTITHREADED)
	{
		return 1;
		HRESULT hr = CoInitializeEx(NULL,appart);
		if(fchekonly||(hr)) CoUninitialize();
		return hr;
	}

*/

	static	inline VARIANT NOP()
	{
		VARIANT v={VT_ERROR};
		v.scode=DISP_E_PARAMNOTFOUND;
		return v;
	}
	inline bool is_optional(const VARIANT& v)
	{
		return (v.vt==VT_ERROR)&&(v.scode==DISP_E_PARAMNOTFOUND);
	}


	//HRESULT m_hr;
	//IDispatch* m_pscript;
	



	/*
	inline com_string moniker_string(com_string appart,int dbg_mode=0)
	{
	char buf[256];

	}
	*/

	/*
	LifeTimeX_Scriptor_base():m_hr(E_POINTER)
	{
	}
	~LifeTimeX_Scriptor_base()
	{
	//if((m_pscript)&&IsComPresents())
	//	m_pscript->Release();
	}
	*/

/*
	inline DWORD Detach() 
	{
		return m_script.Detach();
	}

	inline void Attach(DWORD c) 
	{

		m_script.Attach(c);
	}
*/

	//__declspec(property(get=get_cookie, put=set_cookie)) DWORD cookie;

inline static HRESULT  get_dispid_ex(IDispatch* p,BSTR name,DISPID* pid,DWORD grfdex=fdexNameEnsure|fdexNameCaseInsensitive)
	 {
		 HRESULT hr;
              hr=p->GetIDsOfNames(IID_NULL,&name,1, LOCALE_USER_DEFAULT,pid);
        if(FAILED(hr))
		{
			HRESULT hr2;
           IDispatchEx* pdex=0;
		   if(SUCCEEDED(hr2=p->QueryInterface(__uuidof(IDispatchEx),(void**)&pdex)))
		   {
                 hr=pdex->GetDispID(name,grfdex,pid);
				 pdex->Release();
		   }
		}
		return hr;
	 }

   //
	 template < typename _PVAR>
static   	 inline HRESULT invoke(IDispatch* pscript, VARIANT& txt,int argc,_PVAR pargv,VARIANT *pret=NULL,EXCEPINFO* pex=NULL,int mode=3,UINT flags=DISPATCH_METHOD|DISPATCH_PROPERTYGET)
	//  inline HRESULT invoke(VARIANT& txt,int argc,VARIANT *pargv,VARIANT *pret=NULL,EXCEPINFO* pex=NULL,int mode=3,UINT flags=DISPATCH_METHOD|DISPATCH_PROPERTYGET)
	{
		HRESULT hr;

		//CComPtr<IDispatch> 
   	//		DispatchPtrType pscript;
    //		CHECK_FAIL_hr(m_script.CopyTo(&pscript)); 
		//if(!m_pscript) return m_hr=E_POINTER;
		DISPID putid = DISPID_PROPERTYPUT, *pdispNP=NULL;
		int named_argc=0;
		VARIANT tmp={VT_EMPTY};

		struct dynVariant
		{
           VARIANT* pv;
		   dynVariant():pv(0){};
		   ~dynVariant(){if(pv) free(pv);}
		};

		dynVariant dyv;

//				std::vector<VARIANT> vlist;
//		vlist.resize(argc+2);
		DISPID dispid=DISPID_VALUE;
		

		 VARIANT vvlist[16];
        VARIANT* it;

        //std::vector<VARIANT>::iterator=vlist.begin()+1;
     
		 if(argc<15)  it=&(vvlist[1]);
		 else 
		 {
              dyv.pv= (VARIANT*)malloc((argc+2)*sizeof(VARIANT));    //new VARIANT[argc+2];
			  it=dyv.pv+1;
		 }

		 copy_to_collection(argc,pargv,it,(mode&1)!=0);
		//if(mode&1) std::reverse_copy(pargv,pargv+argc,it);
		//else std::copy(pargv,pargv+argc,it);

         DISPPARAMS dp = {it,NULL,argc,0};

        bool fput=(flags&(DISPATCH_PROPERTYPUTREF|DISPATCH_PROPERTYPUT));
		if(fput)
		{
            pdispNP=&putid;
			named_argc=1;
            //vlist[0]=*pret;
			--dp.rgvarg;
             *( dp.rgvarg)=*pret;
			++dp.cArgs;
			dp.rgdispidNamedArgs=&putid;
			dp.cNamedArgs=1;
			pret=&tmp;
		}
         else if(!pret) pret=&tmp;
		
        if(mode&2)
		{
			//VARIANT& vf=vlist[dp.cArgs++];
			VARIANT& vf=*(dp.rgvarg+dp.cArgs);   //vlist.back();
			vf=txt;
            dp.cArgs++;
		}
		else 
		{
           if(txt.vt==VT_EMPTY) dispid=DISPID_VALUE;
		   else
           if(txt.vt==VT_BSTR)
		   {
			   if(SysStringLen(txt.bstrVal))
			   {
                 
				   DWORD grfdex=(fput||(mode&0x10))?(fdexNameEnsure|fdexNameCaseInsensitive):fdexNameCaseInsensitive;
				    CHECK_FAIL_hr(get_dispid_ex(pscript,txt.bstrVal,&dispid,grfdex));
				 if(mode&8)
				 {
                    ::VariantClear(&txt); txt.vt=VT_I4;txt.intVal=dispid;
    			 }
			   }
		   }
		   else
		   {
			   VARIANT v={VT_EMPTY};
			   if((txt.vt=VT_INT)||(txt.vt=VT_I4))
				   dispid=txt.intVal;
			   else
			   {
               CHECK_FAIL_hr(VariantChangeType(&v,&txt,0,VT_I4));
			   dispid=v.intVal;
			   }
		   }
		}

		//DISPPARAMS dp = { &(vlist[0]),pdispNP,argc,named_argc};
		hr=pscript->Invoke(dispid,IID_NULL, LOCALE_USER_DEFAULT,flags, &dp, pret,pex, NULL);
		::VariantClear(&tmp);
		return hr;
	}
};

template <class com_variant,class com_string,class com_exception,class DispatchHolder=DISPGIT>
struct LifeTimeX_Scriptor_base_T:LifeTimeX_Dispatch_base //<DispatchHolder>
{

	typedef typename DispatchHolder::HolderType DispatchHolderType;
	typedef typename DispatchHolder::DispPtrType DispatchPtrType;
	typedef typename DispatchHolder::DispPtrTypeEx DispatchPtrTypeEx;

    typedef typename  com_variant       variant_type;
	typedef typename  com_string        string_type;
	

inline static com_string unique_str(wchar_t* prfx=L"vbf")
	{
	  uuid_t uid;
	 wchar_t r[128];
	 CoCreateGuid(&uid);
	 wsprintfW(r,L"%s%08X%08X%08X%08X",prfx,uid);
	return r;
	};
	

	inline	static void check_error(HRESULT hr,EXCEPINFO* pei=0)
	{
		if(FAILED(hr))
		{
			IErrorInfo *perrinfo=0;
			if(pei) ErrorHelper(*pei).get_error_info(&perrinfo);
			throw  com_exception(hr,perrinfo);
		}
	};


template <typename _PVAR >
	inline VARIANT& exec_except(com_variant txt_id,int argc=0,_PVAR pargv=0,int mode=3,VARIANT* pr=&com_variant(),UINT flags=DISPATCH_METHOD|DISPATCH_PROPERTYGET)
	{
		ExcepInfo ei;
		//DispatchPtrType pscript;
		//check_error(m_script.CopyTo(&pscript.p)); 
		check_error(invoke(DispatchPtrType(m_script),txt_id,argc,pargv,pr,&ei,mode,flags),&ei);
		return *pr;
	};
	

	inline operator variant_type()
	{

          DispatchPtrType  dispscr;
    	check_error(m_script.CopyTo(&dispscr.p));
		return variant_type(dispscr);
	}

inline DispatchPtrType as_DispatchPtr(variant_type const & vo)
{
	variant_type t;
   if(vo.vt==VT_DISPATCH) 
		 return DispatchPtrType(vo.pdispVal);
	check_error(VariantChangeType(&t,&vo,0,VT_DISPATCH));
    return DispatchPtrType(t.pdispVal);
}

DispatchHolderType m_script;
};






template <class com_variant,class com_string,class com_exception,class DispatchHolder=DISPGIT>
struct LifeTimeX_Function_base_T:LifeTimeX_Scriptor_base_T<com_variant,com_string,com_exception,DispatchHolder>
,invoke_with_helper<LifeTimeX_Function_base_T<com_variant,com_string,com_exception,DispatchHolder>,com_variant,com_variant,1>
{
/*
LifeTimeX_Function_base_T(IDispatch* pfunct,DispatchHolder script=DispatchHolder())
{
 m_script=pfunct;
 m_namespace=script;
};
*/
typedef LifeTimeX_Scriptor_base_T<com_variant,com_string,com_exception,DispatchHolder>   object_base_type;
typedef LifeTimeX_Function_base_T<com_variant,com_string,com_exception,DispatchHolder> function_type;

template <int argc>
struct invoke_helper_stack:_helper_base_type_stack
{
    invoke_helper_stack(owner_type* t,int cb):_helper_base_type_stack(t){};

	inline    ret_type  operator()(arg_type* p,...)
	{
		//VARIANT buf[argc+2];

		return owner().exec_except(owner().get_id_name(),argc,pargv(&p),1);

	}

	inline    ret_type  operator()()
	{
		return owner().exec_except(owner().get_id_name(),0,pargv(),1,&com_variant(),DISPATCH_METHOD);
	}

};
/*
typedef	struct invoke_helper:_helper_base_type
{
	invoke_helper(function_type* t,int cb):_helper_base_type(t,cb){};
	inline    ret_type  operator()(arg_type* p,...)
	{
    	 //cargv pargv(&p);
		 return owner().exec_except(owner().get_id_name(),count(),pargv(&p),1);
		 
	}

	inline    ret_type  operator()()
	{
           return owner().exec_except(owner().get_id_name(),0,pargv(),1,&com_variant(),DISPATCH_METHOD);
	}


} invoke_helper_type;
*/

inline variant_type&  get_id_name()
{
	return m_id_name;
}



struct geter_puter
{
    object_base_type  & m_obj;
	com_variant   m_id;
	int m_mode;
	geter_puter(object_base_type  & obj,com_variant  & id,int mode=1):m_obj(obj),m_id(id),m_mode(mode){};
	
inline geter_puter& operator =(com_variant v)
{
   object_base_type::ExcepInfo ei;
   if(m_mode&2)
            //m_obj.check_error(m_obj.invoke(com_variant(com_string(m_id)+L"=$0"),1,&v,0,&ei,m_mode),&ei);
			 m_obj.exec_except(com_variant(com_string(m_id)+L"=$0"),1,&v,m_mode);
      else	m_obj.exec_except(m_id,0,refref<com_variant>(),m_mode,&v,DISPATCH_PROPERTYPUT);
   return *this;
}

/// HZ CHO .....

#define as_object _call()
#define as_function _call()
inline function_type _call()
{
 return function_type(m_obj,&m_obj.m_script,m_id);
}

template <typename N>
inline N cast_type(com_string* pf)
{
	return cast_type<com_variant>((com_variant*)0);
}


template <typename N>
inline N cast_type(function_type* pf)
{
	return (m_mode&2)? function_type(com_variant(*this),&m_obj.m_script)
		: function_type(com_variant(m_obj),&m_obj.m_script,m_id);
	//return function_type(com_variant(m_obj),&m_obj.m_script,m_id);
}


template <typename N>
inline N cast_type(...)
{
	object_base_type::ExcepInfo ei;
	com_variant r;
	int flags=(m_mode&2)?DISPATCH_METHOD|DISPATCH_PROPERTYGET:DISPATCH_PROPERTYGET;
	//m_obj.check_error(m_obj.invoke(m_id,0,refref<com_variant>(),&r,&ei,m_mode,flags),&ei);
	 m_obj.exec_except(m_id,0,refref<com_variant>(),m_mode,&r,flags);
	return N(r);
}

template <typename N>
inline operator N()
{
   return cast_type<N>((N*)0);
}
/*
inline operator function_type()
{
	 return (m_mode&2)? function_type(com_variant(*this),&m_obj.m_script)
		              : function_type(com_variant(m_obj),&m_obj.m_script,m_id);
	
};


template <typename N>
inline operator N()//com_variant()
{
	object_base_type::ExcepInfo ei;
	com_variant r;
	int flags=(m_mode&2)?DISPATCH_METHOD|DISPATCH_PROPERTYGET:DISPATCH_PROPERTYGET;
	m_obj.check_error(m_obj.invoke(m_id,0,NULL,&r,&ei,m_mode,flags),&ei);
	return N(r);

}

*/

};



inline geter_puter operator[](com_variant name)
{
	name.ChangeType(VT_BSTR);
	return geter_puter(*this,name);
}

inline void init(com_variant const & funct,DispatchHolderType*  pscript,com_variant& const nid)
{
	m_id_name=nid;
	m_script=as_DispatchPtr(funct);
	//if(pscript) m_namespace=*pscript;
	DispatchHolder::safe_assign(&m_namespace,pscript);
}

LifeTimeX_Function_base_T(com_variant const & funct,DispatchHolderType* pscript=NULL,com_variant  nid=int(DISPID_VALUE))
{
	init(funct,pscript,nid);
};

/*
inline com_variant operator()()
{
	return exec_except(m_id_name,0,0,1);
}
inline com_variant operator()(com_variant vb)
{
	return exec_except(m_id_name,1,&vb,1);
}
inline com_variant operator()(com_variant vb,com_variant ve)
{
	//return exec_except(txt,std::distance(&vb,&ve+1),&vb);
	return exec_except(m_id_name,2,&vb,1);
}
inline com_variant operator()(com_variant vb,com_variant v1,com_variant ve)
{
	return exec_except(m_id_name,3,&vb,1);
}
inline com_variant operator()(com_variant vb,com_variant v1,com_variant v2,com_variant ve)
{
	return exec_except(m_id_name,4,&vb,1);
}
inline com_variant operator()(com_variant vb,com_variant v1,com_variant v2,com_variant v3,com_variant ve)
{
	return exec_except(m_id_name,5,&vb,1);
}
inline com_variant operator()(com_variant vb,com_variant v1,com_variant v2,com_variant v3,com_variant v4,com_variant ve)
{
	return exec_except(m_id_name,6,&vb,1);
}
inline com_variant operator()(com_variant vb,com_variant v1,com_variant v2,com_variant v3,com_variant v4,com_variant v5,com_variant ve)
{
	return exec_except(m_id_name,7,&vb,1);
}
inline com_variant operator()(com_variant vb,com_variant v1,com_variant v2,com_variant v3,com_variant v4,com_variant v5,com_variant v7,com_variant ve)
{
	return exec_except(m_id_name,8,&vb,1);
}
*/
variant_type m_id_name;

DispatchHolderType m_namespace;
protected:

LifeTimeX_Function_base_T(){};



};

template <class com_variant,class com_string,class com_exception,class DispatchHolder=DISPGIT>
struct LifeTimeX_array_T:LifeTimeX_Function_base_T<com_variant,com_string,com_exception,DispatchHolder>
{
	inline geter_puter operator[](com_variant name)
	{
		name.ChangeType(VT_BSTR);
		return geter_puter(*this,name,0x11);
	}
LifeTimeX_array_T(com_variant const & funct,DispatchHolderType* pscript=NULL,com_variant  nid=int(DISPID_VALUE))
:LifeTimeX_Function_base_T(funct,pscript,nid){};

LifeTimeX_array_T(LifeTimeX_Function_base_T const & funct)
{
	m_script=funct.m_script;
    m_namespace=funct.m_namespace;
	m_id_name=funct.m_id_name;
}
};



template <class com_variant=variant_t,class com_string=bstr_t,class com_exception=_com_error,class DispatchHolder=DISPGIT>
struct LifeTimeX_Scriptor_T:LifeTimeX_Scriptor_base_T<com_variant,com_string,com_exception,DispatchHolder>
	,invoke_with_helper_0<LifeTimeX_Scriptor_T<com_variant,com_string,com_exception,DispatchHolder>,com_string,com_variant>
{

protected:

	
public:

typedef LifeTimeX_Scriptor_T<com_variant,com_string,com_exception,DispatchHolder> scriptor_type;

typedef	struct invoke_helper:_helper_base_type
{
	invoke_helper(scriptor_type* t,int cb):_helper_base_type(t,cb){};
	inline    ret_type  operator()(com_string* txt,...)
	{
		//va_list argptr;
		//va_start(argptr,txt);
		//cargv pargv((arg_type**)argptr);
		//return owner().exec_except(*txt,count(),pargv);
		return owner().exec_except(*txt,count(),pargv(&txt,1));
	}


} invoke_helper_type;


	class arguments
	{
	protected:


		struct __INVOKEPARAMS 
		{
			DISPPARAMS* pdispparams;
			WORD wFlags;
			HRESULT hr;
			HRESULT (__stdcall *ltx_TlsArgs)(IDispatch** );
		} ;

		typedef  __INVOKEPARAMS INVOKEPARAMS;


         

		inline	   static DWORD _tls_index()
		{
		    HMODULE hm;	
			//LifeTimeX_Scriptor_T<com_variant,com_string,com_exception>
			scriptor_type ltxscr(JS_LTX_STD);
			::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN,"LifeTimeX.dll",&hm);// immortal DLL
			return (int)ltxscr(L"ltx_bind('proc_wrap').TlsIndex");
		}

	public:

inline		static DWORD tls_index()
		{
			static DWORD s_index=_tls_index();
			return s_index;
		}

inline	operator com_variant()
         {
           HRESULT hr=E_POINTER;
           INVOKEPARAMS*	ip=(INVOKEPARAMS*)::TlsGetValue(tls_index());
		   com_variant r((IDispatch*)0);
		   if(ip)  hr=ip->ltx_TlsArgs(&r.pdispVal);
		   if(FAILED(hr)) raise_error(hr);
		   return r;
         }   


inline		static  void raise_error(HRESULT hr,com_string desc=com_string(),com_string source=com_string())
		{
			if(hr==0) hr=DISP_E_EXCEPTION;
			INVOKEPARAMS*	ip=(INVOKEPARAMS*)::TlsGetValue(tls_index());
			if(ip) ip->hr=hr;
			LifeTimeX_Dispatch_base::ExcepInfo ei(desc,source);
			LifeTimeX_Dispatch_base::ErrorHelper eh(ei);
			IErrorInfo* perrinfo=0;
			eh.get_error_info(&perrinfo);
			throw  com_exception(hr,perrinfo);
		}

inline		static		 int count()
		{
			INVOKEPARAMS*	ip=(INVOKEPARAMS*)::TlsGetValue(tls_index());
			if(ip)
				return ip->pdispparams->cArgs;
			else return 0;
		}




inline	static	   int length()
		{
			return count();
		}

		inline VARIANT nop()
		{
          return LifeTimeX_Dispatch_base::NOP();
		}
		inline com_variant& operator [](int n)
		{
			static com_variant verr=nop();  
			INVOKEPARAMS*	ip=(INVOKEPARAMS*)::TlsGetValue(tls_index());
			if(ip)
			{
				int nn=ip->pdispparams->cArgs;
				if((n>=0)&&(n<nn))
					return  *( (com_variant*)  (ip->pdispparams->rgvarg+(nn-1-n)));
			}
			return verr;

		}
	};


typedef  LifeTimeX_Function_base_T<com_variant,com_string,com_exception,DispatchHolder>  function_type;
typedef  typename  function_type::geter_puter geter_puter;


inline geter_puter operator[](com_string name)
{
	//name.ChangeType(VT_BSTR);
	return geter_puter(*this,com_variant(name),3);
}


inline function_type   object(com_string txt)
{
	com_variant fun=(*this)(txt);
	return function_type(fun,&m_script);
}


inline function_type   function(com_string txt)
{
	com_variant fun=(*this)(txt);
	return function_type(fun,&m_script);
}

inline function_type   jsfunction(com_string txt)
{
	return function(com_string(L"$$=( ")+txt+L")");
}

inline function_type   vbfunction(com_string txt,com_string funname)
{
	 com_string stxt=com_string(L"vbg:\n")+txt+L"\n Set vb000111=GetRef(\""+funname+L"\")";
	 (*this)(stxt);
	com_variant fun=(*this)(L"vb000111");
	return function_type(fun,&m_script);
}


	//HRESULT hr;



	/*
	inline com_string moniker_string(com_string appart,int dbg_mode=0)
	{
	char buf[256];

	}
	*/
/*
	LifeTimeX_Scriptor_T(int c)
	{
		m_script.Attach(c);
	}

*/
inline    void Attach(variant_type vo)
	   {
           m_script=as_DispatchPtr(vo);  
	   }





	LifeTimeX_Scriptor_T(com_string moniker_string=com_string(JS_LTX_STD))
	{
		 com_thread_initialize();
		if(moniker_string.length()>0)       
		{
			DispatchPtrType  script;
			check_error(::CoGetObject(moniker_string,0,__uuidof(IDispatch),(void**)&script.p));
			m_script=script;
		}
	}

	//inline operator HRESULT(){return hr;};

	


	template<typename N >
	inline com_variant wrap_callback( N proc,void* context,com_string name=com_string(),int calltype=3)
	{

		//LifeTimeX_Scriptor_T<com_variant,com_string,com_exception>
		//scriptor_type ltxscr(JS_LTX_STD);
		//com_variant wrap_o=ltxscr("ltx_bind('proc_wrap')($0,$1,$2)",IntPtr(proc),IntPtr(context),calltype);
        //
		com_variant wrap_o=::wrap_callback<scriptor_type>(proc,context,calltype);
		if(name.length()>0) 
			(*this)(name+com_string("=$0"),wrap_o);
			return wrap_o;

	}

#ifdef _MSC_VER
	template<typename N >
	inline com_variant wrap_method( N tp,void* pthis,com_string name=com_string())
	{
		return wrap_callback(tp,pthis,name,int(0));
	}
#endif

	typedef void (__stdcall *OLE_STD_PASCAL_ARRAY_PROC_EX)(void* pcontext,const com_variant* pargs,long narglast,com_variant* Result,HRESULT *hr);

	inline com_variant wrap_raw_stdcall( OLE_STD_PASCAL_ARRAY_PROC_EX tp,void* pctx,com_string name=com_string())
	{
		return wrap_callback(tp,pthis,name,int(4));
	}
	inline com_variant exec_n(com_string txt,int argc,com_variant *pargv)
	{ 
		return   exec_except(txt,argc,pargv);

	}

	


/*
#undef PROC_TYPE_OPERATOR_DECLARE
#define PROC_TYPE_OPERATOR_DECLARE(n,m) \
inline com_variant operator()(com_string txt )
*/
//
	/*
	inline com_variant operator()(com_string txt)
	{
		return exec_except(txt);
	}
	inline com_variant operator()(com_string txt,com_variant vb)
	{
		return exec_except(txt,1,&vb);
	}
	inline com_variant operator()(com_string txt,com_variant vb,com_variant ve)
	{
		//return exec_except(txt,std::distance(&vb,&ve+1),&vb);
		return exec_except(txt,2,&vb);
	}
	inline com_variant operator()(com_string txt,com_variant vb,com_variant v1,com_variant ve)
	{
		return exec_except(txt,3,&vb);
	}
	inline com_variant operator()(com_string txt,com_variant vb,com_variant v1,com_variant v2,com_variant ve)
	{
		return exec_except(txt,4,&vb);
	}
	inline com_variant operator()(com_string txt,com_variant vb,com_variant v1,com_variant v2,com_variant v3,com_variant ve)
	{
		return exec_except(txt,5,&vb);
	}
//*/

};


template <class com_variant,class com_string,class com_exception,class DispatchHolder=DISPGIT>
struct LifeTimeX_object_T:LifeTimeX_Function_base_T<com_variant,com_string,com_exception,DispatchHolder>
{
	typedef LifeTimeX_Scriptor_T<com_variant,com_string,com_exception,DispatchHolder> scriptor_type;

	LifeTimeX_object_T(com_string txt=L"{}",scriptor_type scriptor=scriptor_type(JS_LTX_STD))
	{
		/*com_variant funct*/
        com_variant funct=scriptor.jsfunction(txt);
		init(funct,&scriptor.m_script,com_variant());
	}

/*
    LifeTimeX_object_T(LifeTimeX_Function_base_T const & funct)
	{
         init(funct.m_script,&funct.m_namespace);
	}
	LifeTimeX_object_T(LifeTimeX_Function_base_T  & funct)
	{
		init(funct.m_script,&funct.m_namespace);
	}
*/
	//LifeTimeX_object_T(com_variant const & funct,DispatchHolderType* pscript=NULL,com_variant  nid=int(DISPID_VALUE))
	//	:LifeTimeX_Function_base_T(funct,pscript,nid){};

	
};




typedef _com_error com_exception;
typedef bstr_t  com_string;
typedef variant_t  com_variant;

typedef  LifeTimeX_Scriptor_T<variant_t,bstr_t,com_exception> ltx_script;
typedef ltx_script::function_type ltx_function;
typedef  LifeTimeX_object_T<variant_t,bstr_t,com_exception> ltx_object;
//typedef ltx_script::function_type ltx_object;
typedef LifeTimeX_array_T<variant_t,bstr_t,com_exception> ltx_array;

#define ltx_arguments ltx_script::arguments() 

#define _pn(a) #a

#define __Lpn(a) L###a
#define _Lpn(...) __Lpn(__VA_ARGS__)

#define js_text(...) __Lpn(__VA_ARGS__)

#define BEGIN_SCRIPT(...) (ltx_script(__VA_ARGS__))(_Lpn 
#define    END_SCRIPT(...) ,__VA_ARGS__);

#define JS_BEGIN BEGIN_SCRIPT
#define JS_BEGIN_STD JS_BEGIN(JS_LTX_STD)
#define JS_END_ARGS END_SCRIPT
#define JS_END )

#define txt_begin _Lpn
#define txt_end  
#define js_begin JS_BEGIN
#define js_end JS_END
#define js_end_args JS_END_ARGS 
