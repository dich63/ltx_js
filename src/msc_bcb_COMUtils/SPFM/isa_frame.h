#pragma once
#include <Windows.h>

struct isa_frame
{
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject)=0;
	virtual ULONG __stdcall AddRef()=0;
	virtual ULONG __stdcall Release()=0;
	virtual  BOOL __stdcall TryLock(DWORD dwtimeout=0)=0;
	virtual  int __stdcall Lock(VARIANT* pv=0)=0;  
	virtual  int __stdcall Unlock(VARIANT* pv=0)=0;
	virtual  LPSAFEARRAY __stdcall SAFEARRAY_ptr(VARIANT* pv=0)=0;

	inline long long get_total_count()
	{
		return *(long long*)( ((char*)const_cast<isa_frame*>(this))+sizeof(void*));
	}

	inline VARTYPE get_vt()
	{
		return *(int*)(((char*)const_cast<isa_frame*>(this))+sizeof(void*)+sizeof(long long));
	}

	inline static const GUID& iid()
	{
		// {3B0A4445-3F70-4a9d-8AA0-6CF7A61EFFBF}
		static const GUID sgiud = 
		{ 0x3b0a4445, 0x3f70, 0x4a9d, { 0x8a, 0xa0, 0x6c, 0xf7, 0xa6, 0x1e, 0xff, 0xbf } };
		return sgiud;
	}

};

;
template <class MultiSAFEARRAY=void>
struct isa_frame_list:isa_frame
{
	virtual  char* __stdcall getFramePtr(LONG64 ind)=0;
	virtual   MultiSAFEARRAY* __stdcall getHeader()=0;
	virtual  LONG64 __stdcall GetCursor()=0;
	virtual  LONG64 __stdcall SetCursor(LONG64 l)=0;
	virtual  char* __stdcall getFramePtrEx(LONG64 ind,LONG64 offset_begin,LONG64 offset_end)=0;
	virtual  void __stdcall Decommit()=0;
	virtual HRESULT get_unicode_string(LONG64 nbegin, LONG64 nend , DWORD codepage ,  BSTR* pVal)=0;
	virtual HRESULT put_unicode_string(LONG64 nbegin, LONG64 nend , DWORD codepage ,  BSTR NewVal)=0;
	virtual  char* __stdcall getPtr(LONG64 offset_begin,LONG64 offset_end)=0;

	
    inline LONG64 getFrameSize()
	{
		return getHeader()->item_size_b;
	}

	inline static const GUID& iid()
	{
		// {C1C7C7E3-263D-4eb9-AC34-7E5AE61BDD82}
		static const GUID sgiud = 
		{ 0xc1c7c7e3, 0x263d, 0x4eb9, { 0xac, 0x34, 0x7e, 0x5a, 0xe6, 0x1b, 0xdd, 0x82 } };

		return sgiud;
	}

};


