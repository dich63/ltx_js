#pragma once 


struct ibuffer_t
{
	virtual void* ptr()=0;
	virtual void* resize(size_t siz)=0;
	virtual size_t size()=0;
	virtual ~ibuffer_t(){};



};

template <typename ReAllocProc,typename FreeProc=void (*)(void*),typename ContextT=void*>
struct buf_t:ibuffer_t 
{

	virtual void* ptr(){return p;};
	virtual void* resize(size_t siz)
	{
		return reallocate(siz);
	};

	virtual size_t size(){return _size;};


	void* p;
	ContextT pcontext;
	size_t _size;
	ReAllocProc alloc_p;
	FreeProc free_p;


	inline static void* _alloc(void* (*palloc)(void* ,size_t),void *p,size_t siz,ContextT pcontext,...)
	{
		return  palloc(p,siz);
	}



	template<typename AllocProcT,typename L> 
	inline static void* _alloc(AllocProcT _Alloc,void *p,size_t siz,L pcontext,...)
	{

		return  _Alloc(pcontext,p,siz);
	}


	inline static void _free(void(*pfree)(void*),ContextT pcontext,void* p)
	{
		pfree(p);
	}

	template<typename FreeProcT> 
	inline static void _free(FreeProcT _Free,ContextT pcontext,void* p)
	{
		_Free(pcontext,p);

	}


	void* reallocate(size_t cb)
	{      
		if(cb>0)	  return p=_alloc(alloc_p,p,_size=cb,pcontext,alloc_p);
		clear();
		return 0;
	}

	inline   void clear()
	{

		if(p) _free(free_p,pcontext,make_detach(p)); 
	}

	~buf_t(){ clear();}

	buf_t():_size(0),p(0){};
	buf_t(ReAllocProc r,FreeProc f,ContextT context):_size(0),p(0),alloc_p(r),free_p(f),pcontext(context){};
private:
	//buffer_t():size(0),p(0){};
	buf_t(const buf_t&){};

};


template <typename ReAllocProc,typename FreeProc,typename ContextT>
ibuffer_t* create_buffer(ReAllocProc r,FreeProc f,ContextT context)
{
	return new buf_t<ReAllocProc,FreeProc,ContextT>(r,f,context);
}
template <typename ReAllocProc,typename FreeProc>
ibuffer_t* create_buffer(ReAllocProc r,FreeProc f)
{
	return new buf_t<ReAllocProc,FreeProc>(r,f,0);
}

