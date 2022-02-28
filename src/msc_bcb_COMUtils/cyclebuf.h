#ifndef __CYCLEBUFFERSCEL__H
#define __CYCLEBUFFERSCEL__H
//#pragma once




template <class B,class T=B>
struct locker
{ 
	 T*    po;
locker(B* p)
{
    po=static_cast<T*>(p);
	if(po) po->lock();
}
~locker()
{
	if(po) po->unlock();
}
};

//struct fake_lock {};

template <class T>
struct cycle_cursorT
{

	//typedef locker<cycle_cursorT<T>,T> lockerT;


	struct region
	{
		size_t size;
		size_t offset;

     inline  operator bool() 
	 {
		 return size!=0;
	 }


//inline	_ret_type operator() ( _arg_type0 txt COMMA_IF(n,m) ARGS_Z( _arg_type A_,n,m) ) 

 template <class N>
  N* operator()(N* p)
	 {
		 return p+offset;
	 };

	};

	
	struct region_pair
	{
		union{
			struct{region reg[2];};
            struct{region first,second;};
		};
		inline  operator bool() 
		{
			return first||second;
			//(!reg[0].size)&& (!reg[1].size);
		}
      inline  region& operator[](int n)
	  {
		  return reg[n];
	  }
	  inline size_t size()
	  {
		  return first.size+second.size;
	  }
     inline int count()
	 {
		 return ((reg[0].size)?1:0)+((reg[1].size)?1:0);
	 }

	   region_pair subregions(size_t offset,size_t size)
	   {
         
         region_pair r;


		 return r;
	   }

inline     region_pair& pack()
	   {
		   if(!first)
		   {
			   first=second;
			   second.size=0;
		   }
		   return *this;
	   }

	  size_t set_scatter_gather(void* pbuf,void* pscatter_gather)
	  {
		  struct _iovec
		  {
              unsigned long len;
			  char* pbase;
		  } emt={0,0};
          size_t c=0;
         _iovec* pio=(_iovec*)pscatter_gather;
		 pio[0]=emt;
		 pio[1]=emt;
		 if(reg[0])
		 {
			pio[c].len=reg[0].size;
            pio[c].pbase=reg[0]((char*)pbuf);
			++c;
		 }
		 
		 if(reg[1])
		 {
			 pio[c].len=reg[1].size;
			 pio[c].pbase=reg[1]((char*)pbuf);
			 ++c;
		 }
         return c;
	  }
	};


#define THIS_LOCK lockerT __ll00AA__00_(this);  
	struct lockerT
	{ 
		T*    po;
		lockerT(cycle_cursorT<T>* p)
		{
			po=static_cast<T*>(p);
			po->lock();
		}
		~lockerT()
		{
			po->unlock();
		}
	};


	size_t begin_offset;
	size_t end_offset;
	size_t size;
	bool cf;
	cycle_cursorT(){};
	cycle_cursorT(size_t buf_size,size_t b=0,size_t e=0,bool f=false):size(buf_size),begin_offset(b),end_offset(e),cf(f){};


	inline void lock()  { };
	inline void unlock()  { };

	inline void on_full(bool f)  { };
	inline void on_empty(bool f)  { };



	inline bool is_empty()
	{
		THIS_LOCK;
		return  _is_empty();
	}

	inline bool _is_empty()
	{
		return  (!cf)&&(end_offset==begin_offset);
	}
	inline bool is_full()
	{
		THIS_LOCK;
		 return  cf;
	}

	inline size_t _distance(int b,int e)
	{
		int d=e-b;
		//d=(d>=0)? d:d+int(size); 
		if(d<0) d+=int(size);
		return d;
	}

	inline size_t _fill_count()    
	{
		return (cf)? size: _distance(begin_offset,end_offset);
	}

	inline size_t fill_count()    
	{
		THIS_LOCK;
		return _fill_count();    
	}

	inline size_t free_count()    
	{
		 return conj()._fill_count();
		//return  unfill_count();    
	}

	inline size_t unfill_count()    
	{
        THIS_LOCK;
		return  _unfill_count();    
	}
	inline size_t _unfill_count() 
	{
		return (cf)? 0: _distance(end_offset,begin_offset);
	};

	inline region_pair  regions()
	{

		region_pair bp=region_pair();

		if(!_is_empty()) 
		{
            //size_t s=fill_count();
			bp.first.offset=begin_offset;
			if(begin_offset<end_offset)			   
				bp.first.size=_fill_count();
			else
			{
				bp.first.size=size-begin_offset;
				bp.second.size=end_offset;
			}

		}
		return bp;
	};


    
	
	
	

    inline cycle_cursorT conj()
	{
        THIS_LOCK;
	  return cycle_cursorT(size,end_offset,begin_offset,_is_empty());
	}
	inline region_pair  coregions()
	{
		
		return conj().regions();
	};
   
	inline region_pair  read_region_pair()
	{
		THIS_LOCK;
		return regions();
	};
	inline region_pair  write_region_pair()
	{
		
		return conj().regions();
	};



	inline  bool operator << (size_t cb)
	{
	    return add(cb);
	}

	inline  bool operator >> (size_t cb)
	{
		return sub(cb);
	}


	inline size_t _add_mod(size_t &v,size_t n)
	{
		size_t e=v+n;
		v=e%size;
		return v;
	};


	inline	bool add(size_t cb)
	{
         //size_t cb0=free_count(); 
		
		if(cb)
		{
		
         
		  THIS_LOCK;
		  if(cf&&(cb>_unfill_count())) return false;
		  //if(cf||(cb>_distance(end_offset,begin_offset))) return false;

		  T *t=static_cast<T*>(this);

      	  bool f= cf=(begin_offset==_add_mod(end_offset,cb));

    	          t->on_empty(false);
          if(f)  t->on_full(true);
		  	
		}
		return true;
	};

	inline	bool sub(size_t cb)
	{
		//size_t cb0=fill_count(); 
		
		if(cb)
		{
			THIS_LOCK;
		 if(cb>_fill_count()) return false;   
  		   _add_mod(begin_offset,cb);
			cf=false;
			T *t=static_cast<T*>(this);
			if(_is_empty()) t->on_empty(true);
			  t->on_full(false);


		}
	
		return true;
	};

};




struct cycle_cursor:cycle_cursorT<cycle_cursor>
{
	cycle_cursor(){};
	//cycle_cursor(size_t buf_size,size_t b=0,size_t e=0,bool f=false):cycle_cursorT<cycle_cursor>(buf_size,b,e,f){};
	cycle_cursor(size_t buf_size,size_t b=0,size_t e=0,bool f=false):cycle_cursorT(buf_size,b,e,f){};
 ~cycle_cursor()
 {

 };

};

struct fake_synchonizer 
{
	inline void  operator =(bool f){};
	inline void lock()  { };
	inline void unlock()  { };
};

template <typename mutex=fake_synchonizer, typename signal_nonempty=fake_synchonizer, typename signal_nonfull=fake_synchonizer >
struct thread_safe_cycle_cursor:cycle_cursorT<thread_safe_cycle_cursor<mutex,signal_nonempty,signal_nonfull> >
{

    mutex* m_mutex;
	signal_nonempty* m_canwrite;
	signal_nonfull* m_canread;
	
    thread_safe_cycle_cursor(size_t buf_size,mutex* pm=0,signal_nonempty* psne=0,signal_nonfull* povfp=0)
		:cycle_cursorT(buf_size),m_mutex(pm),m_canwrite(psne),m_canread(povfp){};

	inline void lock()  { if(m_mutex) m_mutex->lock(); };
	inline void unlock()  { if(m_mutex) m_mutex->unlock();};

	inline void on_full(bool f) 
	{ 
		if(m_canread) *m_canread=!f;
	};
	inline void on_empty(bool f) 
	{
        if(m_canwrite) *m_canwrite=!f;
	};	

};






template <class T,class CharT,class CycleCursor>
struct cycle_buffer_IO_T
{

	typedef typename CycleCursor::region_pair region_pair;
	typedef typename CycleCursor::region region;
	typedef typename  CharT buf_type;
   CharT* buf;
   CycleCursor* pc;
   

};




//template <typename CycleCursor,typename writeFunctor,typename fd_type,typename buf_type>
//write(CycleCursor*p )


struct test_cb
{


	typedef cycle_cursor cursor;
	typedef  cursor::region_pair block_pair;
	typedef  cursor::region region;
	char buf[32];

	cursor mc;
	bool fok;
    block_pair bp,bp0;
	int cb,cb2;
	test_cb()
	{
		mc=cursor(7);
		

		bp=mc.coregions();
		bp0=mc.regions();
		fok=mc<<3;
		bp=mc.coregions();
		bp0=mc.regions();
		fok=mc<<(4);
		bp=mc.coregions();
		bp0=mc.regions();
		fok=mc>>(5);
		bp=mc.coregions();
		bp0=mc.regions();
		fok=mc<<3;
        cb=mc.fill_count();
		cb2=mc.conj().free_count();
		bp=mc.coregions();
        bp0=mc.regions();
		
	}

};

#endif