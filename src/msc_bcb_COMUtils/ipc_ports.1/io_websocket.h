#pragma once

#include "hs_socket.h"
#include "wbs_buffer.h"
#include "video/singleton_utils.h"
#include "static_constructors_0.h"


template <class T,class _IOHANDLER>
struct on_completition_op_t
{
	enum op_t
	{
       nop =0,
	   send = 1<<0,
	   recv = 1<<1,
	   connect = 1<<2,
	   busy = 1<<16
	};
       //op_t operation;


	   //on_completition_op_t():operation(op_t::nop){};

       typedef typename _IOHANDLER io_handler_t;
	   typedef typename T this_t;


inline void operator()(long error,long bytes_transfered,io_handler_t* povl)
{
     this_t* t=static_cast<this_t>(this);

	 op_t op=t->get_op();

	 //if(error) t->on_close(error,bytes_transfered,povl);

	 else switch(op)
     {
	 case op_t::send : 
		 t->on_sent(error,bytes_transfered,povl);
		 break;
	 case op_t::recv : 
         t->on_message(error,bytes_transfered,povl);
		 break;
	 case op_t::connect: 
		 t->on_open(error,bytes_transfered,povl);
		 break;

	 }
}


};


template <class _IOHANDLER>
struct binder_t
{
	typedef typename _IOHANDLER io_handler_t;

	struct pool_item_t
	{
		io_handler_t*povl;
		long error;
		long bytes_transfered;
		pool_item_t(long _error,long _bytes_transfered,io_handler_t* _povl)
			:error(_error),bytes_transfered(_bytes_transfered),povl(_povl)
		{
            //hss_acquire(povl);
		};
		~pool_item_t(){
			hss_release(povl);
		}
		inline 	void operator()()
		{
			povl->on_completition_op(error,bytes_transfered,povl);
		}

	};


static void __stdcall s_iocr( long error,long bytes_transfered,io_handler_t* povl)
    {
		
	   	if(povl) asyn_call(new pool_item_t(error,bytes_transfered,povl));
        		
	};

 static bool bind(io_handler_t* pioh)
 {
	 bool f=BindIoCompletionCallback(HANDLE(pioh->s),LPOVERLAPPED_COMPLETION_ROUTINE(&s_iocr),0);
 }

};

struct isocket_delegate
{


};



template <class _IOHANDLER,class _BINDERT=binder_t<_IOHANDLER> >
struct io_websocket_t:on_completition_op_t<io_websocket_t<_IOHANDLER>,_IOHANDLER>
{
    typedef typename _IOHANDLER io_handler_t;
	typedef recv_buffer_t<12> io_buffer_t;
	typedef typename _BINDERT binder_t;


	HRESULT hr;
	io_handler_t* povl;

	io_websocket_t():hr(E_FAIL){};

	io_websocket_t&	init(io_handler_t* _povl)
	{
       povl=_povl;
		return this;

	}

 void	on_sent(int error,int bytes_transfered,io_handler_t* povl)
	{

	};

};