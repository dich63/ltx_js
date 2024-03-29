/// dummy_dispatch_aggregator.h
#pragma once
// IDispatch_for_moronic_client_ala_MATLAB
#include "ipc_ports/ipc_utils.h"
#include <windows.h>
#include "tls_dll.h" 
#include <DispEx.h>
#include "dispid_find.h"
#include <map>


#define DISPATCH_GETSF (DISPATCH_PROPERTYGET|DISPATCH_METHOD)
#define DISPATCH_PUTSF (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)


#define is_get(f) (f&DISPATCH_GETSF)
#define is_put(f) (f&DISPATCH_PUTSF)



/*
struct exceptinfo_t:EXCEPINFO
{
	exceptinfo_t(const EXCEPINFO& ei=EXCEPINFO() ):EXCEPINFO(ei){}
	inline void clear(int f=1)
	{
		SysFreeString(bstrSource);
		SysFreeString(bstrDescription);
		SysFreeString(bstrHelpFile);
		if(f) *this=exceptinfo_t();
	}
	~exceptinfo_t()
	{
		clear(0);
	}


};

*/

//IObjectWithSite


//inline HRESULT set_aggregator(GUID iid,IUnknown* punk=0)

template <class T>
struct aggregator_helper_base_t :iaggregator_helper
{
	aggregator_helper_base_t() :refcount(1) {};
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv)
	{
		if (!ppv) return E_POINTER;
		if ((iid == __uuidof(IUnknown)) || (iid == __uuidof(iaggregator_helper)))
		{
			AddRef();
			*ppv = (void*)this;
			return S_OK;
		}
		return E_NOTIMPL;
	}
	virtual ULONG __stdcall AddRef() {
		return InterlockedIncrement(&refcount);
	}
	virtual ULONG __stdcall Release() {
		LONG l = InterlockedDecrement(&refcount);
		if (l == 0) delete static_cast<T*>(this);
		return l;
	}

	inline bool   inner_QI(IUnknown* Outer, REFIID riid, void** ppObj, HRESULT& hr) {
		return false;
	}
	inline HRESULT set_context(void* pContext) { return E_NOTIMPL; }

	virtual BOOL STDMETHODCALLTYPE innerQueryInterface(IUnknown* Outer, REFIID riid, void** ppObj, HRESULT* phr)
	{
		return static_cast<T*>(this)->inner_QI(Outer, riid, ppObj, *phr);
	};
	virtual HRESULT STDMETHODCALLTYPE setContext(void* pContext)
	{
		return static_cast<T*>(this)->set_context(pContext);
	};

	volatile LONG refcount;
};

template <int V = 0>
struct aggregator_helper_helper
{
	aggregator_helper_helper() :ah(0) {};

	inline HRESULT reset(IUnknown* unk = 0)
	{
		HRESULT hr = E_POINTER;

		if ((!unk) && (!ah)) return hr;

		iaggregator_helper *t = ipc_utils::make_detach(ah);
		//t=ah;
		//ah=0;
		if (unk) {
			hr = unk->QueryInterface(__uuidof(iaggregator_helper), (void**)&ah);
		}
		if (t) t->Release();
		return hr;
	}

	inline HRESULT set_context(void* pctx)
	{
		if (ah) return ah->setContext(pctx);
		else return E_POINTER;
	}

	~aggregator_helper_helper() {
		reset();
	}



	inline aggregator_helper_helper& flipflop(aggregator_helper_helper& a)
	{
		if ((&a) != this)
		{
			reset();
			ah = ipc_utils::make_detach(a.ah);
		}
		return *this;
	}


	inline operator bool() { return ah; };

	bool operator()(IUnknown* punkOuter, REFIID riid, void** ppunk, HRESULT& hr)
	{
		return (ah) && (ah->innerQueryInterface(punkOuter, riid, ppunk, &hr));
	}
	iaggregator_helper *ah;
};


template <bool freemarshall = true, class Intf = IUnknown, const IID& MarshalUIID = IID_IMarshal>
//template <bool freemarshall = true, class Intf = IUnknown, class CoI= IMarshal>
struct  FreeThreadedMarshaler_t
{
	IUnknown * pmarshal;
	FreeThreadedMarshaler_t() :pmarshal(0) {}
	~FreeThreadedMarshaler_t()
	{
		if (pmarshal) pmarshal->Release();
	}
	inline bool operator()(Intf * punkOuter, REFIID riid, void** ppunk, HRESULT& hr)
	{
		IUnknown * p;
		if (riid != MarshalUIID) return false;
		if (!(p = init_once(punkOuter))) return false;
		//hr=p->QueryInterface(riid,ppunk);
		hr = p->QueryInterface(__uuidof(IMarshal), ppunk);

		return SUCCEEDED(hr);

	}
	inline IUnknown * init_once(Intf * pintf)
	{
		HRESULT hr;
		IUnknown *p = (IUnknown *)InterlockedCompareExchangePointer((volatile PVOID*)&pmarshal, 0, 0);
		if (p) return p;
		IUnknown * punk = 0;
		hr = pintf->QueryInterface(__uuidof(IUnknown), (void**)&punk);
		if (SUCCEEDED(hr))
		{
			hr = ::CoCreateFreeThreadedMarshaler(punk, &p);
			punk->Release();
		}
		else p = 0;
		//if(FAILED(hr)) p=0;
		IUnknown *pt = (IUnknown *)InterlockedCompareExchangePointer((volatile PVOID*)&pmarshal, p, 0);
		if (pt)
		{
			if (p) p->Release();
			return pt;
		}
		return p;
	};

};

template <class Intf>
struct  FreeThreadedMarshaler_t<false, Intf>
{
	inline bool operator()(Intf*, REFIID, void**, HRESULT&) { return false; }
};




template <const IID& MarshalUIID>
static HRESULT create_FTM_aggregator_helperCLSID(IUnknown** punk)
{

	struct F :aggregator_helper_base_t<F>
	{
		inline bool   inner_QI(IUnknown* Outer, REFIID riid, void** ppObj, HRESULT& hr) {

			return ftm(Outer, riid, ppObj, hr);
		}
		FreeThreadedMarshaler_t<true, IUnknown, MarshalUIID> ftm;
	};
	if (!punk) return E_POINTER;
	*punk = new F;
	return S_OK;
};

inline static HRESULT create_FTM_aggregator_helper(IUnknown** punk)
{
	return create_FTM_aggregator_helperCLSID<__uuidof(IMarshal)>(punk);
}


inline static HRESULT create_FTM_aggregator_helperSM(IUnknown** punk)
{
	return create_FTM_aggregator_helperCLSID<__uuidof(IFreeThreadMarshal)>(punk);
}


template <class T, class IntfA>
struct base_component_t :IntfA
{

	//base_component_t()	{          T *pt=static_cast<T*>(this);	}

	base_component_t(IUnknown *punk = 0) :pOuter(punk) {};

	inline HRESULT impl_QueryInterface(const IID& riid, void** ppv)
	{
		if ((riid == __uuidof(IUnknown)) || (riid == __uuidof(IntfA)))
		{
			*ppv = (void*)static_cast<T*>(this);
			this->AddRef();
			return S_OK;
		}
		return E_NOTIMPL;

	}

	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv) {
		return static_cast<T*>(this)->impl_QueryInterface(iid, ppv);

	}

	virtual ULONG __stdcall AddRef() { return pOuter->AddRef(); }
	virtual ULONG __stdcall Release() {
		ULONG l = pOuter->Release();
		return l;
	}

	inline IUnknown* GetUnknown() { return pOuter; }

	union {
		LONGLONG __laign64;
		IUnknown* pOuter;
	};
};


template <class T, class IntfA>
struct base_aggregator_t : base_component_t<T, IntfA>//:IntfA
{
	base_aggregator_t(IUnknown *punk = 0) : base_component_t<T, IntfA>(punk) {};

	inline HRESULT impl_QueryInterface(const IID& iid, void** ppv) {

		return pOuter->QueryInterface(iid, ppv);
	}
	/*
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv){

		return pOuter->QueryInterface(iid, ppv);
	}

	virtual ULONG __stdcall AddRef(){ return pOuter->AddRef();}
	virtual ULONG __stdcall Release(){
		 ULONG l;
		return l=pOuter->Release();
	}
	*/

	inline bool   inner_QI(IUnknown* Outer, REFIID riid, void** ppObj, HRESULT& hr)
	{
		return  inner_QI(Outer, riid, __uuidof(IntfA), ppObj, hr);
	}

	inline bool   inner_QI2(IUnknown* Outer, REFIID riid, REFIID riid2, void** ppObj, HRESULT& hr)
	{
		return inner_QI(Outer, riid, riid2, ppObj, hr);
	}

	inline bool   inner_QI(IUnknown* Outer, REFIID riid, REFIID riid2, void** ppObj, HRESULT& hr)
	{

		if (riid == riid2)
		{
			InterlockedExchangePointer((void**)&pOuter, Outer);
			if (ppObj)
			{
				this->AddRef();
				*ppObj = static_cast<IntfA*>(this);
				hr = S_OK;
				return true;
			}
			else hr = E_POINTER;

		}

		return false;
	}

	inline bool operator()(IUnknown* punkOuter, REFIID riid, void** ppObj, HRESULT& hr)
	{
		return static_cast<T*>(this)->inner_QI(punkOuter, riid, ppObj, hr);
	}
	/*
		union{
			LONGLONG __laign64;
			T* pOuter;
		};
	*/
};


/*
template <class T>
struct ExternalConnectionSite_t : base_aggregator_t<T,IObjectWithSite>
{
	typedef typename T::external_connection_t external_connection_t;
	virtual HRESULT STDMETHODCALLTYPE SetSite( IUnknown *pUnkSite)
	{
		return  E_NOTIMPL;
	};

	virtual HRESULT STDMETHODCALLTYPE GetSite(REFIID riid,void **ppvSite)
	{
		return  E_NOTIMPL;
	}


};
*/


template <class T>
struct ExternalConnectionAggregator_t : base_aggregator_t<T, IExternalConnection>
{


	struct ExternalConnectionSite_t : base_aggregator_t<ExternalConnectionSite_t, IObjectWithSite>
	{

		virtual HRESULT STDMETHODCALLTYPE SetSite(IUnknown *pUnkSite)
		{
			return  owner->reset_event(pUnkSite);
		};

		virtual HRESULT STDMETHODCALLTYPE GetSite(REFIID riid, void **ppvSite)
		{
			HRESULT hr;
			ipc_utils::smart_ptr_t<IUnknown> unk;
			if (SUCCEEDED(hr = owner->StubEvent.unwrap(unk)))
				hr = unk->QueryInterface(riid, ppvSite);

			return hr;

		}

		ExternalConnectionAggregator_t* owner;


	} site;




	ExternalConnectionAggregator_t(IDispatch* _StubEvent = 0) :nStrongLocks(0), StubEvent(_StubEvent)
	{
		site.owner = this;
	};


	inline HRESULT  disp_invoke_bool(VARIANT_BOOL fconnect)
	{
		HRESULT hr;
		ipc_utils::smart_ptr_t<IDispatch> disp;
		if (FAILED(hr = StubEvent.unwrap(disp)))
			return hr;


		//if(!pStubEvent) return E_POINTER;

		VARIANT v = { VT_BOOL }, r = { VT_EMPTY };
		v.boolVal = fconnect;
		DISPPARAMS dp = { &v,0,1,0 };
		hr = disp->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dp, &r, NULL, NULL);
		VariantClear(&r);
		return hr;
	}


	STDMETHODIMP_(DWORD) AddConnection(DWORD extconn, DWORD dwReserved)
	{
		DWORD dw = 0;
		HRESULT hr;

		if ((extconn & EXTCONN_STRONG))
		{

			dw = InterlockedIncrement(&nStrongLocks);
			ipc_utils::DBG_PRINT(L"EC: ++connection [%d] at=%p tid=%d", dw, (void*)this, ::GetCurrentThreadId());
			if (dw == 1)
				(StubEvent) && (hr = disp_invoke_bool(VARIANT_TRUE));
		}
		return dw;
	}
	STDMETHODIMP_(DWORD) ReleaseConnection(DWORD extconn, DWORD dwReserved, BOOL bLastUnlockReleases)
	{
		DWORD dw = 0;
		HRESULT hr;
		if ((extconn & EXTCONN_STRONG))
		{
			
			dw = InterlockedDecrement(&nStrongLocks);
			ipc_utils::DBG_PRINT(L"EC. --connection [%d] at=%p tid=%d", dw, (void*)this, ::GetCurrentThreadId());
			//static_cast<T*>(this)->OnReleaseConnection(dw == 0, !!bLastUnlockReleases);
			//(StubEvent) && (hr = disp_invoke_bool(VARIANT_FALSE));

			//if ((bLastUnlockReleases))
			if ((dw == 0) && (bLastUnlockReleases))				
			{
				//InterlockedExchange(&nStrongLocks, 0);				
				ipc_utils::DBG_PRINT(L"EC. bLastUnlockReleases [%d] at=%p tid=%d",dw, (void*)this, ::GetCurrentThreadId());
				(StubEvent) && (hr = disp_invoke_bool(VARIANT_FALSE));
				if (pOuter)
				{
					IUnknown* punk = 0;
					if (SUCCEEDED(hr = pOuter->QueryInterface(__uuidof(IUnknown), (void**)&punk)))
					{
						hr = CoDisconnectObject(punk, 0);
						punk->Release();

					}

				}



			}
		}
		return dw;
	}

	// inner_QI(T* Outer,REFIID riid,void** ppObj,HRESULT& hr ) 

	inline bool operator()(IUnknown* punkOuter, REFIID riid, void** ppObj, HRESULT& hr)
	{
		//if(StubEvent) return inner_QI(static_cast<T*>(punkOuter),riid, ppObj,hr) ;

		bool f;

		if (riid == __uuidof(IObjectWithSite))
		{
			FatalAppExitA(0, "IObjectWithSite ���...!!");
		}

		//
	   //f= (StubEvent)&&inner_QI(punkOuter,riid, ppObj,hr) ;
	   //
		f = inner_QI(punkOuter, riid, ppObj, hr);
		f || (f = (site.inner_QI(punkOuter, riid, ppObj, hr)));

		return f;
		//else return false;
	}

	inline HRESULT reset_event(IUnknown* punk)
	{

		HRESULT hr;
		LONG l;
		if (!punk) return E_POINTER;
		ipc_utils::smart_ptr_t<IDispatch> disp;
		if (FAILED(hr = punk->QueryInterface(__uuidof(IDispatch), disp.ppQI())))
			return hr;



		if (l = InterlockedCompareExchange((LONG volatile*)&StubEvent.dwcookie, 0, 0))
			return E_ACCESSDENIED;
		ipc_utils::smart_GIT_ptr_t<IDispatch> s(disp);
		if (l = InterlockedCompareExchange((LONG volatile*)&StubEvent.dwcookie, s.dwcookie, 0))
			return E_ACCESSDENIED;
		s.detach();
		return S_OK;
	}

	~ExternalConnectionAggregator_t() {
		//if(pStubEvent) pStubEvent->Release();
	}
	volatile LONG nStrongLocks;

	ipc_utils::smart_GIT_ptr_t<IDispatch> StubEvent;
	//IDispatch* pStubEvent;
};


inline static HRESULT create_ExternalConnection_aggregator_helper(IDispatch* conn_event, IUnknown** punk)
{

	struct FAA :aggregator_helper_base_t<FAA>
	{
		inline bool   inner_QI(IUnknown* Outer, REFIID riid, void** ppObj, HRESULT& hr) {

			return eca(Outer, riid, ppObj, hr);
		}

		struct ECA_t :ExternalConnectionAggregator_t<ECA_t> {
			ECA_t(IDispatch* p) :ExternalConnectionAggregator_t<ECA_t>(p) {};

		};

		FAA(IDispatch* conn_event) :eca(conn_event) {};


		ECA_t eca;

		//FreeThreadedMarshaler_t<true,IUnknown> ftm;
		//ExternalConnectionAggregator_t<IUnknown> eca;

	};


	if (!punk) return E_POINTER;

	FAA* pfaa = new FAA(conn_event);

	*punk = pfaa;

	return S_OK;
};

inline static HRESULT create_FMTEC_aggregator_helper(IDispatch* conn_event, IUnknown** punk, int flags = 3)
{

	struct FU :aggregator_helper_base_t<FU>
	{
		inline bool   inner_QI(IUnknown* Outer, REFIID riid, void** ppObj, HRESULT& hr) {

			return ftm(Outer, riid, ppObj, hr) || eca(Outer, riid, ppObj, hr);
		}



		struct ECA_t :ExternalConnectionAggregator_t<ECA_t> {
			ECA_t(IDispatch* p) :ExternalConnectionAggregator_t<ECA_t>(p) {};

		};

		FU(IDispatch* conn_event) :eca(conn_event) {};


		FreeThreadedMarshaler_t<true, IUnknown> ftm;
		ECA_t eca;

	};

	HRESULT hr;
	if (!punk) return E_POINTER;
	*punk = 0;
	switch (flags)
	{
	case 1:
		return   hr = create_FTM_aggregator_helper(punk);
		break;
	case 2:
		return   hr = create_ExternalConnection_aggregator_helper(conn_event, punk);
		break;
	case 3:
		*punk = new FU(conn_event);
		break;
	}



	return S_OK;
};


template <int RC0 = 0>
struct RefCounterGlobal_t
{
	RefCounterGlobal_t() :refcount(RC0) {};


	inline ULONG AddRef() {
		return InterlockedIncrement(&refcount);
	}

	inline ULONG AddRefEx(ULONG a) {
		return InterlockedExchangeAdd(&refcount, a);
	}

	inline ULONG Release() {

		return InterlockedDecrement(&refcount);
	}
	inline ULONG RefCount() {
		return AddRefEx(0) - RC0;
	}
	inline static RefCounterGlobal_t<> & get()
	{
		return class_initializer_T<RefCounterGlobal_t<> >().get();
	}

	volatile LONG refcount;
};



template <int RC0 = 1>
struct RefCounterMT_t
{
	RefCounterMT_t() :refcount(RC0) {
		if (RC0)
			global().AddRefEx(RC0);
	};

	template <class T>
	inline ULONG AddRef(T*) {
		//
		global().AddRef();
		return InterlockedIncrement(&refcount);
	}


	inline ULONG AddRef() {
		//
		global().AddRef();
		return InterlockedIncrement(&refcount);
	}

	inline ULONG Release() {

		//
		global().Release();

		return InterlockedDecrement(&refcount);
	}

	template <class T>
	inline ULONG Release(T* owner) {
		long l, m;
		if ((l = Release()) == 0)
			delete owner;
		return l;
	}


	volatile LONG refcount;

	inline static RefCounterGlobal_t<> & global()
	{
		return RefCounterGlobal_t<>::get();
	}

};

template <int RC0 = 1, bool fINIT = true>
struct RefCounterMT_blocked_t
{
	RefCounterMT_blocked_t() :refcount(RC0), f(fINIT) {};

	template <class T>
	inline ULONG AddRef(T*) {
		return AddRef();
	}


	inline ULONG AddRef() {
		RefCounterGlobal_t<>::get().AddRef();
		return InterlockedIncrement(&refcount);
	}

	inline ULONG Release() {
		RefCounterGlobal_t<>::get().Release();
		return InterlockedDecrement(&refcount);
	}

	template <class T>
	inline ULONG Release(T* owner) {
		long l;
		if (((l = Release()) == 0) && f)
			delete owner;
		return l;
	}

	bool f;
	volatile LONG refcount;
};



template <int RC0 = 1>
struct RefCounterST_t
{
	RefCounterST_t() :refcount(RC0) {};
	inline ULONG AddRef() { return ++refcount; }

	inline ULONG Release() { return --refcount; }

	template <class T>
	inline ULONG AddRef(T*) { return ++refcount; }
	template <class T>
	inline ULONG Release(const T* owner) {
		long l;
		if ((l = Release()) == 0)
			delete owner;
		return l;
	}


	volatile LONG refcount;
};

template <int RC0 = 1>
struct RefCounterFake_t
{

	inline ULONG AddRef() { return RC0; }
	inline ULONG Release() { return RC0; }
	template <class T>
	inline ULONG AddRef(T*) { return RC0; }
	template <class T>
	inline ULONG Release(T*) { return RC0; }

};


template <class T, class I, class RefCounter = RefCounterMT_t<1> >
struct IUnknown_base_impl :I
{

	typedef RefCounter refcounter_t;


	inline HRESULT impl_QueryInterface(const IID& riid, void** ppvObject)
	{
		if (!ppvObject) return E_POINTER;
		if ((riid == IID_IUnknown) || (riid == __uuidof(I)))
		{
			*ppvObject = (T*)static_cast<T*>(this);
			this->AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;;
	}
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) {

		return static_cast<T*>(this)->impl_QueryInterface(riid, ppvObject);
	}
	inline ULONG impl_AddRef() {
		return refcounter.AddRef(static_cast<T*>(this));
	};
	inline ULONG impl_Release() {
		return refcounter.Release(static_cast<T*>(this));
	};
	STDMETHOD_(ULONG, AddRef)() { return static_cast<T*>(this)->impl_AddRef(); }
	STDMETHOD_(ULONG, Release)() { return  static_cast<T*>(this)->impl_Release(); }

protected:
	refcounter_t refcounter;
};

template <class T, class RefCounter = RefCounterMT_t<1> >
struct IUnknown_this_t :IUnknown_base_impl<T, IUnknown, RefCounter>
{
	inline HRESULT impl_QueryInterface(const IID& riid, void** ppvObject)
	{
		if (!ppvObject) return E_POINTER;
		if (riid == IID_IUnknown)
		{
			*ppvObject = static_cast<T*>(this);
			this->AddRef();
			return S_OK;
		}
		else if (riid == IID_NULL)
		{
			*ppvObject = static_cast<T*>(this);
			return S_OK;
		}
		return E_NOINTERFACE;;
	}
};


inline bool operator <(const GUID& i1, const GUID& i2)
{
	typedef LONG64* PL64;

	LONG64 l1 = *PL64(&i1.Data1), l2 = *PL64(&i2.Data1);
	if (l1 < l2) return true;
	else if (l1 > l2) return false;

	return (*PL64(i1.Data4) < *PL64(i1.Data4));

};





template <class T, class I, class RefCounter = RefCounterMT_t<1> >
struct IUnknown_impl :IUnknown_base_impl<T, I, RefCounter>                //I
{
	//
	typedef void(*exit_proc_t)(void*);

	typedef aggregator_helper_helper<0> aggregator_helper_helper_t;

	typedef   std::map<GUID, aggregator_helper_helper_t >  uuid_map_t;

	//typedef RefCounter refcounter_t;

	inline HRESULT impl_QueryInterface(const IID& riid, void** ppvObject)
	{
		if (!ppvObject) return E_POINTER;
		/*
		if(riid==GUID_NULL)
		{
			*ppvObject=pcontext;
			return  S_OK;
		}*/
		if ((riid == IID_IUnknown) || (riid == __uuidof(I)))
		{
			*ppvObject = (T*)static_cast<T*>(this);
			this->AddRef();
			return S_OK;
		}
		HRESULT hr = E_NOINTERFACE;
		if ((!mapQI((IUnknown*)this, riid, ppvObject, hr)) && (riid == GUID_NULL)) {
			*ppvObject = pcontext;
			hr = S_OK;
		}

		return hr;
	}
	/*
	   STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject){

		   return static_cast<T*>(this)->impl_QueryInterface(riid,ppv);
	   }
	   STDMETHOD_(ULONG,AddRef)()	{ return refcounter.AddRef(); }
	   STDMETHOD_(ULONG,Release)()	{	return refcounter.ReleaseDestroy(static_cast<T*>(this));}

   */

	bool mapQI(IUnknown* punkOuter, REFIID riid, void** ppunk, HRESULT& hr)
	{
		//return (ah)&&(ah->innerQueryInterface(punkOuter,riid,ppunk,&hr));

		uuid_map_t::iterator i = uuid_map.find(riid);

		if (i != uuid_map.end())
		{
			//aggregator_helper_helper_t& ahh=i->second;
			return ((i->second))(punkOuter, riid, ppunk, hr);

		}




		//if(i!=uuid_map.end())			return (*(i->second))(punkOuter,riid,ppunk,hr);


		return false;
	}
	inline HRESULT set_aggregator(GUID iid, IUnknown* punk = 0)
	{
		HRESULT hr;
		//return aggregator_helper.reset(punk);
		aggregator_helper_helper_t ahh;

		if (FAILED(hr = ahh.reset(punk)))
			return hr;

		std::pair<uuid_map_t::iterator, bool> pp;
		uuid_map_t::iterator i;

		pp = uuid_map.insert(std::make_pair(iid, aggregator_helper_helper_t()));
		//i=pp.first;
		//i->second.flipflop(ahh);
		pp.first->second.flipflop(ahh).set_context(static_cast<T*>(this));
		return S_OK;
		//uuid_map.find(iid);

	}

	IUnknown_impl(void * cntx = 0, exit_proc_t onclose = 0, exit_proc_t onexit = 0) :pcontext(cntx), exit_proc(onclose), exit_proc_after(onexit) {}
	~IUnknown_impl()
	{
		if (exit_proc) exit_proc(pcontext);
		uuid_map.clear();
		if (exit_proc_after) exit_proc_after(pcontext);
	}

protected:
	//   refcounter_t refcounter;
	uuid_map_t uuid_map;
	void * pcontext;
	exit_proc_t exit_proc, exit_proc_after;

};

//iaggregator_container
inline HRESULT create_dummy_unk(IUnknown** ppunk)
{
	struct F :IUnknown_impl<F, IUnknown> {
		~F()
		{

		};
	};

	if (!ppunk) return E_POINTER;
	*ppunk = new F;
	return S_OK;
}


inline HRESULT create_iaggregator_container(iaggregator_container** ppout, void* pcntx = 0, void(*close_proc)(void*) = 0, void(*exit_proc)(void*) = 0)
{
	HRESULT hr;

	struct F_t :IUnknown_impl<F_t, iaggregator_container>
	{
		virtual HRESULT STDMETHODCALLTYPE SetAggregator(GUID iid, IUnknown* punk = 0)
		{
			return set_aggregator(iid, punk);
		}
		virtual HRESULT STDMETHODCALLTYPE GetContext(void** ppcontext)
		{
			if (!ppcontext) return E_POINTER;
			*ppcontext = pcontext;
			return S_OK;

		};

		F_t(void* pcntx, void(*close_proc)(void*), void(*exit_proc)(void*)) :IUnknown_impl<F_t, iaggregator_container>(pcntx, close_proc, exit_proc) {}
	};


	if (!ppout) return E_POINTER;

	F_t* pf = new F_t(pcntx, close_proc, exit_proc);

	*ppout = pf;

	return S_OK;


}


inline HRESULT set_FMT_to_aggregator_container(iaggregator_container* container, bool fspecmarshal = false)
{
	HRESULT hr;
	ipc_utils::smart_ptr_t<IUnknown> fmtm;
	if (fspecmarshal) {
		if (SUCCEEDED(hr = create_FTM_aggregator_helperSM(&fmtm.p)))
			hr = container->SetAggregator(__uuidof(IFreeThreadMarshal), fmtm);

	}
	else
	{
		if (SUCCEEDED(hr = create_FTM_aggregator_helper(&fmtm.p)))
			hr = container->SetAggregator(__uuidof(IMarshal), fmtm);
	}

	return hr;

}

inline HRESULT set_ExternalConnect_to_aggregator_container(iaggregator_container* container, IDispatch* pevent = 0)
{
	HRESULT hr;
	ipc_utils::smart_ptr_t<IUnknown> ec;
	if (SUCCEEDED(hr = create_ExternalConnection_aggregator_helper(pevent, &ec.p)))
		hr = container->SetAggregator(__uuidof(IExternalConnection), ec);
	//if(SUCCEEDED(hr=container->SetAggregator(__uuidof(IExternalConnection),ec)))
	//	hr=container->SetAggregator(__uuidof(IObjectWithSite),ec);
	return hr;

}

//template //smart_ptr_t<>
template <class Aggregator>
struct aggregator_helper_creator_t :aggregator_helper_base_t<aggregator_helper_creator_t<Aggregator> >
{
	typedef   Aggregator   aggregator_t;
	inline bool   inner_QI(IUnknown* Outer, REFIID riid, void** ppObj, HRESULT& hr) {

		return (aggregator) ? (*aggregator)(Outer, riid, ppObj, hr) : 0;
	}


	static ipc_utils::smart_ptr_t<aggregator_helper_creator_t> New(aggregator_t* aggregator)
	{
		ipc_utils::smart_ptr_t<aggregator_helper_creator_t> sA(new aggregator_helper_creator_t(aggregator), 0);
		//	int ic=ipc_utils::ref_count(sA.p);
			//return ipc_utils::smart_ptr_t<aggregator_helper_creator_t>(sA);
		return sA;
	}
	~aggregator_helper_creator_t() {
		delete aggregator;
	}
protected:
	aggregator_t* aggregator;
	aggregator_helper_creator_t(aggregator_t* _aggregator) :aggregator(_aggregator) {};


};

struct aggregator_container_helper
{
	HRESULT hr;
	ipc_utils::smart_ptr_t<iaggregator_container> container;
	aggregator_container_helper(void* pcntx = 0, void(*close_proc)(void*) = 0, void(*exit_proc)(void*) = 0)
	{
		hr = create_iaggregator_container(&container.p, pcntx, close_proc, exit_proc);
	}
	aggregator_container_helper(IUnknown* punk, bool f_default = 0)
	{

		hr = (punk) ? punk->QueryInterface(__uuidof(iaggregator_container), container._ppQI()) : E_POINTER;
		if (FAILED(hr) && f_default && (!punk))
			hr = create_iaggregator_container(&container.p);
	}





	inline HRESULT set_aggregator(GUID iid, IUnknown* pah)
	{
		if (FAILED(hr)) return hr;
		return hr = container->SetAggregator(iid, pah);
	}
	inline HRESULT set_aggregator(GUID iid, GUID iid2, IUnknown* pah)
	{
		bool f = SUCCEEDED(hr = set_aggregator(iid, pah));
		f&&SUCCEEDED(hr = set_aggregator(iid2, pah));
		return hr;
	}
	inline HRESULT set_aggregator(GUID iid, GUID iid2, GUID iid3, IUnknown* pah)
	{
		bool f = SUCCEEDED(hr = set_aggregator(iid, iid2, pah));
		f&&SUCCEEDED(hr = set_aggregator(iid3, pah));
		return hr;
	}
	inline HRESULT set_aggregator(GUID iid, GUID iid2, GUID iid3, GUID iid4, IUnknown* pah)
	{
		bool f = SUCCEEDED(hr = set_aggregator(iid, iid2, pah));
		f&&SUCCEEDED(hr = set_aggregator(iid3, iid4, pah));
		return hr;
	}

	inline aggregator_container_helper& add(GUID iid, IUnknown* pah)
	{
		set_aggregator(iid, pah);
		return *this;
	}
	inline aggregator_container_helper& add(GUID iid1, GUID iid2, IUnknown* pah)
	{
		return add(iid1, pah).add(iid2, pah);
	}
	inline aggregator_container_helper& add(GUID iid1, GUID iid2, GUID iid3, IUnknown* pah)
	{
		return add(iid1, iid2, pah).add(iid3, pah);
	}
	inline aggregator_container_helper& add(GUID iid1, GUID iid2, GUID iid3, GUID iid4, IUnknown* pah)
	{
		return add(iid1, iid2, pah).add(iid3, iid4, pah);
	}

	inline aggregator_container_helper& add_EC(IDispatch* pevent = 0)
	{
		set_ExternalConnect(pevent);
		return *this;
	}
	inline aggregator_container_helper& add_FMT(bool fspec_marshal = false)
	{
		set_FTM(fspec_marshal);
		return *this;
	}

	inline HRESULT set_ExternalConnect(IDispatch* pevent = 0)
	{
		if (FAILED(hr)) return hr;
		return hr = set_ExternalConnect_to_aggregator_container(container, pevent);
	}

	inline HRESULT set_FTM(bool fspec_marshal = false)
	{
		if (FAILED(hr)) return hr;
		return hr = set_FMT_to_aggregator_container(container, fspec_marshal);
	}

	inline operator iaggregator_container*() { return  container; }

	template <class I>
	inline HRESULT QueryInterface(I** pp)
	{
		if (FAILED(hr)) return hr;
		return container->QueryInterface(__uuidof(I), (void**)pp);
	}

	inline HRESULT QueryInterface(REFIID riid, void** pp)
	{
		if (FAILED(hr)) return hr;
		return container->QueryInterface(riid, (void**)pp);
	}



	inline HRESULT unwrap(DWORD* pcookie)
	{
		if (FAILED(hr)) return hr;
		return ipc_utils::GIT_t::get_instance().wrap((IUnknown*)container.p, pcookie);
	}

	inline HRESULT unwrap(VARIANT* pv)
	{
		if (FAILED(hr)) return hr;
		return ipc_utils::toVARIANT(container.p, pv);
	}

	inline operator HRESULT() { return hr; }


};

template <bool _FTM = true, bool _EC = true>
struct aggregator_container_helper_FMT_EC :aggregator_container_helper
{

	aggregator_container_helper_FMT_EC(void* pcntx = 0, void(*close_proc)(void*) = 0, void(*exit_proc)(void*) = 0)
		:aggregator_container_helper(pcntx, close_proc, exit_proc)
	{
		if (_FTM) set_FTM();
		if (_EC) set_ExternalConnect();

		//hr=create_iaggregator_container(&container.p,pcntx,close_proc,exit_proc);
	}
};
//



template <class T, class Base>
struct IDispatch_impl_base_t :Base
{


	inline T* thisT() {
		return static_cast<T*>(this);
	}



	inline	HRESULT impl_GetTypeInfoCount(UINT*) { return E_NOTIMPL; }
	inline	HRESULT impl_GetTypeInfo(UINT, LCID, ITypeInfo **) { return E_NOTIMPL; }

	inline	HRESULT impl_GetIDsOfNames(REFIID, LPOLESTR* pname, UINT cNames, LCID, DISPID* pdispid) {

		if ((cNames == 1) && (pname) && (pdispid)) return thisT()->findname(*pname, *pdispid);
		else 	return E_NOTIMPL;
	}

	inline HRESULT findname(LPOLESTR name, DISPID& dispid) { return E_NOTIMPL; }
	inline	HRESULT impl_Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*) { return E_NOTIMPL; }




	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) {
		return thisT()->impl_GetTypeInfoCount(pctinfo);
	}

	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) {
		return thisT()->impl_GetTypeInfo(iTInfo, lcid, ppTInfo);
	}

	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid) {
		return thisT()->impl_GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
	}
	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
		//DISPID_VALUE;
		return static_cast<T*>(this)->impl_Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
	}

};



template <class T, class IDispatchX = IDispatch, class RefCounter = RefCounterMT_t<1> >
struct IDispatch_base_t :IDispatch_impl_base_t<T, IUnknown_base_impl<T, IDispatchX, RefCounter > > {};




template <class T, class IDispatchX = IDispatch >
struct IDispatch_impl_t :IDispatch_impl_base_t<T, base_aggregator_t<T, IDispatchX> >
{
	inline bool operator()(IUnknown* punkOuter, REFIID riid, void** ppObj, HRESULT& hr)
	{
		return static_cast<T*>(this)->inner_QI(punkOuter, riid, ppObj, hr);
	}

};


struct IDispatch_empty_t :IDispatch_base_t<IDispatch_empty_t> {

	~IDispatch_empty_t()
	{

	}
};

template <int rcounter = 1, class IDispatchX = IDispatch >
struct IDispatch_empty_RC_t :IDispatch_base_t<IDispatch_empty_RC_t<rcounter>, IDispatchX >
{
	~IDispatch_empty_RC_t()
	{

	}
};

struct IDispatch_empty_aggregator_t :IDispatch_impl_t<IDispatch_empty_aggregator_t> {};

template <class T, class IDispatchX = IDispatch>
struct IGITDispatch_t :base_aggregator_t<T, IDispatchX>
{

	typedef ipc_utils::smart_ptr_t<IDispatchX>    disp_ptr_t;
	struct disp_GIT_t
	{
		HRESULT hr;
		disp_ptr_t disp;
		disp_GIT_t(DWORD dwCookie)
		{
			hr = (dwCookie) ? ipc_utils::GIT_t::get_instance().unwrap(dwCookie, &disp.p, false) : E_NOTIMPL;
		}
		inline IDispatchX* operator ->()
		{
			// return (SUCCEEDED(hr))?disp.p:get_NOTIMPL();
			return disp.p;

		}
		/*
		  IDispatchX* get_NOTIMPL()
		  {
			  struct ED_t:IDispatch_base_t<ED_t,IDispatchX,RefCounterFake_t<1> >
			  {
				  ED_t()
				  {

				  }
			  };
			  static ED_t& sed=class_initializer_T<ED_t>().get();
			  return static_cast<IDispatchX*>(&sed);
		  }
		  */

	};


	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
	{
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->GetTypeInfoCount(pctinfo);
		return hr;

	}
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) {
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->GetTypeInfo(iTInfo, lcid, ppTInfo);
		return hr;
	}
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
	{
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
		return hr;

	}
	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
		return hr;

	}



	inline bool operator()(IUnknown* punkOuter, REFIID riid, void** ppObj, HRESULT& hr)
	{
		return inner_QI(punkOuter, riid, ppObj, hr);
	}

	IGITDispatch_t(IDispatchX* pdisp, DWORD SiteCookie = 0) :dwCookie(0), dwSiteCookie(SiteCookie)
	{
		ipc_utils::GIT_t::get_instance().wrap(pdisp, &dwCookie);
	}

	IGITDispatch_t(DWORD Cookie, DWORD SiteCookie = 0) :dwCookie(Cookie), dwSiteCookie(SiteCookie) {};

	~IGITDispatch_t()
	{
		if (dwCookie) ipc_utils::GIT_t::get_instance().unwrap(dwCookie);
		if (dwSiteCookie) ipc_utils::GIT_t::get_instance().unwrap(dwSiteCookie);
	}

	DWORD dwCookie, dwSiteCookie;


};


template <class T>
struct IGITDispatchEx_t :IGITDispatch_t<T, IDispatchEx>
{
	IGITDispatchEx_t(IDispatchEx* pdisp, DWORD SiteCookie = 0) :IGITDispatch_t<T, IDispatchEx>(pdisp, SiteCookie) {};
	IGITDispatchEx_t(DWORD Cookie, DWORD SiteCookie = 0) :IGITDispatch_t<T, IDispatchEx>(Cookie, SiteCookie) {};

	inline bool operator()(IUnknown* punkOuter, REFIID riid, void** ppObj, HRESULT& hr)
	{
		return inner_QI(punkOuter, riid, ppObj, hr) || (inner_QI(punkOuter, riid, __uuidof(IDispatch), ppObj, hr));
	}

	// IDispatchEx


	virtual HRESULT STDMETHODCALLTYPE GetDispID(
		/* [in] */ __RPC__in BSTR bstrName,
		/* [in] */ DWORD grfdex,
		/* [out] */ __RPC__out DISPID *pid) {
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->GetDispID(bstrName, grfdex, pid);
		return hr;

	}



	virtual /* [local] */ HRESULT STDMETHODCALLTYPE InvokeEx(
		/* [in] */ DISPID id,
		/* [in] */ LCID lcid,
		/* [in] */ WORD wFlags,
		/* [in] */ DISPPARAMS *pdp,
		/* [out] */ VARIANT *pvarRes,
		/* [out] */ EXCEPINFO *pei,
		/* [unique][in] */ IServiceProvider *pspCaller) {
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller);
		return hr;

	}


	virtual HRESULT STDMETHODCALLTYPE DeleteMemberByName(
		/* [in] */ __RPC__in BSTR bstrName,
		/* [in] */ DWORD grfdex) {
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->DeleteMemberByName(bstrName, grfdex);
		return hr;

	}


	virtual HRESULT STDMETHODCALLTYPE DeleteMemberByDispID(
		/* [in] */ DISPID id) {
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->DeleteMemberByDispID(id);
		return hr;

	}


	virtual HRESULT STDMETHODCALLTYPE GetMemberProperties(
		/* [in] */ DISPID id,
		/* [in] */ DWORD grfdexFetch,
		/* [out] */ __RPC__out DWORD *pgrfdex) {
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->GetMemberProperties(id, grfdexFetch, pgrfdex);
		return hr;

	}


	virtual HRESULT STDMETHODCALLTYPE GetMemberName(
		/* [in] */ DISPID id,
		/* [out] */ __RPC__deref_out_opt BSTR *pbstrName) {
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->GetMemberName(id, pbstrName);
		return hr;

	}


	virtual HRESULT STDMETHODCALLTYPE GetNextDispID(
		/* [in] */ DWORD grfdex,
		/* [in] */ DISPID id,
		/* [out] */ __RPC__out DISPID *pid) {
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->GetNextDispID(grfdex, id, pid);
		return hr;

	}


	virtual HRESULT STDMETHODCALLTYPE GetNameSpaceParent(/* [out] */ __RPC__deref_out_opt IUnknown **ppunk)
	{
		HRESULT  hr;
		disp_GIT_t disp_GIT(dwCookie);
		if (SUCCEEDED(hr = disp_GIT.hr))
			hr = disp_GIT->GetNameSpaceParent(ppunk);
		return hr;
	}

};

inline static HRESULT create_GITDispatchEx_aggregator_helper(DWORD disp_cookie, IUnknown** punk, DWORD site_cookie = 0)
{

	struct F :aggregator_helper_base_t<F>
	{
		struct IGT :IGITDispatchEx_t<IGT>
		{
			IGT(DWORD dw, DWORD dws) :IGITDispatchEx_t<IGT>(dw, dws) {};
		};

		inline bool   inner_QI(IUnknown* Outer, REFIID riid, void** ppObj, HRESULT& hr) {

			return disp(Outer, riid, ppObj, hr);
		}


		F(DWORD disp_cookie, DWORD site_cookie) :disp(disp_cookie, site_cookie) {};

		//IGITDispatch_t<IUnknown> disp;
		IGT disp;

	};


	if (!punk) return E_POINTER;

	F* pf = new F(disp_cookie, site_cookie);

	*punk = pf;

	return S_OK;
};


inline static HRESULT create_GITDispatch_aggregator_helper(DWORD disp_cookie, IUnknown** punk, DWORD site_cookie = 0)
{

	struct F :aggregator_helper_base_t<F>
	{
		struct IGT :IGITDispatch_t<IGT>
		{
			IGT(DWORD dw, DWORD dws) :IGITDispatch_t<IGT>(dw, dws) {};
		};

		inline bool   inner_QI(IUnknown* Outer, REFIID riid, void** ppObj, HRESULT& hr) {

			return disp(Outer, riid, ppObj, hr);
		}


		F(DWORD disp_cookie, DWORD site_cookie) :disp(disp_cookie, site_cookie) {};

		//IGITDispatch_t<IUnknown> disp;
		IGT disp;

	};


	if (!punk) return E_POINTER;

	F* pf = new F(disp_cookie, site_cookie);

	*punk = pf;

	return S_OK;
};

inline static HRESULT create_GITDispatch_aggregator_helper(IDispatch* pdisp, IUnknown** ppunk, bool *pfex = NULL, DWORD site_cookie = 0)
{
	DWORD dwcookie;
	HRESULT hr;
	OLE_CHECK_PTR(pdisp);
	ipc_utils::smart_ptr_t<IDispatchEx> diex;
	if (SUCCEEDED(hr = pdisp->QueryInterface(__uuidof(IDispatchEx), diex._ppQI())))
	{
		if (SUCCEEDED(hr = ipc_utils::GIT_t::get_instance().wrap(diex.p, &dwcookie)))
			hr = create_GITDispatchEx_aggregator_helper(dwcookie, ppunk, site_cookie);
	}
	else
	{

		if (SUCCEEDED(hr = ipc_utils::GIT_t::get_instance().wrap(pdisp, &dwcookie)))
			hr = create_GITDispatch_aggregator_helper(dwcookie, ppunk, site_cookie);

	}
	if (pfex) *pfex = diex.p;
	return hr;
}



template <class Intf, bool freemarshall = true>
struct DummyIDispatchAggregator_t :IDispatch
{


	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv) {

		return pOuter->QueryInterface(iid, ppv);
	}

	virtual ULONG __stdcall AddRef() { return pOuter->AddRef(); }
	virtual ULONG __stdcall Release() { return pOuter->Release(); }
	///============================================================


	///============================================================
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
	{
		if (pctinfo) *pctinfo = 0;
		return S_OK;
	}
	STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo**) { return E_NOTIMPL; }
	STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, UINT, LCID, DISPID*) { return E_NOTIMPL; }
	STDMETHOD(Invoke)(DISPID id, REFIID, LCID, WORD flags, DISPPARAMS*, VARIANT* pres, EXCEPINFO*, UINT*) {
		if ((id == 0) && (is_get(flags))) {

			return VariantCopy(pres, &info);
		}
		return E_NOTIMPL;
	}

	DummyIDispatchAggregator_t(Intf* _pOuter = 0) :pOuter(_pOuter) {}


	inline bool   operator()(Intf* Outer, REFIID riid, void** ppObj, HRESULT& hr)
	{

		if (__uuidof(IDispatch) == riid)
		{
			InterlockedExchangePointer((void**)&pOuter, Outer);
			*ppObj = static_cast<IDispatch*>(this);
			hr = S_OK;
			return true;
		}
		else return ftm(Outer, riid, ppObj, hr);

	}

	//protected:
	variant_t info;
	union {
		LARGE_INTEGER __laign64;
		Intf* pOuter;
	};
	FreeThreadedMarshaler_t<freemarshall, Intf>  ftm;
};


