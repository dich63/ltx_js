#pragma once


#include "singleton_utils.h"
#include "engine.h"


#pragma comment(lib, "delayimp")

//libmx.lib libmat.lib libeng.lib
#pragma comment(lib, "libmx.lib") 
#pragma comment(lib, "libmat.lib") 
#pragma comment(lib, "libeng.lib") 


#pragma comment(lib, "mclxlmain.lib") 


#import "mwcomutil.tlb" no_namespace, raw_interfaces_only

#include "mclcom.h"
#include "mclcommain.h"
//*/
#define ML_STRUB L"matlab_engine_{2EEA0A46-9C66-44c3-9E78-5A39DCF0D025}"

typedef mutex_cs_t engine_mutex_t;   




template <int Ver=0>
struct matlab_engine_t
{
    typedef mutex_cs_t engine_mutex_t;   

	struct env_var_stack_t
	{

		env_var_stack_t(const wchar_t* name,const wchar_t* val):pold(0)
		{
			if(name)
			{
        	bufname.cat(name);
          sz=GetEnvironmentVariableW(name,0,0);
		  pold=buf.resize(sz+4).get();
		  sz=GetEnvironmentVariableW(name,pold,0);
		  SetEnvironmentVariableW(name,val);
			}
		}
         ~env_var_stack_t()
		 {
            if(bufname.size_b())
				SetEnvironmentVariableW(bufname.get(),pold); 
		 }


      v_buf<wchar_t> buf;
	  v_buf<wchar_t> bufname;
	  wchar_t * pold;

	  int sz;
	};

	static engine_mutex_t& engine_mutex()
	{
         static    engine_mutex_t s_mutex;
		 return s_mutex;
	}


	static bool init_once()
	{
		locker_t<engine_mutex_t> lock(engine_mutex());
		_MCLCONVERSION_FLAGS flags={};
				InitConversionFlags(&flags);

		return true;
	}
	   

	matlab_engine_t(const wchar_t* objref=0,int msgbufsize=8*1024*1024-16):hr(E_POINTER),engine(0)    
	{
		init(objref,msgbufsize);

	}
  matlab_engine_t& init(const wchar_t* objref=0,int msgbufsize=8*1024*1024-16)
   {        
	        
			hr=E_POINTER;
			HRESULT hrl;
			if(objref){

			msgbuf.resize((msgbufsize>(1024*1024))?msgbufsize+16:(1024*1024));
			pbuf=msgbuf;
			locker_t<engine_mutex_t> lock(engine_mutex());
						
			//SetEnvironmentVariableW(L"LTX_MONIKER_EXTERNAL_OBJECT",ML_STRUB);
			//SetEnvironmentVariableW(ML_STRUB,objref);
			env_var_stack_t stack_moniker(L"LTX_MONIKER_EXTERNAL_OBJECT",ML_STRUB);
			env_var_stack_t stack_objref(ML_STRUB,objref);
			
			engine=engOpenSingleUse(0,0,(int*)&hr);
			//SetEnvironmentVariableW(ML_STRUB,0);
			if(S_OK==hr)
			  hr=engOutputBuffer(engine,msgbuf.get(),msgbufsize);

			  hr=HRESULT_FROM_WIN32(hr);

			}

			return *this;
   }
   

	void dispose(){

         //if(SUCCEEDED(make_detach(hr,E_POINTER)))
		
		Engine* t;
			 if(t=make_detach(engine))
				 engClose(t);
			 //hr=E_POINTER;
			 msgbuf.clear();
			 pbuf=0;

	}

   ~matlab_engine_t()
   {
	   dispose();
	   //hr=E_POINTER;
   }

   template <class CH>
   HRESULT execute(const CH* str)
   {
	   HRESULT hrl=hr;
	   //msgbuf[0]=0;
	   
	   int cb=msgbuf.size_b();
	   memset(pbuf,0,cb);
	   if(SUCCEEDED(hrl))
	   {
           
		   hrl=engEvalString(engine,char_mutator<CP_THREAD_ACP>(str));
	   }
		   return hrl=HRESULT_FROM_WIN32(hrl);           
   }

   /*
   HRESULT fill_msg_buffer(v_buf<char>& msgbuffer)
   {
             HRESULT hrl=hr;
			 if(SUCCEEDED(hrl))
				 hrl=engOutputBuffer(engine,msgbuffer.get(),msgbuffer.sizeb()-8);

            return hrl;
   }
   */

 inline   bstr_t get_msg_buffer()
   {
	   return pbuf;
   }
 inline   HRESULT get_msg_buffer(VARIANT* pv)
 {
	 OLE_CHECK_PTR(pv);
	 VARIANT v={VT_BSTR};
	 v.bstrVal=get_msg_buffer().Detach();
	 *pv=v;
	 return S_OK;
 }
   HRESULT hr;
   Engine* engine; 
   v_buf<char> msgbuf; 
   char* pbuf;


};