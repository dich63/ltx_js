#pragma once
// istream_pipe.h
#include "dummy_dispatch_aggregator.h"



#define HGSTREAM ((ISequentialStream*)777)

template <int VV=1>
struct IStreamAggregator_Impl:base_aggregator_t<IStreamAggregator_Impl<VV>,IStream>{


	IStreamAggregator_Impl(ISequentialStream* inner_stream):sstream(inner_stream){		
		sstream.QueryInterface(&stream.p);		
	}


	inline bool operator()(IUnknown* punkOuter,REFIID riid,void** ppObj,HRESULT& hr)
	{
		return inner_QI(punkOuter,riid, ppObj,hr)||inner_QI(punkOuter,riid,__uuidof(ISequentialStream), ppObj,hr);	
	}


	virtual HRESULT STDMETHODCALLTYPE Read(void *pv,ULONG cb, ULONG *pcbRead){		
		return (sstream)?sstream->Read(pv,cb,pcbRead):E_NOTIMPL;	
	};

	virtual  HRESULT STDMETHODCALLTYPE Write( const void *pv,ULONG cb,ULONG *pcbWritten) 	{
              return (sstream)?sstream->Write(pv,cb,pcbWritten):E_NOTIMPL;	
	};

// IStream
	
	virtual HRESULT STDMETHODCALLTYPE Revert(){		
		return (stream)?stream->Revert():E_NOTIMPL;
	}                            

	virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER l){
		return (stream)?stream->SetSize(l):E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream* s, ULARGE_INTEGER l, ULARGE_INTEGER* b ,ULARGE_INTEGER* e){
			return (stream)?stream->CopyTo(s,l,b,e):E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD cb)	{ 	
		return (stream)?stream->Commit(cb):E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER b, ULARGE_INTEGER e, DWORD m)  {
		return (stream)?stream-> LockRegion(b,e,m):E_NOTIMPL;
	}	
	virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER b, ULARGE_INTEGER e, DWORD m)    {return E_NOTIMPL;}      
	virtual HRESULT STDMETHODCALLTYPE Clone(IStream ** pps)  {
		return (stream)?stream->Clone(pps):E_NOTIMPL;
	}                                	
	virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER e, DWORD m,ULARGE_INTEGER* ppos){
		return (stream)?stream->Seek(e,m,ppos):E_NOTIMPL;
	}	
	virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* st, DWORD dw) {
	     return (stream)?stream->Stat(st,dw):E_NOTIMPL;
	}

	ipc_utils::smart_ptr_t<ISequentialStream> sstream;
	ipc_utils::smart_ptr_t<IStream> stream;


	template <class Intf>
	inline static HRESULT CreateInstanceForMarshal(ISequentialStream* inner_stream,IUnknown* pUnk,Intf** ppstream)
	{
		struct FStream_t:aggregator_helper_base_t<FStream_t>
		{

			inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

				return stream(Outer,riid,ppObj,hr);
			}


			FStream_t(ISequentialStream* inner_stream,IUnknown* punk):stream(inner_stream),unkhold(punk){
							

			};

			IStreamAggregator_Impl stream;
			ipc_utils::smart_ptr_t<IUnknown> unkhold;		

		};

		bool f;
		HRESULT hr;
		if(!ppstream) return E_POINTER;
		ipc_utils::smart_ptr_t<IStream> tmp;
		if(HGSTREAM==inner_stream)
		{
			if(FAILED(hr=CreateStreamOnHGlobal(0,true,tmp.address()))) return hr;
			inner_stream=tmp;

		}

		ipc_utils::smart_ptr_t<FStream_t> sp(new  FStream_t(inner_stream,pUnk),0);
		aggregator_container_helper_FMT_EC<false,true> ac;
		f  = SUCCEEDED(hr=ac.set_aggregator(__uuidof(ISequentialStream),__uuidof(IStream),sp) );		
		f && SUCCEEDED(hr=ac.QueryInterface(ppstream));             
		return hr;
	}	

	template <class Intf>
	inline static HRESULT CreateInstance(ISequentialStream* inner_stream,IUnknown* pUnk,Intf** ppstream)
	{
		struct FStream_t:aggregator_helper_base_t<FStream_t>
		{

			inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

				return stream(Outer,riid,ppObj,hr);
			}


			FStream_t(ISequentialStream* inner_stream,IUnknown* punk):stream(inner_stream),dwUnk(0){
				HRESULT hr;
				if(punk) 
					hr=ipc_utils::GIT_t::get_instance().wrap(punk,&dwUnk);

			};

			IStreamAggregator_Impl stream;
			DWORD dwUnk;


			~FStream_t()
			{
				(dwUnk)&&(ipc_utils::GIT_t::get_instance().unwrap(dwUnk));
			}

		};

		bool f;
		HRESULT hr;

		if(!ppstream) return E_POINTER;
       
        ipc_utils::smart_ptr_t<IStream> tmp;
		if(HGSTREAM==inner_stream)
		{
			if(FAILED(hr=CreateStreamOnHGlobal(0,true,tmp.address()))) return hr;
			inner_stream=tmp;

		}

		//if()


		
		ipc_utils::smart_ptr_t<FStream_t> sp(new  FStream_t(inner_stream,pUnk),0);
		aggregator_container_helper_FMT_EC<true,true> ac;



		f  = SUCCEEDED(hr=ac.set_aggregator(__uuidof(ISequentialStream),__uuidof(IStream),sp) );
		//f && SUCCEEDED(hr=ac.set_aggregator(__uuidof(IStream),sp) );
		//f && SUCCEEDED(hr=ac.set_FTM() ) && SUCCEEDED( hr=ac.set_ExternalConnect());
		f && SUCCEEDED(hr=ac.QueryInterface(ppstream));             

		return hr;
	}


};



template <int PIPESIZE=4096>
//struct ISequentialStreamPipe_Impl:base_component_t<ISequentialStreamPipe_Impl<PIPESIZE>,ISequentialStream>
struct ISequentialStreamPipe_Impl:base_aggregator_t<ISequentialStreamPipe_Impl<PIPESIZE>,IStream>
{
	struct handle_t
	{
		handle_t(HANDLE _h=0):h(_h){}

		~handle_t(){ close(); }

		  inline void close()		  {
			  if(h) CloseHandle(ipc_utils::make_detach(h));
		  };

		inline operator HANDLE(){ return h;}
		HANDLE h;

	};

	//ISequentialStreamPipe_Impl(IUnknown* punk,HANDLE h_r,HANDLE h_w):base_component_t<ISequentialStreamPipe_Impl<PIPESIZE>,ISequentialStream>(punk),unk(punk)
	//ISequentialStreamPipe_Impl(IUnknown* punk,HANDLE h_r,HANDLE h_w):unk(punk)
	ISequentialStreamPipe_Impl(HANDLE h_r,HANDLE h_w)
		:hread(ipc_utils::dup_handle(h_r)),hwrite(ipc_utils::dup_handle(h_w)){}

	//ipc_utils::smart_ptr_t<IUnknown> unk;

	handle_t hread,hwrite;

	virtual HRESULT STDMETHODCALLTYPE Read(void *pv,ULONG cb, ULONG *pcbRead)
	{
		HRESULT hr;
		ULONG cbRead=0;
		bool f;

		char* p=(char*)pv;
		while( cb>0	)
		{
			int c=(cb<=PIPESIZE)?cb:PIPESIZE,co=0;
			if(!(f=ReadFile(hread,p,c,(DWORD*)&co,0)))
				return hr=HRESULT_FROM_WIN32(GetLastError());
			cb-=co;
			p+=co;
			cbRead+=co;
		}

		if(pcbRead) *pcbRead=cbRead;
		return hr=S_OK;	

	};

	virtual  HRESULT STDMETHODCALLTYPE Write( const void *pv,ULONG cb,ULONG *pcbWritten) 
	{

		HRESULT hr;
		ULONG cbWritten=0;
		bool f;

		char* p=(char*)pv;
		while( cb>0	)
		{
			int c=(cb<=PIPESIZE)?cb:PIPESIZE,co=0;
			if(!(f=WriteFile(hwrite,p,c,(DWORD*)&co,0)))
				return hr=HRESULT_FROM_WIN32(GetLastError());
			cb-=co;
			p+=co;
			cbWritten+=co;
		}

		if(pcbWritten) *pcbWritten=cbWritten;
		return hr=S_OK;	
		return hr;
	};

	inline bool operator()(IUnknown* punkOuter,REFIID riid,void** ppObj,HRESULT& hr)
	{
		return inner_QI(punkOuter,riid, ppObj,hr)||inner_QI(punkOuter,riid,__uuidof(ISequentialStream), ppObj,hr);	
	}

	virtual HRESULT STDMETHODCALLTYPE Revert(void){
		hread.close();
		hwrite.close();		
		return S_OK;
	}                            

	virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER)	{return E_NOTIMPL;}
	virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*,ULARGE_INTEGER*){return E_NOTIMPL;}
	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD cb)	{ 		return E_NOTIMPL;   	}
	
	virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)      {return E_NOTIMPL;}	
	virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)    {return E_NOTIMPL;}      
	virtual HRESULT STDMETHODCALLTYPE Clone(IStream **)  {return E_NOTIMPL;}                                	
	virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER, DWORD,ULARGE_INTEGER*){return E_NOTIMPL;}	
	virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* , DWORD ) {return E_NOTIMPL;}
	
	~ISequentialStreamPipe_Impl()
	{

	}

	inline static HRESULT CreateInstance(IUnknown* pUnk,HANDLE h_r,HANDLE h_w,ISequentialStream** ppstream)
	{
		struct FStream_t:aggregator_helper_base_t<FStream_t>
		{

			inline bool   inner_QI(IUnknown* Outer,REFIID riid,void** ppObj,HRESULT& hr ){

				return stream(Outer,riid,ppObj,hr);
			}


			FStream_t(IUnknown* punk,HANDLE h_r,HANDLE h_w):stream(h_r,h_w),dwUnk(0){
				HRESULT hr;
				if(punk) 
					hr=ipc_utils::GIT_t::get_instance().wrap(punk,&dwUnk);

			};

			ISequentialStreamPipe_Impl stream;
			DWORD dwUnk;


			~FStream_t()
			{
				(dwUnk)&&(ipc_utils::GIT_t::get_instance().unwrap(dwUnk));
			}

		};

		if(!ppstream) return E_POINTER;
		bool f;
		HRESULT hr;
		ipc_utils::smart_ptr_t<FStream_t> sp(new  FStream_t(pUnk,h_r,h_w),0);
		aggregator_container_helper ac;

		f  = SUCCEEDED(hr=ac.set_aggregator(__uuidof(ISequentialStream),sp) );
		f && SUCCEEDED(hr=ac.set_aggregator(__uuidof(IStream),sp) );
		f && SUCCEEDED(hr=ac.set_FTM() ) && SUCCEEDED( hr=ac.set_ExternalConnect());
		f && SUCCEEDED(hr=ac.QueryInterface(ppstream));             

		return hr;
	}


inline static HRESULT CreateStreamPair(IUnknown* pUnk,ISequentialStream** pps1,ISequentialStream** pps2)
	{
		HRESULT hr;


		if(!(pps1&&pps2)) return E_POINTER;


		handle_t hr1,hw1,hr2,hw2;
		bool f;
		if(!(f=CreatePipe(&hr1.h,&hw1.h,0,PIPESIZE)))
			return HRESULT_FROM_WIN32(GetLastError());
		if(!(f=CreatePipe(&hr2.h,&hw2.h,0,PIPESIZE)))
			return HRESULT_FROM_WIN32(GetLastError());



		ipc_utils::smart_ptr_t<ISequentialStream> s;   


		if(SUCCEEDED(hr=ISequentialStreamPipe_Impl::CreateInstance(pUnk,hr1,hw2,&s.p)))
			if(SUCCEEDED(hr=ISequentialStreamPipe_Impl::CreateInstance(pUnk,hr2,hw1,pps2)))
				*pps1=s.detach();

		return hr;
	};

inline static HRESULT CreateInstance(IUnknown* pUnk,HANDLE h_r,HANDLE h_w,IStream** ppstream)
{
      return CreateInstance(pUnk,h_r,h_w,(ISequentialStream**)ppstream);
}
inline static HRESULT CreateStreamPair(IUnknown* pUnk,IStream** pps1,IStream** pps2)
{
	return CreateStreamPair(pUnk,(ISequentialStream**)pps1,(ISequentialStream**)pps2);
}

};

template <class IS>
inline HRESULT CreateStreamPair(IUnknown* pUnk,IS** pps1,IS** pps2)
{
	const int PIPESIZE=4*1024;
	return ISequentialStreamPipe_Impl<PIPESIZE>::CreateStreamPair(pUnk,pps1,pps2);

};
