#pragma once

#include <dispex.h>
#include "activdbg.h"
#include "com_utils.h"

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
   //
 OLE_CHECK_VOID(m_hr=m_pdm.CoCreateInstance(__uuidof(ProcessDebugManager)));
  OLE_CHECK_VOID(m_hr=m_pdm->CreateApplication(&m_DebugApp.p));
    m_DebugApp->SetName(L"Scripting Application");
  OLE_CHECK_VOID(m_hr=m_pdm->AddApplication(m_DebugApp.p, &m_dwAppCookie));
   OLE_CHECK_VOID(m_hr=m_pdm->CreateDebugDocumentHelper(NULL, &m_DebugDocHelper.p));
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


inline IProcessDebugManager* get_ProcessDebugManager()
{
	return m_pdm.p;
}

inline IDebugDocumentHelper* get_DebugDocumentHelper()
{
	return m_DebugDocHelper.p;
}
inline IDebugApplication* get_DebugApplication()
{
	return m_DebugApp.p;
}


    HRESULT  m_hr;
	DWORD m_dwAppCookie;
    com_ptr<IProcessDebugManager> m_pdm;
	com_ptr<IDebugApplication>    m_DebugApp;
	com_ptr<IDebugDocumentHelper>  m_DebugDocHelper;

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


//   begin IActiveScriptSite .....

	STDMETHOD(GetLCID)(LCID __RPC_FAR *plcid)	{return E_NOTIMPL;}

	STDMETHOD(GetItemInfo)(  LPCOLESTR pstrName,DWORD dwReturnMask,
		IUnknown **ppiunkItem,ITypeInfo **ppti)
	{ 
		return hr=static_cast<T*>(this)->OnItemInfo(pstrName,dwReturnMask,ppiunkItem,ppti);
	} 

	STDMETHOD(GetDocVersionString)( 
		BSTR*pbstrVersion)
	{ 
		return E_NOTIMPL;
	} 

	STDMETHOD(OnScriptTerminate)(	const VARIANT  *pvarResult,const EXCEPINFO  *pexcepinfo){return E_NOTIMPL;} 
	STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState)	{return S_OK;} 

	STDMETHOD(OnScriptError)(IActiveScriptError  *pscripterror) {return S_OK;}

	STDMETHOD(OnEnterScript)( void)	{ return S_OK;} 

	STDMETHOD(OnLeaveScript)( void)	{ return S_OK;} 

//   end IActiveScriptSite .....

//   begin IActiveScriptSiteWindow .....
	STDMETHOD(GetWindow)(HWND __RPC_FAR *phwnd)
	{
		if(phwnd)  	*phwnd = NULL;
		return S_OK;
	}
	STDMETHOD(EnableModeless)(BOOL fEnable){return S_OK;}

//   end IActiveScriptSiteWindow .....



//   begin IActiveScriptSiteDebug .....
	STDMETHOD(GetDocumentContextFromPosition)
		(DWORD dwSourceContext,ULONG uCharacterOffset,ULONG uNumChars,IDebugDocumentContext **ppsc)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetApplication)(IDebugApplication** ppda)
	{
		T* thisT=static_cast<T*>(this);
		HRESULT hr; 	
		if((ppda)&&(thisT->get_DebugApplication()))
			return hr=thisT->get_DebugApplication()->QueryInterface(__uuidof(IDebugApplication),(void**)ppda);
		else       return E_NOTIMPL;
	};


	STDMETHOD(GetRootApplicationNode)(IDebugApplicationNode **ppdanRoot)
	{
		HRESULT hr; 	
		if (!ppdanRoot) return E_INVALIDARG;

		T* thisT=static_cast<T*>(this);
		if(thisT->get_DebugDocumentHelper())
			return hr=thisT->get_DebugDocumentHelper()->GetDebugApplicationNode(ppdanRoot);
		else 	return E_NOTIMPL;
	}

	STDMETHOD(OnScriptErrorDebug)( IActiveScriptErrorDebug *pErrorDebug, 
		// whether to pass the error to the debugger to do JIT debugging 
		BOOL*pfEnterDebugger, 
		// whether to call IActiveScriptSite::OnScriptError() when the user 
		// decides to continue without debugging 
		BOOL *pfCallOnScriptErrorWhenContinuing)
	{

		if (pfEnterDebugger)
			*pfEnterDebugger = TRUE;
          
		if (pfCallOnScriptErrorWhenContinuing)
			*pfCallOnScriptErrorWhenContinuing = TRUE;
		return S_OK;
	};

//   end  IActiveScriptSiteDebug .....

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

	

HRESULT OnItemInfo(  LPCOLESTR pstrName,DWORD dwReturnMask,	IUnknown **ppiunkItem,ITypeInfo **ppti)
{
	 return E_NOTIMPL;
}


	inline IProcessDebugManager* get_ProcessDebugManager()
	{
		return 0;
	}

	inline IDebugDocumentHelper* get_DebugDocumentHelper()
	{
		return 0;
	}
	inline IDebugApplication* get_DebugApplication()
	{
		return 0;
	}



	
};




