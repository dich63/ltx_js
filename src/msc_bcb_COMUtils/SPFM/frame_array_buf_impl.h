#pragma once


#include "membuffer.h"
#include "frame_array_buf.h"



template <class MVB_DATA>
struct safearray_frame_buf:isa_frame_buf,item_frame_buf<MultiSAFEARRAY>
{
	MVB_DATA* pmvb_data;
	 IUnknown* pUnkOuter;
	 //	 typedef MultiSAFEARRAY::SAFEARRAY_MAP_t SAFEARRAY_MAP_t;
	 SAFEARRAY_F m_safearray;
	 SAFEARRAY_MAP_t * m_psafearrayf;
	 HRESULT hrlast;

    safearray_frame_buf():pmvb_data(0),pUnkOuter(0),m_psafearrayf(0){};

	//inline SAFEARRAY_F& safearrayf()
inline	SAFEARRAY_MAP_t& safearrayf()
	{
       return  *m_psafearrayf;
	}

  inline	SAFEARRAY_F& safearray()
 {
	return  m_safearray;
  }

    inline void*& data_ptr_ref()
	{
      return safearray().m_array.pvData;
	}

	virtual HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject)
	{
	//
		if(pUnkOuter) return pUnkOuter->QueryInterface(riid,ppvObject);
		return E_NOTIMPL;
	};
	virtual ULONG __stdcall AddRef()
	{
	//
		if(pUnkOuter) return pUnkOuter->AddRef();
		return 1;
	};
	virtual ULONG __stdcall Release()
	{
	//
		if(pUnkOuter) return pUnkOuter->Release();
		return 1;
	};


	virtual  LONG64 __stdcall GetCursor()
	{
		if(!pmvb_data) return -1;
		HANDLE hmutex= *(PHANDLE(pmvb_data)+1);
		WaitForSingleObject(hmutex,INFINITE);
		LONG64 l=pheader->cursor;
		ReleaseMutex(hmutex);
		return l;

	}
	virtual  LONG64 __stdcall SetCursor(LONG64 l)
	{
		if(!pmvb_data) return -1;

		PHANDLE phmutex= PHANDLE(pmvb_data)+1;
		WaitForMultipleObjects(2,phmutex,2,INFINITE);
		long lo=pheader->cursor;
        pheader->cursor=l;
        ReleaseMutex(phmutex[1]);ReleaseMutex(phmutex[0]);
         return lo;  
	};


   inline  int inline_TryLock(DWORD dwtimeout=0)
   {
	   if(pmvb_data)
	   {
		   HANDLE hmutex= *(PHANDLE(pmvb_data)+1);
		   return WAIT_TIMEOUT!=WaitForSingleObject(hmutex,dwtimeout);
	   }
	   return 1;
   }

   inline  int inline_Unlock()
   {
	   if(pmvb_data)
	   {
		   //pheader->safearray.m_array.pvData=0;
		   data_ptr_ref()=0;
		   HANDLE hmutex= *(PHANDLE(pmvb_data)+1);
		   ReleaseMutex(hmutex);
	   }
	   return 0;
   };

   virtual BOOL __stdcall TryLock(DWORD dwtimeout=0)
   {

	   return inline_TryLock(dwtimeout);
   }
	virtual  int __stdcall Lock(VARIANT* pv=0)
	{
		inline_TryLock(INFINITE);
		if(pv) *pv=safearray();
		return 1; 
	}
	virtual  int __stdcall Unlock(VARIANT* pv=0)
	{
        if(pv) pv->vt=0;
		return inline_Unlock();
	};

	virtual  LPSAFEARRAY __stdcall SAFEARRAY_ptr(VARIANT* pv=0)
	{
		if(pv) *pv=safearray();
		return &(safearray().m_array);
	}
	inline void* mbv_init(MVB_DATA* pmvb,IUnknown* pUnk=0)
	{
		pmvb_data=pmvb;
		pUnkOuter=pUnk;
        if(!pmvb_data) return 0;    
		init(*(PHANDLE(pmvb_data)));
		if(pheader)
		{
			pheader->offset=sizeof(MultiSAFEARRAY);
		    m_psafearrayf=&(pheader->safearray);
			m_safearray=pheader->safearray;

		}
		return pheader;
	}

	inline LONG64 get_offset_begin()
	{
       return pmvb_data->userdata.begin_off;
	}
	inline LONG64 get_offset_end()
	{
		return pmvb_data->userdata.end_off;
	}
	
	virtual  char* __stdcall getFramePtr(LONG64 ind)
	{
		return getFramePtrEx(ind,get_offset_begin(),get_offset_end());
	}

	virtual  char* __stdcall getPtr(LONG64 offset_b,LONG64 offset_e)
	{
              LONG64 f=get_offset_begin();  
        return getFramePtrEx(0,f+offset_b,f+offset_e);

	};

    virtual  char* __stdcall getFramePtrEx(LONG64 ind,LONG64 offset_begin,LONG64 offset_end)
	{
		if(ind<0) ind=GetCursor();
		//long64
		SetLastError(E_FAIL);
		offset_begin+=pheader->offset;
		long64 isizeb=pheader->item_size_b;
		char *p=(char*)(data_ptr_ref()=get_ptr(ind*isizeb+offset_begin,isizeb+offset_end));
		if(!p) hrlast=HRESULT_FROM_WIN32(GetLastError());
		return p;
	};


	virtual   MultiSAFEARRAY* __stdcall getHeader()
	{
		return pheader;
	};
	virtual  void __stdcall Decommit()
	{
		decommit();
	}

	virtual HRESULT get_unicode_string(LONG64 nbegin, LONG64 nend , DWORD CP_XX,  BSTR* pVal)
	{
		decommiter_t  decommiter(this);
		HRESULT hr;
		if(!pVal) return E_POINTER;
		if(nend==-4) nend=GetCursor();

		LONG64 size=nend-nbegin;
		if(size==0) { *pVal=0;return S_OK;};
		if(size<0) return E_INVALIDARG;
		char* p=( char*) getFramePtrEx(0,nbegin,nend);
		if(!p) return hr=hrlast;


		//	 char_mutator
		BSTR bout=0;
		size_t cb=MultiByteToWideChar(CP_XX,0,p,DWORD(size),0,0);
		if(cb)
		{
			bout=SysAllocStringLen(0,cb);
			if(!bout)		 return hr=HRESULT_FROM_WIN32(GetLastError());

			cb=MultiByteToWideChar(CP_XX,0,p,DWORD(size),bout,cb);
			if(!cb) 
		 {
			 hr=HRESULT_FROM_WIN32(GetLastError());
			 SysFreeString(bout);
			 return hr;
		 }

			*pVal=bout;
			hr=S_OK;

		}
		else hr=HRESULT_FROM_WIN32(GetLastError());


		return hr;

	};

	virtual HRESULT put_unicode_string(LONG64 nbegin, LONG64 nend , DWORD CP_XX ,  BSTR NewVal)
	{

		decommiter_t  decommiter(this);
		HRESULT hr;
		size_t cb=SysStringLen(NewVal);
		if(cb)
		{
			size_t size=WideCharToMultiByte(CP_XX,0,NewVal,cb,0,0,0,0);
			if(!size) return  hr=HRESULT_FROM_WIN32(GetLastError());

			char* p=( char*) getFramePtrEx(0,nbegin,nbegin+LONG64(size));
			if(!p) return hr=hrlast;

			size=WideCharToMultiByte(CP_XX,0,NewVal,cb,p,size,0,0);
			if(size) SetCursor(nbegin+LONG64(size));

		}
		else SetCursor(nbegin);


		//if(nbend==-4) nbend=m_safearray_frame_buf.GetCursor();
		return hr=S_OK;
	

	}

	template <typename N>
	inline  N* operator[](long64 ind)
	{
		long64 offset=pheader->offset;
		long64 isizeb=pheader->item_size_b;
		return (N*)get_ptr(ind*isizeb+offset,isizeb);
	}

};

//template<class Header,class Item=char>
//struct item_frame_buf
