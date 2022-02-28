#pragma once

#include <video/singleton_utils.h>


template <class Mutex =mutex_fake_t, class Pos=int,class Size=int>
struct cycle_buffer_t
{
	typedef Pos pos_t;
	typedef Size size_t;
	typedef Mutex mutex_t;


	struct region_t
	{
		size_t size;
		pos_t offset;

		inline  operator bool() 
		{
			return size!=0;
		}

		template <class N>
		N* operator()(N* p)
		{
			return p+offset;
		};

		inline  void* operator()(void* p)
		{
			return ((char*)p)+offset;
		};

		template <class BUFT,class L>
        inline  void* set_buf_len(void* p,BUFT*& buf,L& l)
		{
            buf=(BUFT*)(((char*)p)+offset);
			l=L(size);
            return buf;
		}

	};


	struct region_pair_t
	{
		union{
			struct{region_t reg[2];};
			struct{region_t first,second;};
		};
		inline  operator bool() 
		{
			return first||second;
			//(!reg[0].size)&& (!reg[1].size);
		}
		inline  region_t& operator[](int n)
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
		
		inline     region_pair_t& pack()
		{
			if(!first)
			{
				first=second;
				second.size=0;
			}
			return *this;
		}

        template < class io_t>
		inline 	int set_scatter_gather(void* pbuf,io_t* pio)
		{
			 pio[0].len=0;
			 pio[1].len=0;
			 int c=1;
			 if(reg[0])
			     reg[0].set_buf_len(pbuf,pio[0].buf,pio[0].len);
			 else
			 {
				 if(reg[1])  
				 {
					 reg[1].set_buf_len(pbuf,pio[0].buf,pio[0].len);
					 return 1;
				 }
				 else return 0;
			 }
			 if(reg[1])
			 {
				 reg[1].set_buf_len(pbuf,pio[1].buf,pio[1].len);
				 ++c;

			 };
                
			 return c;
			 


		}


	};

  
   template <bool fconj>
   struct cursor_t
   {
	   pos_t& b;  
	   pos_t& e;
	   bool& cf;
	   size_t& size;
      cursor_t(cycle_buffer_t& buf):b(fconj?buf.e:buf.b),e(fconj?buf.b:buf.e),cf(buf.cf),size(buf.size)
	  {

	  };

     inline bool cflag()
	 {
		 //		 return (b==e)&&(fconj^cf);
		 //
		 return (fconj^cf);
	 }
	 inline bool set_cflag(bool f=true)
	 {
		 return cf=(b==e)&&(fconj^f);
	 }


     inline bool is_full()
	 {
        return (b==e)&&cflag();
	 }
     inline bool is_empty()
	 {
		 return (b==e)&&(!cflag());
	 }
     
	inline  size_t distance()
	{	
		if(is_empty()) return size;
		else return  (e<b)?e+size-b:e-b;	
	}



	
	inline  size_t next(size_t cb)
	 {
		 
		 size_t d=distance();

		 if(cb>d) return 0;		 

		 if(cb)
		 {
            b=(b+cb)%size;
        	set_cflag();
		 }

		 return cb; 
	 }

	inline region_pair_t  regions(size_t cb=-1)
	{
		region_pair_t bp=region_pair_t();

		if(cb<0) cb=size;
		size_t d=distance();

		if(cb>d) return bp;

		if(!is_full()) 
		{
			
			bp.first.offset=b;
			if(b<e)			   
				bp.first.size=cb;
			else 
			{
				size_t ss=size-b;
				if(cb>ss)
				{
				 bp.first.size=ss;
				 bp.second.size=cb-ss;
				}
				else  bp.first.size=cb;
			}

		}
		return bp;
	};





   };


template <class BUF>
struct iobuf_t
{
   BUF buf[2];
   BUF* pbuf;
   int nbuf;
   void *p;
   iobuf_t(void *pp,BUF* _pbuf=0)
   {
	   p=pp;
	   pbuf=(_pbuf)?_pbuf:buf;   
   }

   int operator()(region_pair_t& regpair)
   {
	     return  nbuf=regpair.set_scatter_gather(p,pbuf);
   }

};




   cycle_buffer_t(size_t _size):size(_size),b(0),e(0),cf(false)
	   ,cursor(*this),cocursor(*this)
   {};
   

   template <class IOFunctor,class Cursor  >
   size_t op(IOFunctor& io, Cursor& _cursor, size_t maxcb=-2)
   {
	   locker_t<mutex_t> lock(mutex);

	   size_t d=_cursor.distance();
	   if(maxcb<0)	   maxcb=size/(-maxcb);

	   size_t cbr=(maxcb<d)?maxcb:d;

	   region_pair_t  r=_cursor.regions(cbr);

	   if(!r) return 0;
           io(r);

   	   return cbr;
   }


   template <class Cursor>
   inline   size_t op_complete( Cursor& _cursor,size_t cb)   
   {
	   locker_t<mutex_t> lock(mutex);
	   return _cursor.next(cb);
   }


template <class BUF>
inline   int get_recv_buf(void* p,BUF* pbuf,size_t maxcb=-2)
{
	int cc;
	iobuf_t<BUF> iob(p,pbuf);
     cc=op(iob,cursor,maxcb);
	 return iob.nbuf;
}

template <class BUF>
inline   int get_send_buf(void* p,BUF* pbuf,size_t maxcb=-2)
{
	int cc;
	iobuf_t<BUF> iob(p,pbuf);
	cc=op(iob,cocursor,maxcb);
	return iob.nbuf;
}


inline   size_t recv_complete(size_t cb)   
  {
	  return op_complete(cursor,cb);
  }

inline   size_t send_complete(size_t cb)   
  {
	  return op_complete(cocursor,cb);
  }


  template <class RecvFunctor>
   size_t recv_op(RecvFunctor& recvf,size_t maxcb=-2)
   {
	   return op(recvf,cursor,maxcb);
   };

   template <class SendFunctor>
   size_t send_op(SendFunctor& sendf,size_t maxcb=-2)
   {
	   return op(sendf,cocursor,maxcb);      
   }



   inline    bool is_empty(){ return cursor.is_empty();}
   inline    bool is_full(){ return cursor.is_full();}

   pos_t b;  
   pos_t e;
   bool cf;
   size_t size;

   cursor_t<false> cursor;
   cursor_t<true> cocursor;
   mutex_t mutex;
  

};