#pragma once



//struct fake_lock {};


struct cycle_cursor
{

	//typedef locker<cycle_cursorT<T>,T> lockerT;


	struct region
	{
		size_t offset;
		size_t size;

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
	};



	size_t begin_offset;
	size_t end_offset;
	size_t size;
	bool cf;
	cycle_cursor(){};
	cycle_cursor(size_t buf_size,size_t b=0,size_t e=0,bool f=false):size(buf_size),begin_offset(b),end_offset(e),cf(f){};


	inline bool is_empty()
	{
		return  (!cf)&&(end_offset==begin_offset);
	}
	inline bool is_full()
	{
		 return  cf;
	}

	inline size_t _distance(int b,int e)
	{
		int sz=e-b;
		sz=(sz>=0)? sz:sz+int(size); 
		return sz;
	}

	inline size_t fill_count()    
	{
		return (cf)? size: _distance(begin_offset,end_offset);
	}

	
	inline size_t free_count()    
	{
		return  unfill_count();    
	}

	
	inline size_t unfill_count() 
	{
		return (cf)? 0: _distance(end_offset,begin_offset);
	};

	inline region_pair  regions()
	{

		region_pair bp=region_pair();

		if(!is_empty()) 
		{
            bp.first.offset=begin_offset;
			if(begin_offset<end_offset)			   
				bp.first.size=fill_count();
			else
			{
				bp.first.size=size-begin_offset;
				bp.second.size=end_offset;
			}

		}
		return bp;
	};


   
    inline cycle_cursor conj()
	{
          return cycle_cursor(size,end_offset,begin_offset,_is_empty());
	}
	
   
	inline region_pair  read_region_pair()
	{
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
		if(cb)
		{
		  if(cf&&(cb>unfill_count())) return false;
        	 cf=(begin_offset==_add_mod(end_offset,cb));
 	
		}
		return true;
	};

	inline	bool sub(size_t cb)
	{
		if(cb)
		{
		 if(cb>fill_count()) return false;   
  		   _add_mod(begin_offset,cb);
			cf=false;
		}
	
		return true;
	};

};



template <class B,class T=B,bool fnochecked=false>
struct locker
{ 
	T*    po;
	locker(B* p)
	{
		po=static_cast<T*>(p);
		if(fnochecked||po) po->lock();
	}
	~locker()
	{
		if(fnochecked||po) po->unlock();
	}
};


struct lock_base
{

	inline void lock(){};
	inline void unlock(){};
	inline void on_full(bool f) {};
	inline void on_empty(bool f){};	
};


template <class T>
struct locked_cycle_cursor_base:lock_base
{
	
	#define THIS_LOCK locker<locked_cycle_cursor_base<T>,T,true> ll00AA(this);  
	//cycle_cursor(size_t buf_size,size_t b=0,size_t e=0,bool f=false):cycle_cursorT<cycle_cursor>(buf_size,b,e,f){};
	locked_cycle_cursor_base(size_t buf_size=0):m_cursor(buf_size){};
	inline size_t capacity(){ return m_cursor.size;};
	inline size_t& capacity_ref(){ return m_cursor.size;};

    inline bool is_empty()
	{
          THIS_LOCK
		return m_cursor.is_empty();
	};
	inline bool is_full()
	{
		THIS_LOCK
			return m_cursor.is_full();
	};
	inline region_pair  read_region_pair()
	{
		THIS_LOCK
		return m_cursor.regions();
	};
	inline region_pair  write_region_pair()
	{
			return conj().regions();
	};


	inline	bool add(size_t cb)
	{
		
		if(cb)
		{
			THIS_LOCK
         //if(cf&&(cb>unfill_count())) return false;
          if(!m_cursor.add(cb)) return false;
			
			T* pt=static_cast<T*>(this);
          if(m_cursor.is_full()) 
			  pt->on_full(true);
   		  pt->on_empty(false);

		}
       return true;
	};

	inline	bool sub(size_t cb)
	{

		if(cb)
		{
			THIS_LOCK
				//if(cf&&(cb>unfill_count())) return false;
			if(!m_cursor.sub(cb)) 
				return false;

			T* pt=static_cast<T*>(this);
			if(m_cursor.is_empty()) 
				pt->on_empty(true);

			pt->on_full(false);


		}
		return true;
	};


	inline  bool operator << (size_t cb)
	{
		return add(cb);
	}

	inline  bool operator >> (size_t cb)
	{
		return sub(cb);
	}




protected:
 cycle_cursor conj()
 {
       THIS_LOCK
       return 	m_cursor.conj();
 }
 cycle_cursor m_cursor;
};


struct fake_signal 
{
	inline void  operator =(bool f){};
	
};



template <typename mutex=lock_base, typename signal_nonempty=fake_signal, typename signal_overflop=fake_signal >
struct thread_safe_cycle_cursor_base:locked_cycle_cursor_base< thread_safe_cycle_cursor_base<mutex,signal_nonempty,signal_overflop> > 
{

    mutex* m_pmutex;
	signal_nonempty m_ready;
	signal_overflop* m_overflop;
    thread_safe_cycle_cursor_base(size_t buf_size=0,mutex* pm=0,signal_nonempty* psne=0,signal_overflop* povfp=0)
		:locked_cycle_cursor_base(buf_size),m_pmutex(pm),m_ready(psne),m_overflop(povfp){};

	inline void lock()  {  if(m_pmutex) m_pmutex->lock(); };
	inline void unlock()  { if(m_pmutex)  m_pmutex->unlock();};

	inline void on_full(bool f) 
	{ 
		if(m_overflop) *m_overflop=f;
	};
	inline void on_empty(bool f) 
	{
        if(m_ready) *m_ready=!f;
	};	

};



template <typename mutex, typename signal_nonempty=fake_signal, typename signal_overflop=fake_signal >
struct locked_cursor: thread_safe_cycle_cursor_base<mutex,signal_nonempty,signal_overflop>
{
      mutex m_inner_mutex;
  locked_cursor(size_t buf_size=0,signal_nonempty* psne=0,signal_overflop* povfp=0)
	  :thread_safe_cycle_cursor_base(buf_size,&m_inner_mutex,psne,povfp){};
};




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
		fok=mc<<3;
		bp=mc.coregions();
		fok=mc<<(4);
		fok=mc>>(5);
		bp=mc.coregions();
		fok=mc<<3;
        cb=mc.fill_count();
		cb2=mc.conj().free_count();
        bp0=mc.regions();
		bp=mc.coregions();
	}

};
