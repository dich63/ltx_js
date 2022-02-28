#pragma once
#include <windows.h>
#include <stdio.h>
#include <memory>
#include <U\pp\static_constructors_0.h>

#define PROCESS_ENVIRON_PREFIX  "_DICH.PROCESS."
#define PROCESS_ENVIRON_PREFIX_W  L"_DICH.PROCESS."
#define INJMODAL_NAME "MODAL_HOOK_8061E332_02A2_4b1e_ADF7_DA8085D740C2"





struct exe_allocator:public class_initializer<exe_allocator>
{



static	void static_ctor()
	{
     get_heap();
	};

static	void static_dtor(){};
	template<class T>
inline static	T* create()
	{
		return ::new (::HeapAlloc(get_heap(),0,sizeof(T))) T();
	};


//
inline static void free(void*p)
	{
		if(p) ::HeapFree(get_heap(),0,p);
	}

	template<class T>
	void destroy(T* pt)
	{
		if(pt)
		{
			(pt)->~T();  
			::HeapFree(get_heap(),0,pt);
		}
	}


protected:
	struct s_heap
	{
		HANDLE m_h;
		s_heap()
		{
			m_h=HeapCreate(HEAP_CREATE_ENABLE_EXECUTE,0,0);
		}
		~s_heap()
		{
			HeapDestroy(m_h);
		}

	};

	inline	static  HANDLE& get_heap()
	{
		static s_heap sh;
		return sh.m_h;
	}

};






template <class T>
class RefT:public T
{ 

};

template <class T>
class process_data_map
{
protected:
	T* m_pdata;
	HANDLE m_hmap,m_hmutex;
public:
	
	process_data_map():m_pdata(NULL),m_hmap(NULL),m_hmutex(NULL){};
	~process_data_map(){close();};


inline HANDLE hmap(){return m_hmap;} 
inline HRESULT Lock()
{
 WaitForSingleObject(m_hmutex,INFINITE);
  return S_OK;
};
inline void Unlock()
{
	ReleaseMutex(m_hmutex);
};



T* open(bool fnew=false)
{
     wchar_t buf[1024];
	 wchar_t *p=PROCESS_ENVIRON_PREFIX_W L"INJMODAL.MUTEX";
 
  if(fnew)
  {
	  SECURITY_ATTRIBUTES sa={sizeof(sa),NULL,true}; 
      m_hmutex=CreateMutex(&sa,1,NULL); 


  }


     if(GetEnvironmentVariableW(p,buf,1024))
	 {
	 }

   

 return 0;
}



T* open(LPCSTR prefix,DWORD pid=GetCurrentProcessId(),bool fcreate=0)
 {

	 char buf[256];
	 sprintf_s(buf,256,"%s:%08X:MX",prefix,pid);
     m_hmutex=CreateMutexA(NULL,0,buf); 
	 sprintf_s(buf,256,"%s:%08X:FM",prefix,pid);
	 if(fcreate)
	 {
		 Lock();
		 //CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, 
		//	 PAGE_READWRITE, 0, sizeof(*m_psi), szResult);
		 if( m_hmap=CreateFileMappingA(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(T),buf))
		 { 
			 bool fnew=ERROR_ALREADY_EXISTS!=GetLastError();
			 T* tmp;
          if(tmp=(T*)MapViewOfFile(m_hmap,FILE_MAP_WRITE|FILE_MAP_READ,0,0,0))
		  {
		    if(fnew) ZeroMemory(tmp,sizeof(T));
           InterlockedExchangePointer((void**)&m_pdata,tmp);
		  }
		 }
		 Unlock();
	 }
	 else 
	 {
		 m_hmap=OpenFileMappingA(FILE_MAP_WRITE|FILE_MAP_READ,0,buf);
		 m_pdata=(T*)MapViewOfFile(m_hmap,FILE_MAP_WRITE|FILE_MAP_READ,0,0,0);
	 }
	 return m_pdata;
 }
void close()
 {
   
   if(m_pdata) UnmapViewOfFile(m_pdata);
   if(m_hmap)  CloseHandle(m_hmap);
   if(m_hmutex)  CloseHandle(m_hmutex);
   m_pdata=NULL;
   m_hmap=NULL;
   m_hmutex=NULL;
 }
 operator T*() const throw()
 {
	return m_pdata;
 }

 
/*   
 RefT<T>* operator->() const throw()
 {
	 	 return  (RefT<T>*)m_pdata;
 }
*/
 T* operator->() const throw()
 {
	 return  m_pdata;
 }

};
//*/

//#define ILTYPE LONG_PTR

class _interlock
{
public:
 LONG_PTR volatile m_val;

_interlock():m_val(0){};

template <class B>
_interlock(B val):m_val((LONG_PTR)(val)){};


inline LONG_PTR read()
{
 //return (LONG_PTR) InterlockedExchangePointer((void**)&m_val,(void*)m_val);
	//return (LONG_PTR) InterlockedAdd((void**)&m_val,0);
	//return InterlockedBitTestAndReset(&m_val,1);
//LONG_PTR l= InterlockedCompareExchange(&m_val,m_val,m_val);
	LONG_PTR l= InterlockedExchangeAdd(&m_val,0);
   return l;

}

inline LONG_PTR write(LONG_PTR v)
{
	return (LONG_PTR) InterlockedExchangePointer((void**)&m_val,(void*)v);
}

//inline LONG_PTR* ptr(){ return &m_val;};

template <class A>
inline  operator A()
{
  
  //return A(m_val);
	return A(read());
}
template <class A>
inline _interlock& operator =(A a)
{   
  //
/*	register LONG_PTR tmp=(LONG_PTR)a; 
 //InterlockedExchange(&m_val,tmp);
  InterlockedExchangePointer((void**)&m_val,(void*)a);
*/
  write(LONG_PTR(a));
  return *this;
}

template <class A>
inline bool operator ==(A a)
{   
	return read()==LONG_PTR(a);
}

inline operator bool()
{
	return (bool)read();
}
};


typedef  struct _MODAL_HWINS
{ 
/*
   HWND hwinPP;
   HWND hwinModal;
   HWND hwinPrev;
*/   
	_interlock hwinPP;
	_interlock hwinModal;
	_interlock hwinPrev;
	 DWORD    findflags;
    _interlock    pid_dbg_delegate;  
     wchar_t  classname[256];  
	 wchar_t  title[256];
     wchar_t  moniker_in[1024];
	 wchar_t  moniker_out[1024];
} MODAL_HWINS,*PMODAL_HWINS;



typedef    process_data_map<MODAL_HWINS> process_modal_data,*pprocess_modal_data;




template<class T>
class cexe_heap
{
public:
	HANDLE m_h;
	cexe_heap()
	{
		m_h=HeapCreate(HEAP_CREATE_ENABLE_EXECUTE,0,0);
	}

	~cexe_heap(){HeapDestroy(m_h);};
	T* create()
	{

		return ::new (HeapAlloc(m_h,0,sizeof(T))) T;
	};

template <class F>   
  F* alloc(int cn=1)
   {
     return  HeapAlloc(m_h,HEAP_ZERO_MEMORY,sizeof(F)*cn); 
   }
  

	void destroy(T* t)
	{
		HeapFree(m_h,0,t);
	}

};

#pragma pack(push)
#pragma pack(1)
#define PUSH_CODE 0x68
#define RET_CODE  0xC3
#define RET_N_CODE  0xC2
#define PUSHEAX_CODE  0x50
#define POPEAX_CODE   0x58
#define MOV_EAX_CODE 0xB8
#define CALL_EAX_CODE     0xD0FF
#define CALL_PTR     0xE8
#define NOP_CODE   0x90
#define NOP_CODE_DW   0x90909090
#define NOP_CODE_W   0x9090
#define PUSH_PTR_ESP   0x2434FF90
#define JMPNEAR   0xEB
 struct byte6 { byte cm[6];};
 const byte6 EAXESP={0x8B,0x04,0x24,0xc7,0x04,0x24};


struct  _proc_stub_base_0
{
  byte cjmp;byte cnj;
  WORD cflag;
  DWORD crefcount;
  void* cproc;
  void* cpdatal;
  void* cpdatah;
  _proc_stub_base_0():cjmp(JMPNEAR),cflag(0),crefcount(0),cnj(0x12),cproc(NULL),cpdatal(NULL),cpdatah(NULL){};
} ;

struct  _proc_stub_base_1{};
#define _proc_stub_base _proc_stub_base_1
typedef _proc_stub_base* pproc_stub_base;

typedef struct  _proc_stub_cdecl :public  _proc_stub_base
{
	//byte cpopeax;
    //byte cpush1;void* pthis;            // PUSH this
	
    byte6 c_eaxesp_esp_pthis;  //   <<  RET ADRESS->EAX & 
    void* pthis;               //   pthis -> [ESP]  
    byte cpusheax;             //  push RET ADRESS(EAX)   
    byte ccall;LONG32 caddr_off;
    byte cretn;WORD cn;

//	byte cpush2;void* paddr;byte cret;  //  jmp  T:: method
	//=========================
	void* pallocator;
    //_proc_stub_base_0 pppp;   
	///*
	inline _proc_stub_cdecl():  
	    // cpopeax(POPEAX_CODE)
        //,cpush1(PUSH_CODE)
	     c_eaxesp_esp_pthis(EAXESP) 
		,pthis(NULL)
		,cpusheax(PUSHEAX_CODE)
		,ccall(CALL_PTR),caddr_off(NULL)
		,cretn(RET_CODE),cn(NOP_CODE_W)
		,pallocator(NULL)
	{
     }

	inline _proc_stub_cdecl* init(void *_proc,void *_this,void* _pallocator)
	{
		caddr_off=LONG32(_proc)-LONG32(&cretn);    
		pthis=_this;
		pallocator=_pallocator;
		return this;
	}
	//*/
} PROC_STUB_CDECL, *PPROC_STUB_CDECL;


typedef struct  _proc_stub :public  _proc_stub_base
{
	byte cpopeax;                       // POP EAX <-- RET ADDRESS
	byte cpush1;void* pthis;            // PUSH this
	byte cpusheax;                      // PUSH EAX -->RET ADDRESS
	byte cpush2;void* paddr;byte cret;  //  jmp  T:: method
 	//=========================
	void* pallocator;
	///*
	 _proc_stub():_proc_stub_base(), cpopeax(POPEAX_CODE)
		,cpush1(PUSH_CODE),pthis(NULL)
		,cpusheax(PUSHEAX_CODE)
		,cpush2(PUSH_CODE),paddr(NULL)
		,cret(RET_CODE)
		,pallocator(NULL)
	{}

	inline _proc_stub* init(void *_proc,void *_this,void* _pallocator=NULL)
	{
		paddr=_proc;    
		pthis=_this;
		pallocator=_pallocator;
		return this;
	}
	//*/
} PROC_STUB, *PPROC_STUB;

typedef struct  _proc_stub_struct :public  _proc_stub_base
{
	//_proc_stub_base_0 ppp;
	byte cpopeax;                       // POP EAX <-- RET ADDRESS
	DWORD cdoublefirst;                  // PUSH ret param so from  void (_stdcall *)(void* this, STRUCT*) RET 8;
	                                    //   first param &= : (char**)&this+sizeof(void*)*3
	byte cpush1;void* pthis;            // PUSH this
	byte cpusheax;                      // PUSH EAX -->RET ADDRESS
	byte cpush2;void* paddr;byte cret;  //  jmp  T:: method
	//=========================
	void* pallocator;
	///*
	 _proc_stub_struct(): _proc_stub_base(), cpopeax(POPEAX_CODE),cdoublefirst(PUSH_PTR_ESP)
		,cpush1(PUSH_CODE),pthis(NULL)
		,cpusheax(PUSHEAX_CODE)
		,cpush2(PUSH_CODE),paddr(NULL)
		,cret(RET_CODE)
		,pallocator(NULL)
	{}

	inline _proc_stub_struct* init(void *_proc,void *_this,void* _pallocator=NULL)
	{
		paddr=_proc;    
		pthis=_this;
		pallocator=_pallocator;
		return this;
	}
	inline static void free()
	{

	}

	inline void reset_struct_stack()
	{
		cdoublefirst=NOP_CODE_DW;
	}

inline void set_struct_stack()
{
  cdoublefirst=PUSH_PTR_ESP;
}
	//*/
} PROC_STUB_STRUCT, *PPROC_STUB_STRUCT;



#pragma pack(pop)


//typedef cexe_heap<PROC_STUB> proc_stub_allocator;
template <class PROC_STUB_CC>
class proc_stub_allocator_CC:public cexe_heap<PROC_STUB_CC>
{
public:
typedef PROC_STUB_CC* PPROC_STUB_CC;
class wild_type
{
  void* m_ptr;
  public:
 inline  wild_type():m_ptr(NULL){};
 inline  wild_type(void* p):m_ptr(p){};
template <class A>
inline operator A()
{
  return (A)(m_ptr);
}


};
	//inline  PPROC_STUB stub_create(void* pproc,void* pthis)

inline static void*& this_ptr(void* pstub)
{
  return  PPROC_STUB_CC(pstub)->pthis;
}

inline	wild_type stub_create(void* pproc,void* pthis)
	{
		return create()->init(pproc,pthis,this);
	};

	inline  static  void free(void*p)
	{
		if(p) ((proc_stub_allocator_CC*)(PPROC_STUB_CC(p)->pallocator))->destroy(PPROC_STUB_CC(p));
	};
};

typedef  proc_stub_allocator_CC<PROC_STUB> proc_stub_allocator;
typedef  proc_stub_allocator_CC<PROC_STUB_CDECL> proc_stub_allocator_cdecl;
typedef  proc_stub_allocator_CC<PROC_STUB_STRUCT> proc_stub_allocator_STRUCT;

// 2)
#define METHOD_PTR(cl,mm) __method_ptr<cl> ((__method_ptr<cl>::TMT)&cl::mm)

// 1)
//#define METHOD_PTR(cl,mt) \
//	__method_ptr<cl>((__method_ptr<cl>::TMT)&(cl::mt))
	

template <class T>
class __method_ptr
{
	
public:

	typedef  void  ( __stdcall   T::*TMT)();
	typedef  void  (_cdecl   *PPVARG)(...);
	__method_ptr  (const TMT& tt):tp(tt){};
public:
	union
	{
		TMT   tp;
		void* ptr;
		DWORD_PTR dwptr;
		FARPROC proc;
	};
    
	template<class FP >
	inline 	operator FP()
	{ 
		return FP(dwptr);
	}
} ;





template <class T>
class CWndProcT
{
public:
	HWND m_hwin;
	WNDPROC old_proc;
	proc_stub_allocator *ppsa,*pinternal_psa;
	void* pstub; 

	static	LRESULT __stdcall s_wndproc(T* thisT,HWND hwin,UINT umsg, WPARAM wparam , LPARAM lparam)
	{  	  
		return thisT->wndproc(hwin,umsg,wparam,lparam);
	};

inline LRESULT wndproc(HWND hwin,UINT umsg, WPARAM wparam , LPARAM lparam)
{
  if(old_proc) return  old_proc(hwin,umsg,wparam,lparam);
  else return 0;
}

inline void*  proc_ptr()
       {
		   T* thisT=static_cast<T*>(this);
           if(!pstub) pstub=ppsa->stub_create((void*)s_wndproc,(void*)thisT);
		   return pstub;
       }

	inline  void init(HWND hwin)
	{
		reset();
        m_hwin=hwin;  
		
		old_proc=(WNDPROC)SetWindowLong(m_hwin,GWL_WNDPROC,(LONG)proc_ptr());    
	};

	inline void reset()
	{
		if((old_proc)&&(m_hwin)) SetWindowLong(m_hwin,GWL_WNDPROC,LONG(old_proc));
		old_proc=NULL;	  
		ppsa->free(pstub); 
		pstub=NULL;
		
		
	};
	CWndProcT(proc_stub_allocator* ps=NULL):m_hwin(NULL),pinternal_psa(NULL),pstub(NULL),ppsa(NULL)
	{
		old_proc=NULL;
		if(ps) ppsa=ps;
		else     ppsa=pinternal_psa=new proc_stub_allocator;
	};

	~CWndProcT()
	{
		reset();
		if(pinternal_psa) 
			delete pinternal_psa;
	};
};

