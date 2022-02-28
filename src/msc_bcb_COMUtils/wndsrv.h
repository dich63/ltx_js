#pragma once

#include <windows.h>
#include <ipc.h>
#include <tls_dllex.h>
#include <dispcall/flipflop.h>
#include <set>
struct SRV_MSG_BASE
{
	int sizeb;
	int cmd_id;
	int nerr;
	
};


struct SRV_INIT:SRV_MSG_BASE
{   
};




template <class T>
struct message_srv_base
{

   HWND hwnd;
   UINT WM_SYNC_MSG;


inline T* thisT()
{
  return static_cast<T*>(this);
}
    //proc_stub_allocator m_psa;

typedef  int (*sync_proc_type)(T*,void*);

static  int syncro_method(T* t,WPARAM wParam,LPARAM lParam)
{
	return(t)? sync_proc_type(wParam)(t,(void*)lParam):0;
}

void send_syncro(void* proc,void* param)
{
  SendMessage(hwnd,WM_SYNC_MSG,WPARAM(proc),LPARAM(param)); 
}

	static LRESULT _stdcall s_wndproc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{

		//if(msg==WM_SETTEXT) return 0;
        T* _this;
        _this=(T*) GetWindowLong(hWnd,GWL_USERDATA);
         if(msg==WM_SYNC_MSG)
		 {
           return syncro_method(_this,wParam,lParam);
         }

		LRESULT lr;
		if((_this)&&(lr=_this->wndproc(hWnd,msg,wParam,lParam)))
			 return lr;
   		 else  return DefWindowProc(hWnd,msg,wParam,lParam);
   };

	inline HWND wnd_alloc(char* winclass,char* winname=NULL,HWND hwinParent=HWND_MESSAGE)
	{
		hwnd=NULL;
		;
		WNDCLASSA wc;
		HINSTANCE HInstance=GetModuleHandleA(NULL);
		if(!GetClassInfoA(HInstance,winclass,&wc) )
		{

			memset(&wc,0,sizeof(wc));
			wc.hInstance = HInstance;
			wc.lpszClassName=winclass;
			wc.lpfnWndProc=&s_wndproc;
			if ( ! RegisterClassA( &wc ) ) return NULL;
             
			
		};
		 hwnd=CreateWindowA(winclass,winname,0,0,0,0,0,hwinParent,NULL,HInstance,0);
		if(hwnd)
		{

		//T* _this=static_cast<T*>(this);               
		 if(!SetWindowLong(hwnd,GWL_USERDATA,(LONG)thisT()))
			return DestroyWindow(hwnd),NULL;
		}
		return hwnd;
	}

	message_srv_base():hwnd(NULL)
	{
      WM_SYNC_MSG=RegisterWindowMessageA("WM_SYNC_MSG_000");   
	};
     ~message_srv_base()
	 {
       if(hwnd) DestroyWindow(hwnd);
	 }
};

template <class T>
struct auto_locker
{
	T& m_t;
	auto_locker(T& t):m_t(t)
	{
		m_t->lock();
	}

	~auto_locker(){m_t->unlock();}
};

template <class T>
struct process_base
{
	HANDLE pid;
	HANDLE hprocess;
	bool fsuccess;
	T* m_pt;
	process_base():hprocess(0),m_pt(0){};
	process_base(T* pt,int pid)
	{
		HANDLE hwo;
		m_pt=pt;
        hprocess=OpenProcess(PROCESS_ALL_ACCESS,0,pid);     
		m_pt->register_process(this);
	   fsuccess=::RegisterWaitForSingleObject(&hwo,hprocess,WAITORTIMERCALLBACK(&s_terminate),this,INFINITE,WT_EXECUTEONLYONCE);  
	}
	virtual ~process_base()
	{
		if(hprocess) CloseHandle(hprocess);
	};

	static void __stdcall s_terminate(process_base* p,BOOLEAN )
	{
		//auto_locker<T> cc(p->m_pt);
		if(p->m_pt)
			p->m_pt->unregister_process(p);

   };

	
};

//std::less
template<class _Ty>
struct less_pid
	: public std::binary_function<_Ty, _Ty, bool>
{	// functor for operator<
	bool operator()(const _Ty& _Left, const _Ty& _Right) const
	{	// apply operator< to operands
		return (_Left->pid < _Right->pid);
	}
};


template <class T>
struct message_srv_process_base:message_srv_base<T>
{

typedef flipflop_ptr<process_base<T>> process_ptr;

std::set<process_ptr,less_pid<process_ptr> > m_process_list;

 inline   void lock(){};
 inline   void unlock(){};
inline void register_process(process_base<T>* p)
{
  auto_locker<T> cc(thisT());
  m_process_list.insert(p);
}

static long s_unregister_process(T* t,process_base<T>* p)
{
    t->unregister_process(p);
	return 1;
}

inline void unregister_process(process_base<T>* p)
{
  auto_locker<T> cc(thisT());
  m_process_list.erase(p);
  if(m_process_list.empty())
  {
	  DWORD tid,pid;
      tid=GetWindowThreadProcessId(hwnd,&pid);
      PostThreadMessage(tid,WM_QUIT,0,0);
  };
}


};


