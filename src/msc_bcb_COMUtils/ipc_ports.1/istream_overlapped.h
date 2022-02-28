#pragma once
#include <windows.h>
#include "dummy_dispatch_aggregator.h"



// 
//static const GUID <<name>> = 
//{ 0x9d5b09e0, 0xa6fc, 0x4d1f, { 0xaf, 0xc0, 0xb6, 0xd, 0xb3, 0xfc, 0x49, 0xb0 } };

struct  __declspec(uuid("{9D5B09E0-A6FC-4D1F-AFC0-B60DB3FC49B0}")) i_get_handle_t
{
	virtual HRESULT get_handle(HANDLE* h)=0;
};


template <class Ovl>
struct os_io_op_t
{

  typedef typename Ovl ovl_t;

  struct locker_t
  {
	  ovl_t* p;
	  locker_t(ovl_t* _p):p(_p){ p->lock();}
	  ~locker_t(){ p->unlock();}
  };

static   inline HRESULT CheckOverlappedResult(ovl_t* povl,bool f,ULONG* pcb)
  {
	  HRESULT hr;
	  if(f) return S_OK;

	  hr=GetLastError();
	  if(ERROR_IO_PENDING==hr)
	  {
		  if(f=GetOverlappedResult(povl->fd(),povl,pcb,true))
		  {
		    return S_OK;    	  
		  }

		  hr=GetLastError();

	  }


	  return HRESULT_FROM_WIN32(hr);

  }


static inline    HRESULT read(ovl_t* povl,void* pv, ULONG cb, ULONG* pcbRead)
{
	locker_t lock(povl);
	BOOL f = ReadFile(povl->fd(), pv, cb, pcbRead, povl);
	return CheckOverlappedResult(povl,f,pcbRead);

 }

static inline    HRESULT write(ovl_t* povl,const void* pv, ULONG cb, ULONG* pcbWritten)
{
	HRESULT hr;
	 ULONG& cbWritten=*pcbWritten;
	 
	if((pv==NULL)||(cb==0))
	{
		povl->close();
		*pcbWritten=0;
		return S_FALSE;
	}
	locker_t lock(povl);
	HANDLE h=povl->fd();
	BOOL f = WriteFile(h, pv, cb, pcbWritten,povl);
	 hr=CheckOverlappedResult(povl,f,pcbWritten);

	 if(FAILED(hr))
		 return hr;
	 if(cbWritten&&cb)
		 return hr;
	 

	 ULONG cb2=cb>>1,cb2r=cb-cb2,cbw;
      cbWritten=0;
	 
	 if(SUCCEEDED(hr=write(povl,pv,cb2,&cbw))) 
	 {
		 cbWritten+=cbw;
		 cb2r=cb-cbw;
		 if((cb2r>0)&&SUCCEEDED(hr=write(povl,((char*)pv)+cbw,cb2r,&cbw))) 
			     cbWritten+=cbw;

	 }
	  

	return hr;

}


};



template <class _Ovl,bool freemarshall=true,template<class> class _io_op=os_io_op_t>
class StreamOverlapped : public IStream
{

	typedef typename _Ovl ovl_t;
	typedef typename _io_op<ovl_t> io_operation_t;
	typedef typename DummyIDispatchAggregator_t<StreamOverlapped,freemarshall> dummy_dispatch_t;
	//typedef typename  FreeThreadedMarshaler_t<freemarshall> FreeThreadedMarshaler_t;

	struct get_handle_t:i_get_handle_t
{
	get_handle_t(ovl_t* _povl=0):povl(_povl){}
	virtual HRESULT get_handle(HANDLE* phandle)
	{
		HRESULT hr;
		if(!(phandle)) return hr=E_POINTER;		
		if((povl)&&(*phandle=povl->fd())&&(*phandle!=INVALID_HANDLE_VALUE))
			hr=S_OK;  
			else hr=E_HANDLE;
			return hr;
	};
	ovl_t* povl;
   };

	get_handle_t get_handle;
	public:
		StreamOverlapped(){};
	StreamOverlapped(ovl_t* _povl):povl(_povl),get_handle(_povl){
			}
	~StreamOverlapped(){}

static inline const GUID& uuid()
{
static const GUID s = 
{ 0x5c9ccfe4, 0xf52c, 0x4623, { 0x93, 0x1a, 0x45, 0xef, 0x18, 0xb0, 0x71, 0x86 } };
    return s;
}
	



inline  static   HRESULT CreateInstance(ovl_t* povl, IStream ** ppStream)
    {        
		if(!ppStream ) return E_POINTER;
        *ppStream = new StreamOverlapped(povl);
               
        return S_OK;
    }

template < class I>
inline	 static HRESULT CreateInstance(ovl_t* povl, I** ppI)
	{      
		HRESULT hr;
		 IStream * ps;
		 if(SUCCEEDED(hr=CreateInstance(fd,&(ps=0))))
		 {
               hr=ps->QueryInterface(__uuidof(I),ppI);
             ps->Release();
		 }

		return hr;
	}


// IUnknown Interface
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
    { 
		if(!ppvObject) return E_POINTER;
		 HRESULT hr=E_NOINTERFACE;
		 

		 

        if (iid == __uuidof(IUnknown)
            || iid == __uuidof(IStream)
            || iid == __uuidof(ISequentialStream))
        {
            *ppvObject = static_cast<IStream*>(this);
            AddRef();
            hr=S_OK;
        } 
		else if (iid==__uuidof(i_get_handle_t))
		{
			*ppvObject = static_cast<i_get_handle_t*>(&get_handle);
			hr=S_OK;
		}
        //if(!fmt(static_cast< IUnknown*>(this),iid,ppvObject,hr))
		     ddisp(this,iid,ppvObject,hr);
		return hr;
    }

	virtual ULONG STDMETHODCALLTYPE AddRef(void) 
	{ 
		return povl->AddRef(); 
	}

	virtual ULONG STDMETHODCALLTYPE Release(void) 
	{

		return povl->Release();
	}

    // ISequentialStream Interface
public:

	struct safe_null_ptr
	{
		ULONG l;
		safe_null_ptr(ULONG*& pcb)
		{
           if(pcb==0) pcb=&l; 
		}
	};


    virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
    {
		HRESULT hr;
		
		safe_null_ptr sn(pcbRead);

		return hr=io_operation_t::read(povl,pv,cb,pcbRead);

    }

    virtual HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG* pcbWritten)
    {
		HRESULT hr;
		safe_null_ptr sn(pcbWritten);

		return hr=io_operation_t::write(povl,pv,cb,pcbWritten);

     }

    // IStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER)
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*,
        ULARGE_INTEGER*) 
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD cb)                                      
    { 
		HRESULT hr;
		return hr=io_operation_t::read(povl,0,0,&cb);
    }
    
    virtual HRESULT STDMETHODCALLTYPE Revert(void)                                       
    { 
		povl->close();
		return S_OK;   
        
    }
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)              
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)            
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **)                                  
    { 
        return E_NOTIMPL;   
    }

    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER, DWORD,ULARGE_INTEGER*)
    { 
        return E_NOTIMPL;   
    }

    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* , DWORD ) 
    {
        return E_NOTIMPL;   
    }

//  blende  from  moronic MatlabActiveX developer;


private:
    
    ovl_t* povl;
	//FreeThreadedMarshaler_t fmt;
	dummy_dispatch_t ddisp;

};


