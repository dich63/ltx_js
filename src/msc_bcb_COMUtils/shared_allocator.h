#pragma once
#include <windows.h>
#include <comdef.h>
#include "heap_allocator.h"


inline bool region_read_check(void* pb,void* pe)
{
	MEMORY_BASIC_INFORMATION mi;
  if(VirtualQuery(pb,&mi,sizeof(mi))&&(mi.State==MEM_COMMIT))
  {
	  DWORD fp=(PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_READ|PAGE_READWRITE|PAGE_READONLY);
	  fp&=mi.Protect;
	  if(!fp) return false;
	  DWORD lp=DWORD(pe)-DWORD(mi.BaseAddress);
	  return lp<=mi.RegionSize; 
  }
  else return false;
}


struct i_handle:IUnknown
{

	enum
	{
		_SIGN_=0xBABAEBCA,
		add_cb=4+2+2,
		add_cbv=3+2+2+2
	};
	struct info
	{
   	  union
	    {
		struct{ const char* vv[add_cbv];};
		struct{ 
				size_t cb;
				const char* cv[add_cb];
		     };
		};
	};
	inline	static    long acquire(void* p)
	{ 
		i_handle *h=i_get(p);
		return (h)? h->AddRef():0;
	}
	inline	static    long release(void* p)
	{
		i_handle *h=i_get(p);
		return (h)? h->Release():0;
		
	}

	inline  static info* get_info(void* p)
	{
    	 static const IID riid={0xFFFFFFFF,0xFFFF,0xFFFF,{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}};
       info* pi=0;
	   i_handle *h=i_get(p);
     	if((h)&&SUCCEEDED(h->QueryInterface(riid,(void**)&pi))) 
			return pi;
		else return 0;	     
	}
	
	inline	static    size_t size(void* p)
	{
          info* pi=get_info(p);
		  if(pi) return size_t(pi->vv[0]);
		  else return 0;
	}

    inline	static    void*& event_ref(void* p)
	{
		  void ** pp=  (void **)(get_info(p)->cv+4);
		return *pp;
	}

	inline	static    void*& completion_port_ref(void* p)
	{
		void ** pp=  (void **)(get_info(p)->cv+5);
		return *pp;
	}
	inline	static    size_t& size_ref(void* p)
	{
		info* pi=get_info(p);
		 return *((size_t*)(pi->vv));
	}

	inline	static  const  char* id(void* p,int n=0)
	{
		const int nmax=sizeof(((info*)0)->cv)/sizeof(void*);
		if((0<=n)&&(n<nmax)){

		info* pi=get_info(p);
		if(pi) return pi->cv[n];

		}

		return 0;
	}


	template <class T>
	inline T* ptr()
	{

        //void* p=const_cast<void*>(this);
		void* p=(void*)(this);
		return (p)?(T*)(((void**)p)+1):(T*)0;
	}

	template <class N>
	bool set_destroy_event(N n,void * p=0)
	{
      if(this)
	  {
		union{
            N n;
			char *p;
		} pn;
		pn.n=n;
		info* pi=get_info(ptr<char>());
		pi->cv[2]=pn.p;
        pi->cv[3]=(char*)p;
		return true;
	  }
        return false;
	}
    inline	static   void*  base_ptr(void* p)
	{
		__try
		{
            char* pt=(char*)p;
				pt-=sizeof(void*)+2*sizeof(long);
            return ((char*)p)-(*((long*)pt));  
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{};
       return 0;
	}

	inline	static    i_handle* i_get(void* p)
	{

		return (verify_ptr(p))?(i_handle*)(((void**)p)-1):(i_handle*)0;
	}


static	inline void* verify_ptr(void* pv,bool frc=false)
	{

		if((pv)&&(ULONG_PTR(pv)>0x10000)) 
			//try {
	__try
		{
				bool f=false;  
				volatile char n=0;    
				char *p=(char *)pv;
				p-=sizeof(void*)+2*sizeof(long);

				long *ps= (long*)p;
				long off=ps[0];
				long sign=ps[1];
				if(sign!=_SIGN_) return 0;
				if(off<=0) return 0;
				//char* pt=p-off;
				char* pt=((char*)pv)-off;

				long fc=heap_mem_global_set::get_ptr_set().check(pt);

				return (fc)?pv:NULL;

/*				

				if(0&&frc)
				{
					fc=InterlockedExchangeAdd((long volatile *)pt,0);
					if(fc<=0) return 0;
				}

				while(pt!=pv) f|=*(pt++);
				return f? pv:0;
				*/

		}
		__except(EXCEPTION_EXECUTE_HANDLER)	{

		};
		
		//catch (...){}

		return 0;
	}


};



template <class T>
struct ptr_holder
{
	union
	{
		struct{ T* p;};
		struct{ void* pv;};
	};
     ptr_holder():p(0){};

inline	 T* operator=(const ptr_holder<T>& h)
	 {
     
	   i_handle::acquire(h.p);
	   i_handle::release(p);
	   pv=i_handle::verify_ptr(h.pv);
	   return p;
	 }
	ptr_holder(T* pt,bool faddref=true)
	{
		pv=i_handle::verify_ptr(pt);

		if(faddref) i_handle::acquire(pv);
	};


inline T& ref()
 {
	  return *p;
 }
	 ptr_holder(const ptr_holder<T>& h)
	 {
		 	i_handle::acquire(pv=i_handle::verify_ptr(h.pv));
		 
	 };
	 ~ptr_holder()
	 {
		 i_handle::release(pv);
	 }

	 void release()
	 {
         if(pv) i_handle::release(pv);
		 pv=0;
	 };

inline	 T*  attach(void* pn,bool faddref=false)
	 {
		 if(pn!=pv)
		 {
			 i_handle::release(pv);
             pv=i_handle::verify_ptr(pn);
			 if(faddref) i_handle::acquire(pv);
		 }
		 return p;
	 }

inline	 T*  detach()
	 {
		 T* t=p;
		 pv=0;
		 return t;
	 }
	 inline 	T* operator->()
	 {
		 return p;
	 }
	 inline 	 operator T*()
	 {
		 return p;
	 }

	 inline T** address()
	 {
		 i_handle::release(pv);
		 return &p;
	 }

};


struct __declspec(uuid("DB9ADB1C-253C-4ae4-B801-F8D347C86D1F")) i_handle;


struct  i_lib_ptr:i_handle
{};

struct __declspec(uuid("DB9ADB1C-253C-4ae4-B801-F8D347C86D1F")) i_lib_ptr;



struct __declspec(uuid("00000000-0000-0000-0000-000000000000")) i_data_ptr{};
struct __declspec(uuid("FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF")) i_header_ptr{};




#include <memory>
#include <typeinfo>
#include "closures.h"

#pragma pack(push)
#pragma pack(1)

struct  i_shared_ptr:i_lib_ptr
{
	virtual size_t STDMETHODCALLTYPE get_size()=0;
	virtual const char* STDMETHODCALLTYPE get_id(int n=0)=0;
    virtual void* STDMETHODCALLTYPE get_data_ptr()=0;	
	virtual void* STDMETHODCALLTYPE get_base_ptr()=0;
	virtual LONG* STDMETHODCALLTYPE get_ref()=0;
	virtual void STDMETHODCALLTYPE destroy()=0;
	

/*
	inline	static    i_shared_ptr* get_instance(void* p)
	{
		void** pc=(void**)p;

		return (i_shared_ptr*)(pc-1);
	}
	inline	static    long acquire(void* p)
	{
		 return get_instance(p)->AddRef();
	}
	inline	static    long release(void* p)
	{
		return get_instance(p)->Release();
	}
*/
};

 template <typename T>  char* _base_typeid(T* p,typename T::base_t*pp ){	return (char*)typeid( T::base_t* ).name();}
 template <typename T>  char* _base_typeid(T* p,... ){	return (char*) typeid( T* ).name();}
 template <typename T> char* base_typeid(T* p) {  return  _base_typeid(p,0);}

 template<class T>
 inline  bool is_handle_type(void* p,bool fstrong=false,bool fref=false)
 {
	 if(!i_handle::verify_ptr(p,fref)) return false;
	 if(fstrong) return i_handle::id(p,1)==typeid(T).name();
		 else return i_handle::id(p)==base_typeid((T*)0);
 };

 template<class T>
 inline  T* i_handle_cast(void* p,bool fstrong=false,bool fref=false)
 {
	 return (is_handle_type<T>(p,fstrong,fref)) ? (T*)p: (T*)0;
 };

template <long initial_ref_count=1>
struct header_rc_size_type
{
	enum
	{
		tn_count=i_handle::add_cb+2
	};
	void* ref_count_;
	void* sizeb;
	const char *tn[tn_count];
	
	void weak_release(void* p=0)
	{
          closure_stub::safe_call((void*)tn[2],(void*)tn[3],p);
	}
	
	header_rc_size_type(): ref_count_((void*)initial_ref_count)	{};
	~header_rc_size_type()
	{
		
		//i_handle::release((void*)tn[4]);  
		//i_handle::release((void*)tn[6]);
		//i_handle::release((void*)tn[7]);
	}
};



template <long initial_ref_count=1>
struct ref_count_NOT_COM_support:header_rc_size_type<initial_ref_count>
{ 
	//void* ref_count_;
	//ref_count_NOT_COM_support(){ ref_count_=(void*)initial_ref_count;}

	inline HRESULT operator()(IUnknown * punkOuter,REFIID riid,void** ppunk)
	{
		return E_NOINTERFACE;
	}
	~ref_count_NOT_COM_support()
	{}

};


inline long ref_count(IUnknown * p)
{
   if(!p) return -777;
   p->AddRef();
   return p->Release();
};


struct  FTM
{
	IUnknown * pmarshal;
	//HRESULT hrCoI;




	FTM():pmarshal(0)//,hrCoI(E_FAIL)
	{

	}
	
	
	
	~FTM()
	{
       //long ll=ref_count(pmarshal);
		if(pmarshal) pmarshal->Release();

		
		//if(SUCCEEDED(hrCoI)) CoUninitialize();
	}
	inline HRESULT operator()(IUnknown * punkOuter,REFIID riid,void** ppunk)
	{
		HRESULT hr;
		//if(!ppunk)  return E_POINTER;
		if(riid!=__uuidof(IMarshal)) return E_NOINTERFACE;
		IUnknown * p=init_once(punkOuter);
		//init_once(punkOuter);		 return E_NOINTERFACE;
		if(!p)  return E_NOINTERFACE;
		hr=p->QueryInterface(riid,ppunk);
		return hr;
	}
	inline IUnknown * init_once(IUnknown * punk)
	{
		HRESULT hr;
		//hrCoI=CoInitializeEx(NULL,0); 
		IUnknown *p=(IUnknown *)InterlockedCompareExchangePointer((volatile PVOID*)&pmarshal,0,0);
		if(p) return p;
		hr=::CoCreateFreeThreadedMarshaler(punk,&p);
		if(FAILED(hr)) return NULL;
		IUnknown *pt=(IUnknown *)InterlockedCompareExchangePointer((volatile PVOID*)&pmarshal,p,0);
		if(pt)
		{
			p->Release();
			return pt;
		}
		return p;
	};

};

template <class N>
struct verifier_header
{
    typedef typename N inner_t;
	N n;
	long sign[2];
	verifier_header()
	{
		sign[0]=sizeof(N);
		sign[1]=i_handle::_SIGN_;
	}
//~verifier_header(){	InterlockedIncrement(sign+1);}

};


//template <long initial_ref_count=1,const IID* piid=&__uuidof(i_lib_ptr)>
template <long initial_ref_count=1,const IID& iid_data=__uuidof(i_data_ptr),const IID& iid_header=__uuidof(i_header_ptr)>
struct ref_count_COM_support:header_rc_size_type<initial_ref_count>
{ 
	//void* ref_count_;
	FTM marshaler;
	//ref_count_COM_support(){		ref_count_=(void*)initial_ref_count;	}

/*
inline bool QI(REFIID riid1,REFIID riid2,IUnknown *punk,void *p ,void** pp)
{
   if(riid1!=riid2) return false;

   return true;
}
*/

	inline HRESULT operator()( i_shared_ptr* punkOuter,REFIID riid,void** pp)
	{
		HRESULT hr;
		if(!pp) return E_POINTER;
		if((riid==__uuidof(IUnknown))||(riid==__uuidof(i_lib_ptr)))
		{
            *pp=punkOuter;
				punkOuter->AddRef();
			 return S_OK; 
		};

		if(riid==iid_data)
		{
			*pp=punkOuter->get_data_ptr();
			//punkOuter->AddRef();
			return S_OK; 
		};
		if(riid==iid_header)
		{
			
           *pp= &sizeb;
		   return S_OK; 
		}
		
    //    FTM marshaler;

		//return E_NOINTERFACE;
		hr=marshaler(punkOuter,riid,pp);
		return hr;
	}
	~ref_count_COM_support()
	{

	}
};



template <class Header>//=ref_count_NOT_COM_support<1> >
struct  shared_base:i_shared_ptr
{
	typedef typename Header refcount_QI_t;
	typedef typename  refcount_QI_t* header_pointer;
	typedef typename  verifier_header<refcount_QI_t> verifier_header_t;

	enum
	{
		//head_size=sizeof(refcount_QI_t)
		  head_size=sizeof(verifier_header_t)
	};

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void **ppvObject)
	{
		REFIID rriid=(&riid)?riid:IID_NULL;
		//const void* p=&riid;
		//if(!(&riid)) return E_POINTER;
        return (*get_header())(this,rriid,ppvObject);
		
	};

	inline LONG inner_release()
	{
		return _InterlockedDecrement(get_ref()); 
	}

	inline LONG inner_addref()
	{
		return _InterlockedIncrement(get_ref()); 
	}
	virtual ULONG STDMETHODCALLTYPE AddRef()
	{ return  inner_addref(); };


	
	virtual ULONG STDMETHODCALLTYPE Release()
	{
		LONG l;
		l= inner_release();

		if(!l)
		{

			destroy();
		}
		return l;
	};

	

	virtual void* STDMETHODCALLTYPE get_base_ptr()
	{
		//return  (char*)(((void**)this)-1);
        return  ((char*)this)-head_size;
	};

inline header_pointer get_header()
{
  return header_pointer(get_base_ptr());
}

	virtual LONG* STDMETHODCALLTYPE get_ref()
	{
		return (LONG*)get_base_ptr();
	};


};



template <class T,class A=std::allocator<T>,class Header=ref_count_COM_support<1> > 
struct shared_allocator:shared_base<Header>
{
  
 typedef typename shared_allocator<T,A,Header> self_t;

 //typedef typename A::rebind<char>::other  allocator;

 
 shared_allocator()
 {
	 A aa;
 }

 virtual size_t STDMETHODCALLTYPE get_size()
 {
         return  size_t(get_header()->sizeb);
 };
 virtual const char* STDMETHODCALLTYPE get_id(int n=0)
 {
	 if((0<=n)&&(n<2))
	 {
		 return  get_header()->tn[n];
	 }
	 return NULL;
 };


static    ptr_holder<T> allocate_holder(int count=1)
{
	 ptr_holder<T> h;
     h.attach(new(allocate(count)) T() );

	 /*
 	 T * p=h.attach(allocate(count) );
	 T * pend=p+count;
	 for(;p!=pend;p++) 	 p->T();*/
	 return h;
}


static    void* allocate(int count=1)
	{
      //struct dummy:shared_allocator<T,A> { char c[2];};

		struct dummy:self_t { char c;};
		struct dummy2{
			
			verifier_header_t p;
				dummy  m;
			};


     const int off0=offsetof(dummy2,m);
     const int off1=offsetof(dummy,c);
	 
	 const int off=off0+off1;

	 size_t sizb=sizeof(T)*count;

	 //char* p=allocator().allocate(sizb +off);
	 char* p = A::rebind<char>::other().allocate(sizb + off);
	 
    	 
     dummy2* pt= new(p)  dummy2();

	 pt->p.n.sizeb=(void*)sizb;
	 memset(pt->p.n.tn,0,verifier_header_t::inner_t::tn_count*sizeof(void*));
	 
     
     pt->p.n.tn[0]=base_typeid((T*)0);
	 pt->p.n.tn[1]=typeid(T).name();
/*
	 pt->p.n.tn[2]=0;
	 pt->p.n.tn[3]=0;
	 pt->p.n.tn[4]=0;
	 pt->p.n.tn[5]=0;
	 pt->p.n.tn[6]=0;
	 pt->p.n.tn[7]=0;
 */

	 pt->p.sign[0]=off;

	  return p+off;
	}

	virtual void STDMETHODCALLTYPE destroy()
	{
        struct dummy:self_t { char c;};
     	int off=offsetof(dummy,c);
        void *pt=const_cast<self_t*>(this);

		char *p=((char*)pt)+off;
		T* ptrT=(T*)(p);

		//~verifier_header(){	InterlockedIncrement(sign+1);}

		LONG l=inner_addref();
        get_header()->weak_release(ptrT);
		if(l=inner_release()) return ; 
          
         InterlockedIncrement(((long*)pt)-1);// verifier_header FAILED!!!

        get_header()->~refcount_QI_t();

        
        //ptrT->~T();
		
			A().destroy(ptrT);
		

        
        

		//A::rebind<self_t>::other().deallocate(get_base_ptr(),0);
		A::rebind<char>::other().deallocate((char*)get_base_ptr(),0);
        
	};
	virtual void* STDMETHODCALLTYPE get_data_ptr()
	{
		struct dummy:shared_base { char c;};  
		int off=offsetof(dummy,c);
		return  ((char*)this)+off;
	};
	inline T* data_ptr(){ return (T*) get_data_ptr();}

};




#pragma pack(pop)
/*
template <class A>
void * operator new(size_t count, void* shared_allocator<char,A> )
{
	const int offs=sizeof(void*)*2;
	char* p= shared_allocator<char,A>::allocator().allocate(count+offs);
	return p+offs;
}
*/