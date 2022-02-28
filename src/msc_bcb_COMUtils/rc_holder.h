#pragma once

#include "dummy_dispatch_aggregator.h"





struct RefCountDef_t
{
	template<class O>
	inline  long AddRef(O* p)
	{
		return _InterlockedIncrement(&refcount);
	}
	template<class O>
	inline  long Release(O* p)
	{
		long l= _InterlockedDecrement(&refcount);
		if(l==0)
			delete p;
		return l;
	}

	
	RefCountDef_t(long _refcount=1):refcount(_refcount){};
   private:
	   volatile long refcount;
	   RefCountDef_t(const RefCountDef_t&){};
	   void operator=(const RefCountDef_t&){}; 

};

struct RefCountFake_t
{
	template<class O> 
	inline  long AddRef(O* p){	return 1;}
	template<class O> 
	inline  long Release(O* p){	return 1;}
	RefCountFake_t(long _refcount=1){};
};


struct v_holder_base_t
{
	virtual ~v_holder_base_t(){};
};
template<class T>
struct v_holder_t:v_holder_base_t
{
     T t;
	 v_holder_t(){};
	 v_holder_t(const T& _t):t(_t){};
	 
};


  //template<class T, const IID& uiid_T=__uuidof(T)>
  template<class T, const IID& uiid_this=IID_NULL,class RefCount_t=RefCountDef_t> 
  struct i_holder_base_t:IUnknown
  {

	  virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv)
	  {

		      if(!ppv) return E_POINTER;
			  
				  
              if(iid==__uuidof(IUnknown)) 
			  { 
                 counter.AddRef(this);
				 *ppv=static_cast<IUnknown*>(this);
				 return S_OK;
			  }
            HRESULT hr=E_NOINTERFACE;

			if((*this_T())(iid,ppv,hr)) 
				return hr;
			else   aggregator(this_T(),iid,ppv,hr);

    		 return hr;
	  }

    virtual ULONG  __stdcall AddRef()
	{
		return counter.AddRef(this_T());
	}
	virtual ULONG  __stdcall Release()
	{
		return counter.Release(this_T());
	}

   inline T* this_T()
   {
	   T* t=static_cast<T*>(this);
	   return t;
   }


   inline bool operator()(REFIID riid,void** ppv,HRESULT& hr)
   {
	   if(riid==uiid_this) 
	   { 
		   *ppv=this_T(); 
		   hr=S_OK;
	   }
	   return SUCCEEDED(hr);
   }


	 
	

	RefCount_t counter;
	DummyIDispatchAggregator_t<T> aggregator;
/*
	T *p,*pdel;
	i_holder_t(T *_p=0):p(_p),pdel(_p){};
	~i_holder_t(){
		delete pdel; 
	};
*/
	

  };





  template<class T, const IID& uiid_this=IID_NULL, class RefCount_t=RefCountDef_t> 
  struct i_holder_t:i_holder_base_t<i_holder_t<T,uiid_this,RefCount_t>,uiid_this,RefCountDef_t>
  {
	  T t;  
	  i_holder_t(){};
	  inline bool operator()(REFIID riid,void** ppv,HRESULT& hr)
	  {
		  if(riid==uiid_this) 
		  { 
			  *ppv=&t; 
			  hr=S_OK;
		  }
		  return SUCCEEDED(hr);
	  }

  };

  
  template<class T, const IID& uiid_this=IID_NULL, class RefCount_t=RefCountDef_t> 
  struct i_holder_ptr_t:i_holder_base_t<i_holder_t<T,uiid_this,RefCount_t>,uiid_this,RefCountDef_t>
  {
	   T *p;  
	  i_holder_ptr_t(T *_p=0):p(_p){};
	  ~i_holder_ptr_t(){ delete p;};

	  inline bool operator()(REFIID riid,void** ppv,HRESULT& hr)
	  {
		  if(riid==uiid_this) 
		  { 
			  *ppv=p; 
			  hr=S_OK;
		  }
		  return SUCCEEDED(hr);
	  }

  };
