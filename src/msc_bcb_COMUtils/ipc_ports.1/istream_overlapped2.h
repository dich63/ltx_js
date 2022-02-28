#pragma once
#include <windows.h>

template <int ii=1>
class StreamOverlapped : public IStream
{

	public:
	StreamOverlapped(){};
	StreamOverlapped(HANDLE hFile,OVERLAPPED* povl):_hFile(hFile),_povl(povl){}
	StreamOverlapped(SOCKET s,OVERLAPPED* povl):_hFile(HANDLE(s)),_povl(povl){}
    ~StreamOverlapped(){}



	template <class FD>
    HRESULT static CreateInstance(FD fd,OVERLAPPED* povl, IStream ** ppStream)
    {        
		if(!ppStream ) return E_POINTER;
        *ppStream = new StreamOverlapped(fd,povl);
               
        return S_OK;
    }

template <class FD, class I>
	HRESULT static CreateInstance(FD fd,OVERLAPPED* povl, I** ppI)
	{      
		HRESULT hr;
		 IStream * ps;
		 if(SUCCEEDED(hr=CreateInstance(fd,&(ps=0))))
		 {
               hr=QueryInterface(__uuidof(I),ppI);
             ps->Release();
		 }

		return hr;
	}

// IUnknown Interface
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
    { 
		if(!ppvObject) return E_POINTER;
        if (iid == __uuidof(IUnknown)
            || iid == __uuidof(IStream)
            || iid == __uuidof(ISequentialStream))
        {
            *ppvObject = static_cast<IStream*>(this);
            AddRef();
            return S_OK;
        } else
            return E_NOINTERFACE; 
    }

	virtual ULONG STDMETHODCALLTYPE AddRef(void) 
	{ 
		return 1; 
	}

	virtual ULONG STDMETHODCALLTYPE Release(void) 
	{

		return 1;
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

inline HRESULT CheckOverlappedResult(bool f,ULONG* pcb)
{
	 HRESULT hr;
     if(f) return S_OK;

	 hr=GetLastError();
	 if((ERROR_IO_PENDING==hr)&&(_povl))
	 {
		 if(f=GetOverlappedResult(_hFile,_povl,pcb,true))
			 return S_OK;

		 hr=GetLastError();

	 }


	 return HRESULT_FROM_WIN32(hr);

}

    virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
    {
		HRESULT hr;
		
		safe_null_ptr sn(pcbRead);

        BOOL f = ReadFile(_hFile, pv, cb, pcbRead, _povl);
		return hr=CheckOverlappedResult(f,pcbRead);
    }

    virtual HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG* pcbWritten)
    {
		HRESULT hr;
		safe_null_ptr sn(pcbWritten);
        BOOL f = WriteFile(_hFile, pv, cb, pcbWritten,_povl);
		return hr=CheckOverlappedResult(f,pcbWritten);
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
    
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD)                                      
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Revert(void)                                       
    { 
        return E_NOTIMPL;   
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

private:
    HANDLE _hFile;
    OVERLAPPED* _povl;
};