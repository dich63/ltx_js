#pragma once

#include "free_com_factory.h"
#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include "tls_dll.h"


template<class TSS_cleaner_class=tss_cleaner,int Port=777>
struct CLSID_DLL_Registrator
{

	typedef CLSID_DLL_Registrator<TSS_cleaner_class,Port> self_type;
	struct LIB_REG
	{
		CLSID m_clsid;
		HMODULE hm;
		DWORD dwCookie;
	};


	static void destructor(void* p)
	{
		self_type* pt=tls_var<self_type>();
		if(pt)
		{
			tls_var<self_type>()=0;
			delete pt;
		}
		
	};

	static self_type& instance() 
	{
		self_type* p=tls_var<self_type>();
		if(!p)
		{
			tls_var<self_type>()=p=new self_type;
			p->m_hinstance=HINSTANCE_IN_CODE;
			TSS_cleaner_class::on_exit_thread(&destructor,p);
		}
		return *p;
	}


	struct lib_reg:LIB_REG
	{
		lib_reg(){hm=0;dwCookie=0;};


		inline  void revoke()
		{ 
			 HRESULT hr;
			if((dwCookie))
				hr=CoRevokeClassObject(dwCookie),dwCookie=0;
		}

		inline 		bool clear()
		{
			//DllCanUnloadNow(void); 
			typedef HRESULT (__stdcall *PDllCanUnloadNow)(void);

			//if(dwCookie) CoRevokeClassObject(dwCookie),dwCookie=0;
			revoke();

			bool f=true;
			if(hm)
			{ 
				void *p=GetProcAddress(hm,"DllCanUnloadNow");
				if(p)
				{
					f=(PDllCanUnloadNow(p)()==S_OK);
					if(f)
					{

						CoFreeLibrary(hm);
						hm=0;
					}
				}
				return f;
			}
		}
		~lib_reg()
		{
			//printf("tid=%d\n",)
			char buf[256];
			sprintf(buf,"CLSID_list free tid=%d\n",GetCurrentThreadId());
			OutputDebugStringA(buf);
			clear();    
		}
	};

	struct clsid_lib:lib_reg 
	{

		operator CLSID&()
		{
			return  m_clsid;
		}

	};



	typedef  typename  std::list<clsid_lib> item_list;
	item_list  m_list;
	HMODULE m_hinstance;

	typedef  typename item_list::iterator item_iterator;

	//inline	item_iterator& find_CLSID(const CLSID &clsid)
	inline	bool find_CLSID(const CLSID &clsid)
	{
		struct _pred
		{
			CLSID m_clsid;
			_pred(const CLSID &clsid):m_clsid(clsid){};
			bool operator ()(const clsid_lib &l)
			{
				return m_clsid==l.m_clsid;  
			}

		};

		return std::find_if(m_list.begin(),m_list.end(),_pred(clsid))!=m_list.end();     
	}



	inline HRESULT Register(std::pair<HMODULE,CLSID> p)
	{
		return Register(p.first,p.second);
	}

	inline HRESULT Register(HMODULE hm,CLSID& clsid)
	{
		HRESULT hr;
		//if(find_CLSID(clsid)!=m_list.end()) 
     if(find_CLSID(clsid))
			return ERROR_ALREADY_EXISTS;
		LIB_REG lr={clsid,0,0};
		hr=free_com_factory::_creator(hm).RegisterCLSID(clsid,&lr.dwCookie);
		if(FAILED(hr)) return hr;

		if(hm!=m_hinstance)
		{
			free_com_factory::DLL_AddRef(hm);
			lr.hm=hm;
		}
		m_list.push_front( *((clsid_lib*)&lr));
	};


};




template <class TSS> 
HRESULT RegisterCLSID(HMODULE hm,const CLSID& clsid,bool fMTA=false)
{
	HRESULT hr;

	struct _registrator
	{

		HRESULT  operator ()(std::pair<HMODULE,CLSID> p)
		{
			HRESULT hr;
			return hr = CLSID_DLL_Registrator<TSS>::instance().Register(p);  
		}
	} registrator;

	if(fMTA) 
	{
		int tid=free_com_factory::MTA_tid(&CLSID_DLL_Registrator<TSS>::destructor);
		return hr=call_in_thread_context(registrator,std::make_pair(hm,clsid),tid,E_UNEXPECTED);
	}

	DWORD apt;
	if(FAILED(hr=free_com_factory::apartment_type(&apt)))
		return hr;
	if(apt==COINIT_APARTMENTTHREADED)
		return hr=registrator(std::make_pair(hm,clsid));
	else return hr=RegisterCLSID<TSS>(hm,clsid,true);
		//return hr=call_in_thread_context(registrator,std::make_pair(hm,clsid),free_com_factory::MTA_tid(),E_UNEXPECTED);

}


template <class TSS> 
HRESULT RegisterCLSID(wchar_t* plib,CLSID& clsid,bool fMTA=false)
{
	HRESULT hr;
	if(!plib) 
		return E_POINTER;
	  HMODULE hm=LoadLibraryW(plib);
	  if(!hm) return HRESULT_FROM_WIN32(GetLastError());
	  hr=RegisterCLSID<TSS>(plib,clsid,fMTA);
	  FreeLibrary(hm);
	  return hr;
}

template <class TSS,class LIB> 
HRESULT RegisterCLSID(LIB lib,wchar_t* strclsid,bool fMTA=false)
{
	HRESULT hr;
  CLSID clsid;
  if(FAILED(hr=CLSIDFromString(strclsid,&clsid))) return hr;
  return hr=RegisterCLSID<TSS>(lib,clsid,fMTA);
}

