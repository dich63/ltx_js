#pragma once

#include <comdef.h>
#include <atldef.h>
#include <mscoree.h> 

#include "singleton_utils.h" 
#include "tls_dll.h" 
#include "ipc_utils_base.h" 

#pragma comment(lib, "delayimp")
#pragma comment(lib, "mscoree.lib") 
#import <mscorlib.tlb> raw_interfaces_only high_property_prefixes("_get","_put","_putref") rename("ReportEvent", "StdReportEvent") rename("_DSA", "_CLR_DSA") 
//
//using namespace mscorlib; 
//IMAGE_COR20_HEADER

struct CorRuntimeHostHolder
{
  HRESULT m_hr;
  //CComAutoCriticalSection m_cs;
  mutex_cs_t mutex;
  ICorRuntimeHost*  m_pCRH;
  DWORD     m_startupFlags;
  CorRuntimeHostHolder(  DWORD startupFlags=
    STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN | STARTUP_CONCURRENT_GC|STARTUP_LOADER_SAFEMODE
	  )
	  :m_startupFlags(startupFlags),m_pCRH(NULL),m_hr(E_POINTER){};
  ~CorRuntimeHostHolder()
  {
	  //CS_AUTOLOCK(m_cs);
	  locker_t<mutex_cs_t> lock(mutex);
	  if(m_pCRH)    m_pCRH->Stop();
	  
  }
inline ICorRuntimeHost* CRH()
{
   //CS_AUTOLOCK(m_cs);
   locker_t<mutex_cs_t> lock(mutex);
   if(!m_pCRH)
   {
    //CComPtr<ICorRuntimeHost> p;
	ipc_utils::smart_ptr_t<ICorRuntimeHost> p;
	//wchar_t* pwszBuildFlavor=L"svr";
	//pwszBuildFlavor=L"wks";
	_bstr_t ve="v2.0.50727";
	m_hr = CorBindToRuntimeEx(NULL,bstr_t("svr"), 
		m_startupFlags, 
		CLSID_CorRuntimeHost, IID_ICorRuntimeHost,p._ppQI()); 
  if(SUCCEEDED(m_hr)&&SUCCEEDED(m_hr=p->Start()))
          m_pCRH=p.Detach();
      
   };
   return m_pCRH;
}
  
inline operator HRESULT()
{
	 CRH();
 return m_hr;
}
};

struct AppDomainHolder
{
    HRESULT m_hr;
	ipc_utils::smart_ptr_t<mscorlib::_AppDomain> m_domain;
	//AppDomainHolder():m_hr(E_FAIL){};
   CorRuntimeHostHolder& m_rh;
      bstr_t m_FriendName;

inline bool init_once()
{
	//CS_AUTOLOCK(m_rh.m_cs);
	locker_t<mutex_cs_t> lock(m_rh.mutex);
       if(!m_domain)
	   {
		   if(FAILED(m_hr=m_rh)) return false; 
		   ipc_utils::smart_ptr_t<IUnknown> unk;
		   if(FAILED(m_hr=m_rh.CRH()->CreateDomain(m_FriendName,NULL ,unk._address()))) return false;
		   m_hr=unk->QueryInterface(__uuidof(mscorlib::_AppDomain),  m_domain._ppQI()); 
		   if(FAILED(m_hr)) return false; 
	   }
	return true;
}

     AppDomainHolder(CorRuntimeHostHolder& rhh,bstr_t FriendName=bstr_t(""))
		 :m_rh(rhh),m_hr(E_POINTER)
	{
       m_FriendName=(wchar_t*)FriendName;
	   /*
       if(FAILED(m_hr=rhh)) return; 
	   CComPtr<IUnknown> unk;
       if(FAILED(m_hr=rhh.CRH()->CreateDomain(FriendName,NULL ,&unk.p))) return;
	   m_hr=unk->QueryInterface(__uuidof(mscorlib::_AppDomain), (void**) &m_domain); 
	   */

	}
inline HRESULT   CreateComponent(bstr_t AssemblyName,bstr_t typeName,VARIANT* pv)
{
	if(!pv) return E_POINTER;
	HRESULT hr;
	if(!init_once()) return m_hr;
    ipc_utils::smart_ptr_t<mscorlib::_ObjectHandle> oh;
if(FAILED(hr=m_domain->CreateInstance(AssemblyName,typeName,oh._address()))) return hr;
return hr=oh->Unwrap(pv);

}

inline HRESULT   CreateComponentFrom(bstr_t AssemblyFile,bstr_t typeName,VARIANT* pv)
{
	if(!pv) return E_POINTER;
	HRESULT hr;
	if(!init_once()) return m_hr;
	ipc_utils::smart_ptr_t<mscorlib::_ObjectHandle> oh;
	BSTR ba=AssemblyFile;
	BSTR bt=typeName;
	if(FAILED(hr=m_domain->CreateInstanceFrom(ba,bt,oh._address()))) return hr;
	return hr=oh->Unwrap(pv);

}
/*
template <class I>
inline HRESULT   CreateComponentFrom(bstr_t AssemblyFile,bstr_t typeName,I** ppOut)
{
	if(!ppOut) return E_POINTER;
	VARIANT res=VARIANT();
	HRESULT hr;

    if(SUCCEEDED(hr=CreateComponentFrom(AssemblyFile,typeName,&res)))
	{
          *ppOut=res.*CVarTypeInfo<I*>::pmField;
	}
    return hr;
}
*/
};

struct AppDomainHolder2:AppDomainHolder
{
 static CorRuntimeHostHolder& RHH()
 {
          //static CorRuntimeHostHolder rhh;
	      //return rhh;
	     CorRuntimeHostHolder& rhh=class_initializer_T<CorRuntimeHostHolder>().get();
		  return rhh;	
		  
 }

 
 bool m_fdetached;
AppDomainHolder2(bstr_t FriendName=bstr_t(""),bool fglobal=false):AppDomainHolder(RHH(),FriendName)
 {m_fdetached=fglobal;}
~AppDomainHolder2(){if(m_fdetached) m_domain.Detach();};
};

