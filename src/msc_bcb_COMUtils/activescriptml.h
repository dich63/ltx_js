#pragma once

#include <dispex.h>
#include "activdbg.h"
#include "web_silk.h"

#define OLE_CHECK_VOID(a) if(FAILED((a))) return;
//#define CHECK_VOID_hr(a) if(FAILED((m_hr=a))) return;




template<typename V, V v=V()>
struct valueT
{
	V m_v;
	inline 	valueT():m_v(v){};
	inline 	operator V&(){return m_v;}
	inline 	valueT& operator =(V v){m_v=v;return *this;}
};




struct ExternalDebugger
{

 ExternalDebugger()
 {
	 m_dwAppCookie=0;
   OLE_CHECK_VOID(m_hr=m_pdm.CoCreateInstance(__uuidof(ProcessDebugManager)));
   OLE_CHECK_VOID(m_hr=m_pdm->CreateApplication(&m_DebugApp));
    m_DebugApp->SetName(L"Scripting Application");
   OLE_CHECK_VOID(m_hr=m_pdm->AddApplication(m_DebugApp, &m_dwAppCookie));

   OLE_CHECK_VOID(m_hr=m_pdm->CreateDebugDocumentHelper(NULL, &m_DebugDocHelper));
    m_DebugDocHelper->Init(m_DebugApp,NULL,NULL,NULL);
    m_DebugDocHelper->Attach(NULL);
 }

~ExternalDebugger()
{
    if(!m_pdm) return;
	m_DebugDocHelper.Release();
	m_DebugApp.Release();
	m_pdm->RemoveApplication(m_dwAppCookie);

}

    HRESULT  m_hr;
	DWORD m_dwAppCookie;
    CComPtr<IProcessDebugManager> m_pdm;
	CComPtr<IDebugApplication>    m_DebugApp;
	CComPtr<IDebugDocumentHelper>  m_DebugDocHelper;
};







struct script_site_tuner
{
	inline HRESULT OnItemInfo(  LPCOLESTR pstrName,DWORD dwReturnMask,
		IUnknown **ppiunkItem,ITypeInfo **ppti)
	{ 
		return E_NOTIMPL;
	} 


	CComPtr<IActiveScriptError>    m_error;
	CComPtr<IDebugApplication>    m_DebugApp;
	CComPtr<IDebugDocumentHelper>  m_DebugDocHelper;
};


template<class Intf,class T>
inline bool   _CheckQI(T* _this,REFIID iid,void ** ppObj)
{
	bool f=IsEqualIID(iid,__uuidof(Intf));
	if(f)  *ppObj= (void*)static_cast<Intf*>(_this);
	return f;
}


template <class T>
struct __declspec(novtable) IActiveScriptSiteImpl_ML : public T,
	public IActiveScriptSite, 
	public IActiveScriptSiteWindow,
	public IActiveScriptSiteDebug
{
public:
 bool fdebug;

#define COM_INTERFACE_ENTRY_IActiveScriptSiteImpl_ML(T)\
	{NULL,offsetofclass(IActiveScriptSiteImpl_ML<T>,_ComMapClass),\
	&IActiveScriptSiteImpl_ML<T>::ATL_Helper<_ComMapClass>::ATL_QI_EntryFunc),

template <class CCOM>
struct ATL_Helper
{
	static HRESULT __stdcall ATL_QI_EntryFunc(void* pv,REFIID iid,void** ppvObject, DWORD dw)
	{
		HRESULT hr;
		IActiveScriptSiteImpl_ML<T>* pthis=static_cast<IActiveScriptSiteImpl_ML<T>*>(DWORD_PTR(pv)+dw);
		CCOM* pcomclass=static_cast<CCOM*>(pv);  
		hr=pthis->ML_QueryInterface(iid,ppvObject);
		if(SUCCEEDED(hr))
		           pcomclass->GetUnknown()->AddRef();
		return hr;
	}
};
	inline HRESULT ML_QueryInterface(REFIID iid, void** ppvObject)
	{
		if(!ppvObject) return E_POINTER;
         
        if(!_CheckQI<IActiveScriptSite>(this,iid,ppvObject))
          if(!_CheckQI<IActiveScriptSiteWindow>(this,iid,ppvObject)) 
			  if(!_CheckQI<IActiveScriptSiteDebug>(this,iid,ppvObject))
                    	return E_NOINTERFACE;
           
		   //this->AddRef();   
		   return S_OK;

	}

inline    IActiveScriptSite* ScriptSite()
{
	return static_cast<IActiveScriptSite*>(this);
}

	STDMETHOD(GetLCID)(LCID __RPC_FAR *plcid)
	{return E_NOTIMPL;}

	STDMETHOD(GetItemInfo)(  LPCOLESTR pstrName,DWORD dwReturnMask,
		IUnknown **ppiunkItem,ITypeInfo **ppti)
	{ 
		__if_exists(T::OnItemInfo)
		{
			return OnItemInfo(pstrName,dwReturnMask,ppiunkItem,ppti);
		}

		return E_NOTIMPL;
	} 

	STDMETHOD(GetDocVersionString)( 
		BSTR*pbstrVersion)
	{ 
		return E_NOTIMPL;
	} 

	STDMETHOD(OnScriptTerminate)( 
		const VARIANT  *pvarResult,
		const EXCEPINFO  *pexcepinfo)
	{ 
		return E_NOTIMPL;
	} // end OnScriptTerminate

	STDMETHOD(OnStateChange)( 
		SCRIPTSTATE ssScriptState)
	{ 
		return S_OK;
	} 

	STDMETHOD(OnScriptError)( 
		IActiveScriptError  *pscripterror)
	{ 
       __if_exists(T::m_error)  
	   {  
  		 m_error=pscripterror;
	   }  

		return S_OK;
	} // end OnScriptError

	STDMETHOD(OnEnterScript)( void)
	{ 
		return S_OK;
	} // end OnEnterScript

	STDMETHOD(OnLeaveScript)( void)
	{ 
		return S_OK;
	} 
	////*
	STDMETHOD(GetWindow)( 
		HWND __RPC_FAR *phwnd)
	{
		if(phwnd)  	*phwnd = NULL;

		return S_OK;
	} // end GetWindow

	STDMETHOD(EnableModeless)( 
		BOOL fEnable)
	{
		return S_OK;
	} // end EnableModeless

	STDMETHOD(GetDocumentContextFromPosition)
		(
		DWORD_PTR dwSourceContext,// As provided to ParseScriptText 
		// or AddScriptlet 
		ULONG uCharacterOffset,// character offset relative 
		// to start of script block or scriptlet 
		ULONG uNumChars,// Number of characters in context 
		// Returns the document context corresponding to this character-position range. 
		IDebugDocumentContext **ppsc)
	{


		ULONG ulStartPos = 0;
		HRESULT hr= E_NOTIMPL;

		/*
		if (m_DebugDocHelper)
		{
		hr = m_DebugDocHelper->GetScriptBlockInfo(dwSourceContext, NULL, &ulStartPos, NULL);
		hr = m_DebugDocHelper->CreateDebugDocumentContext(ulStartPos + uCharacterOffset, uNumChars, ppsc);
		}
		else
		{
		hr = E_NOTIMPL;
		}
		*/
		return hr;
	}

	STDMETHOD(GetApplication)(IDebugApplication** ppda)
	{
		HRESULT hr; 
		return E_NOTIMPL;
		__if_exists(T::m_DebugApp)
		{
			if((ppda)&&(m_DebugApp.p))
			{
				//*ppda=m_pDebugApp;

				
					hr=m_DebugApp.CopyTo(ppda);
					
				//(*ppda)->StartDebugSession();
				//if(!fdebug) (*ppda)->StepOutComplete();
					//if(!fdebug) (*ppda)->Close();
					return hr;
			}

		}   
		return E_NOTIMPL;
	};
							

	STDMETHOD(GetRootApplicationNode)(IDebugApplicationNode **ppdanRoot)
	{
		HRESULT hr; 	
		if (!ppdanRoot) return E_INVALIDARG;


		__if_exists(T::m_DebugDocHelper)
		{

			if (m_DebugDocHelper)
			{
				//if(fdebug) 
                    
					return hr=m_DebugDocHelper->GetDebugApplicationNode(ppdanRoot);
				//*ppdanRoot=0;
				//return S_OK;
			}

		}

		return E_NOTIMPL;
	}
	STDMETHOD(OnScriptErrorDebug)( IActiveScriptErrorDebug *pErrorDebug, 
		// whether to pass the error to the debugger to do JIT debugging 
		BOOL*pfEnterDebugger, 
		// whether to call IActiveScriptSite::OnScriptError() when the user 
		// decides to continue without debugging 
		BOOL *pfCallOnScriptErrorWhenContinuing)
	{

		if (pfEnterDebugger)
		{
			//
			*pfEnterDebugger = fdebug;
			//   *pfEnterDebugger = FALSE;
		}
		if (pfCallOnScriptErrorWhenContinuing)
		{
			*pfCallOnScriptErrorWhenContinuing = TRUE;
		}
		return S_OK;




	};

};




