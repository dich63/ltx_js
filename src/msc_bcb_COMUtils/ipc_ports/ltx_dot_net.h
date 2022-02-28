#pragma once

#include <comdef.h>
#include <atldef.h>
#include <mscoree.h> 
#pragma comment(lib, "delayimp")
#pragma comment(lib, "mscoree.lib") 
#import <mscorlib.tlb> raw_interfaces_only high_property_prefixes("_get","_put","_putref") rename("ReportEvent", "StdReportEvent") rename("_DSA", "_CLR_DSA") 
//
//using namespace mscorlib; 
//IMAGE_COR20_HEADER

struct CorRuntimeHostHolder
{
  HRESULT m_hr;
  CComAutoCriticalSection m_cs;
  ICorRuntimeHost*  m_pCRH;
  DWORD     m_startupFlags;
  CorRuntimeHostHolder(  DWORD startupFlags=
    STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN | STARTUP_CONCURRENT_GC|STARTUP_LOADER_SAFEMODE
	  )
	  :m_startupFlags(startupFlags),m_pCRH(NULL),m_hr(E_POINTER){};
  ~CorRuntimeHostHolder()
  {
	  CS_AUTOLOCK(m_cs);
	  if(m_pCRH)    m_pCRH->Stop();
	  
  }
inline ICorRuntimeHost* CRH()
{
  CS_AUTOLOCK(m_cs);
   if(!m_pCRH)
   {
    CComPtr<ICorRuntimeHost> p;
	//wchar_t* pwszBuildFlavor=L"svr";
	//pwszBuildFlavor=L"wks";
	_bstr_t ve="v2.0.50727";
	m_hr = CorBindToRuntimeEx(NULL,bstr_t("svr"), 
		m_startupFlags, 
		CLSID_CorRuntimeHost, IID_ICorRuntimeHost,(void **)&p); 
  if(SUCCEEDED(m_hr)&&SUCCEEDED(m_hr=p->Start()))
          m_pCRH=p.Detach();
  /*
	if(FAILED(m_hr)) return NULL;
     m_hr=p->Start(); 
    if(FAILED(m_hr)) return NULL;
	*/
      
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
	CComPtr<mscorlib::_AppDomain> m_domain;
	//AppDomainHolder():m_hr(E_FAIL){};
   CorRuntimeHostHolder& m_rh;
      bstr_t m_FriendName;

inline bool init_once()
{
	CS_AUTOLOCK(m_rh.m_cs);
       if(!m_domain)
	   {
		   if(FAILED(m_hr=m_rh)) return false; 
		   CComPtr<IUnknown> unk;
		   if(FAILED(m_hr=m_rh.CRH()->CreateDomain(m_FriendName,NULL ,&unk.p))) return false;
		   m_hr=unk->QueryInterface(__uuidof(mscorlib::_AppDomain), (void**) &m_domain); 
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
    CComPtr<mscorlib::_ObjectHandle> oh;
if(FAILED(hr=m_domain->CreateInstance(AssemblyName,typeName,&oh))) return hr;
return hr=oh->Unwrap(pv);

}

inline HRESULT   CreateComponentFrom(bstr_t AssemblyFile,bstr_t typeName,VARIANT* pv)
{
	if(!pv) return E_POINTER;
	HRESULT hr;
	if(!init_once()) return m_hr;
	CComPtr<mscorlib::_ObjectHandle> oh;

	if(FAILED(hr=m_domain->CreateInstanceFrom(AssemblyFile,typeName,&oh))) return hr;
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
          static CorRuntimeHostHolder rhh;
		  return rhh;
 }
 bool m_fdetached;
AppDomainHolder2(bstr_t FriendName=bstr_t(""),bool fglobal=false):AppDomainHolder(RHH(),FriendName)
 {m_fdetached=fglobal;}
~AppDomainHolder2(){if(m_fdetached) m_domain.Detach();};
};

