#pragma once

#include <windows.h>
#include <stdio.h>
#include <memory>


#pragma pack(push)
#pragma pack(1)
/*
#define PUSH_CODE 0x68
#define RET_CODE  0xC3
#define PUSHEAX_CODE  0x50

#define RET_N_CODE  0xC2

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
*/


struct  closure_stub_base
{
	enum
	{
		PUSH_CODE=0x68,
		PUSHEAX_CODE= 0x50,
		POPEAX_CODE = 0x58,
		RET_CODE= 0xC3,
		CALL_EAX_CODE =  0xD0FF,
		JMP_EAX_CODE =  0xE0FF
	};

   typedef void (*proc_t)(void* );

	template <typename N> 
	inline static void* addressof(N n)
	{
		union
		{
			N n;
		void* p;
		} un;
		un.n=n;
		return un.p;
	};

	template <typename N>
static	N& wild_cast(void** pp)
	{
	
        return *((N*)pp);  
	}

	
inline  static long call(void *proc,void* param0=0,void* param1=0,void* param2=0,void* param3=0)
{
			DWORD ustack,ret;

			__asm {
				mov ustack,ESP;
				push param3;
				push param2;
				push param1;
				mov  ECX,param0;
				mov  EAX,ECX;
				push ECX;
				call proc;
				mov ESP,ustack;
			}; 
}

inline  static long safe_call_res(long* pres,void *proc,void* param0=0,void* param1=0,void* param2=0,void* param3=0)
{
	    long res;
		__try
		{



			DWORD ustack;

			__asm {
				mov ustack,ESP;
				push param3;
				push param2;
				push param1;
				mov  ECX,param0;
				mov  EAX,ECX;
				push ECX;
				call proc;
				mov ESP,ustack;
				mov res,EAX;
			}; 

            if(pres) *pres=res;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			return 1;
		}
        return 0;
}

inline  static long safe_call(void *proc,void* param0=0,void* param1=0,void* param2=0,void* param3=0)
	{
		if(proc)
		{
			__try
			{



				DWORD ustack;

				__asm {
					mov ustack,ESP;
					push param3;
					push param2;
					push param1;
					mov  ECX,param0;
                    mov  EAX,ECX;
					push ECX;
					call proc;
					mov ESP,ustack;
				}; 


			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				return 0;
			}

		}
	}
   closure_stub_base(){};
 private:
	 closure_stub_base(closure_stub_base&){};
	 closure_stub_base(const closure_stub_base&){};
};



struct  closure_stub:closure_stub_base 
{

	byte cpopeax;                       // POP EAX <-- RET ADDRESS
	byte cpush1;void* pthis;            // PUSH this
	byte cpusheax;                      // PUSH EAX -->RET ADDRESS
	byte cpush2;void* paddr;byte cret;  //  jmp  T:: method
 	//=========================
	void *pexit,*pthis2;
	
	
	 closure_stub(): cpopeax(POPEAX_CODE)
		,cpush1(PUSH_CODE),pthis(NULL)
		,cpusheax(PUSHEAX_CODE)
		,cpush2(PUSH_CODE),paddr(NULL)
		,cret(RET_CODE),pexit(0),pthis2(0)
	{}

 
	 typedef int (__stdcall* proc1_t)(void*);
	 typedef int (__stdcall* proc2_t)(void*,void*);
	 typedef int (__stdcall* proc3_t)(void*,void*,void*);
	 typedef int (__stdcall* proc4_t)(void*,void*,void*,void*);

	 template <typename N,typename E>
     closure_stub (N proc,void * context,E exit_proc,void *context2)
	 {
        ::new(this) closure_stub ();
		this->init(addressof(proc),context,addressof(exit_proc),context2);
	 }

	 template <typename N,typename E>
	 closure_stub (void * context,N proc,E exit_proc)
	 {
		 ::new(this) closure_stub ();
		 this->init(addressof(proc),context,addressof(exit_proc),context);
	 }

	 template <typename N>
	 closure_stub (N proc,void * context)
	 {
		 ::new(this) closure_stub ();
		 this->init(addressof(proc),context);
	 }

   ~closure_stub()
   {
       safe_call(pexit,pthis2);
   }

    void _stdcall __fake(){};

	template<typename N> N& this_ref()
	{
          return wild_cast<N>(&pthis);
	}
	

	inline closure_stub* init(void *_proc,void *_this=0,void *_exit_proc=0,void *_this2=0)
	{
		paddr=(_proc)?_proc:addressof(&closure_stub::__fake);    
		pthis=_this;
		pexit=_exit_proc;
		pthis2=_this2;
		return this;
	}
	//*/

	inline operator PROC()
	{
		return PROC(this);
	}
	inline operator proc1_t()
	{
		return proc1_t(this);
	}
	inline operator proc2_t()
	{
		return proc2_t(this);
	}
	inline operator proc3_t()
	{
		return proc3_t(this);
	}
	inline operator proc4_t()
	{
		return proc4_t(this);
	}

} ;





struct closure_signaler:closure_stub
{

	HANDLE m_h;

	inline HANDLE handle()
	{
		return handle_ref();
	}
	inline HANDLE& handle_ref()
	{
		return m_h;
	}
	 bool check()
	 {
		 DWORD dw,f;
          return f=GetHandleInformation(m_h,&dw); 
	 }  
	~closure_signaler()
	{
		if(m_h) CloseHandle(m_h);
	};

	inline bool assign_handle(HANDLE hev)
	{
		HANDLE hp=GetCurrentProcess();
		return DuplicateHandle(hp,hev,hp,&m_h,DUPLICATE_SAME_ACCESS,0,DUPLICATE_SAME_ACCESS);
	}
	closure_signaler(HANDLE hevent):m_h(INVALID_HANDLE_VALUE)
	{
		if((hevent)&&(hevent!=INVALID_HANDLE_VALUE))  
		bool f=assign_handle(hevent);

	}
};

struct event_signaler:closure_signaler
{
	typedef event_signaler base_t;
    typedef void (*proc_t)();


	inline operator proc_t ()
	{
		return proc_t(this);
	}
    
	inline  static void __stdcall  set(event_signaler* p)
	{
		SetEvent(p->handle());
	}

	inline unsigned int wait(int itimeout=INFINITE)
	{
       return WaitForSingleObject(handle(),itimeout);
	}

/*
	inline  static void  __stdcall  exit(event_signaler* p)
	{
		delete p;
	}
*/

	event_signaler(HANDLE hevent): closure_signaler(hevent)
	{
		init(&set,this);
	}
	

};


struct  closure_stub2:closure_stub
{
	void* proc;
	void* pcontext;

	inline  static long __stdcall  safe_stub(closure_stub2* _this,void* arg0)
	{
		return call(_this->proc,_this->pcontext,&arg0,&arg0);
	}


	closure_stub2():closure_stub(){};

	template <typename N,typename E>
	closure_stub2 (void * context,N nproc,E exit_proc):closure_stub()
	{
		proc=addressof(nproc);
        pexit=addressof(exit_proc);
		pcontext=context;
		init(&safe_stub,this);
	
	}

	template <typename N>
	closure_stub2 (N nproc,void * context):closure_stub()
	{
		proc=addressof(nproc);
		pcontext=context;
		init(&safe_stub,this);
	}

};


//#include "shared_allocator.h"

