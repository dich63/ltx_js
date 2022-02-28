#pragma once




template <class H ,typename P ,class T=H>
struct _auto_close_P
{
	H m_h;
	P m_proc;
	explicit _auto_close_P(H h=NULL,P proc=NULL):m_h(h),m_proc(proc){};

	inline void close()
	{
		if((m_h)&&(m_proc))
		m_proc(m_h);
		m_h=NULL;
	}

	~_auto_close_P()
	{
		close();
	}
	
inline _auto_close_P&  operator =( _auto_close_P& c)
	{
        P proc=c.m_proc;
		attach(c.detach());
 		m_proc=proc;
		return *this;
	}
	inline operator T()
	{
		return T(m_h);
	}
	inline H& ref()
	{
		return m_h;
	}
inline  T attach(H h)
{
	close();
    m_h=h;
	return T(h);
}

inline  T detach()
{
	//H tmp=m_h;
	m_proc=NULL;
	return T(m_h);
}



};

template <typename T,int N=1>
struct dummy_ptr
{
     T buf[N];
inline operator T* ()
{
	return buf;
}
};

//CHandle

template<typename H,typename RetType=void>
struct noop_type {
	inline 	noop_type(){};
	inline 	noop_type(H h){}; 
};


template<typename H,typename RetType=BOOL>
struct stdcall_type {	typedef RetType (__stdcall  *close_H_proc)(H );};

template<typename H,typename RetType=void>
struct cdecl_type {	typedef RetType (__cdecl  *close_H_proc)(H );};

template <class H,typename RetType, RetType (__stdcall  *close_H_proc)(H ),class T=H>
struct _auto_stdcall :_auto_close_P<H,typename stdcall_type<H,RetType>::close_H_proc,T> 
{
 explicit _auto_stdcall(H h=NULL):_auto_close_P(h,close_H_proc){};
 inline _auto_stdcall&  operator =(_auto_stdcall& c)
 {
	 _auto_close_P(*this)=(_auto_close_P(c));
	 return *this;
 }
 
        
/*
  _auto_stdcall( _auto_stdcall& s):_auto_close_P(s)
		 {
            
		 };
*/
};

template <class H,typename RetType,RetType (__cdecl  *close_H_proc)(H ),class T=H>
struct _auto_cdecl :_auto_close_P<H,typename cdecl_type<H>::close_H_proc,T> 
{
	explicit _auto_cdecl(H h=NULL):_auto_close_P(h,close_H_proc)
	{      
	};
};

