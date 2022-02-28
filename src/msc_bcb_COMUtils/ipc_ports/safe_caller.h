#pragma once
//safe_caller.h
#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>

#include "callback_context_arguments.h"
#include "ipc_ports/ipc_utils.h"

//MAKEINTRESOURCE
namespace ipc_utils{




	template <int _VERSION=0>
	struct safe_caller_t
	{
		struct V_C_t:VARIANT
		{

			V_C_t(VARIANT _v=VARIANT()){
				*((VARIANT*)this)=_v;
			};
			~V_C_t(){
				VariantClear(this);
			}

		};


		inline void static  _fill_ExceptInfo(EXCEPINFO* pexcepinfo,wchar_t* pmsg,wchar_t* psource=0,void* proc=0) 
		{
			if(pexcepinfo)
			{  
				*pexcepinfo=EXCEPINFO();
				bstr_t bdesc= (wchar_t*)pmsg;
				bool f=psource;
				bstr_t bsource;
				if(psource) bsource=psource;
				else{
					bsource=(wchar_t*)module_t<>::module_name(proc);
				}
				
				pexcepinfo->scode=DISP_E_EXCEPTION;
				pexcepinfo->bstrDescription=bdesc.Detach();
				pexcepinfo->bstrSource=bsource.Detach();

			}
		}

		inline bool static fill_ExceptInfo(EXCEPINFO* pexi,IErrorInfo* ei=0)
		{
			if(!pexi) return 0;
			IErrorInfo* eih=0;
			if(ei==0)
			{
				if(GetErrorInfo(0,&eih))
					return 0;
				ei=eih;
			}
			EXCEPINFO& exi=*pexi;
			exi.scode=DISP_E_EXCEPTION;
			ei->GetDescription(&(exi.bstrDescription));
			ei->GetSource(&(exi.bstrSource));
			ei->GetHelpContext(&(exi.dwHelpContext));
			ei->GetHelpFile(&(exi.bstrHelpFile));
			if(eih) eih->Release();
			return 1;
		};

		//static	void __stdcall __raise_exception(wchar_t* smessage,wchar_t* ssource=0)
		static	void  __raise_exception(wchar_t* smessage,wchar_t* ssource=0)
		{
			ULONG_PTR args[2]={ULONG_PTR(smessage),ULONG_PTR(ssource)};

			RaiseException(0xBABAEB,0,2,args);
		}

		static	DWORD __exception_filter(void* proc,EXCEPTION_POINTERS* pep,
			EXCEPINFO* pexcepinfo)
		{
			PEXCEPTION_RECORD per=pep->ExceptionRecord;
			if(pexcepinfo)
			{
				if(per->ExceptionCode==0xBABAEB)
				{
					_fill_ExceptInfo(pexcepinfo,(wchar_t*)per->ExceptionInformation[0],(wchar_t*)per->ExceptionInformation[1],proc);
				}
				else fill_ExceptInfo(pexcepinfo);
			}
			return EXCEPTION_EXECUTE_HANDLER;
		}

		inline static void safecall(void *proc,void *presult=0,void* pctx=0,void *perrp=0,DWORD c=0,void *pvar=0,DWORD cn=-1,void* pf=0,void* pex=0,void* flags=0)
		{
			if(!proc) return ;

#if defined(_WIN64)

			typedef void (*x64__safe_wild_call_t)(void* param0,void* param1,void* param2,void* param3,void* param4,void* param5,void* param6,void* param7,void* param8);

			((x64__safe_wild_call_t)proc)(presult,pctx,perrp,(void*)c,pvar,(void*)cn,pf,pex,flags);
			//(param0,param1,param2,param3,param4,param5,param6,param7);

#else

			DWORD ustack;
			__asm {
				mov ustack,ESP;
				push flags;
				push pex;
				push pf;
				push cn;
				push pvar;
				push c;
				push perrp;
				push pctx;
				mov  ECX,presult;
				push ECX;
				mov  EAX,proc;
				call EAX;
				mov ESP,ustack;
			}; 
#endif

		};

inline static HRESULT call_prop(void* proc,void*& pcntxt,DWORD dwFlags,int argc,VARIANT* pargv,VARIANT* presult,EXCEPINFO* pexcepinfo,DISPID did,IDispatch*  pthis)
		{

			struct args_obj_t:args_base_t<args_obj_t,RefCounterFake_t<1> > 
			{
				args_obj_t(int c,VARIANT *pv,raise_error_proc_t re)
					:args_base_t(c,pv,true,re){}
			} args_obj(argc,pargv,&__raise_exception);

			HRESULT hr=S_OK;
			
			VARIANT r={VT_EMPTY};
			WORD wFlags=WORD(0x0000FFFF&dwFlags);

			void* pcc;

			

			if(!presult) presult=&r;
			callback_context_arguments_t cca=
			{&args_obj,&__raise_exception,pcntxt,argc,pargv,presult,wFlags,&hr,pexcepinfo,&pcntxt,0,&process_com_t::get_instance().callback_lib_arguments,pthis};
			cca.tss_id.id=did;
			if(did==callback_context_arguments_t::disp_id_t::TSSid)
			{
                cca.tss_id.tssName=tls_com_t<>::get_instance().get_ptr<wchar_t>();
			}

			 pcc=((dwFlags&((FLAG_CBD_OLD)<<16))==0)?(void*)&cca:(void*)&__raise_exception;
			 if((dwFlags&((FLAG_CBD_COM)<<16))) 
				 pcc=&args_obj;

			__try
			{				
				if((proc)) 								
					safecall(proc,presult,pcntxt,pcc,argc,pargv,argc-1,&hr,pexcepinfo,(void*)wFlags);				
				else	 __raise_exception(L"server callback method not implemented.");
			}
			__except(__exception_filter(proc,GetExceptionInformation(),pexcepinfo))
			{	
				hr=DISP_E_EXCEPTION;
			};
			::VariantClear(&r);
			return hr;
		};




	};

     struct  base_callsback_dispatch_base_t
	 {
		 void* pcalback;
		 void* context;
		 DISPID id0;		
		 DWORD flagmask;
		 base_callsback_dispatch_base_t(){}
		 base_callsback_dispatch_base_t(void* _pcalback,void* _context=0,DISPID id=DISPID_VALUE,DWORD _flagmask=DISPATCH_PUTSF)
			 :pcalback(_pcalback),context(_context),id0(id),flagmask(_flagmask){}; 
	 };

	template<class T,class IDispatchX=IDispatch >
	struct  base_callsback_dispatch_t:base_callsback_dispatch_base_t
	{

		typedef safe_caller_t<> caller_t;
		typedef typename base_callsback_dispatch_t<T,IDispatchX> base_callsback_t;

		inline HRESULT findname(LPOLESTR name,DISPID& dispid){

			HRESULT hr;
			static  BEGIN_STRUCT_NAMES(pn)										
				DISP_PAIR_IMPLC(call,DISPID_VALUE)								
				END_STRUCT_NAMES;
			hr=find_id(pn,name,&dispid);
			return hr;
		}

		inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT*,IDispatch* pthis){

			WORD flagmask_lo=WORD(flagmask&0x0FFFF);
			DWORD flagmask_hi=DWORD(flagmask&0xFFFF0000);
			if((id!=id0)||((flags)&flagmask_lo)) return E_NOTIMPL;

			safe_caller_t<>::V_C_t tmp;

			if(!res) res=&tmp;


			   HRESULT hr;
			   int c=dp->cArgs;
			   VARIANT* pdv=dp->rgvarg;
			   if(is_put(flags))
			   {
				   bool f=(dp->cNamedArgs==1)&&(dp->rgdispidNamedArgs[0]==DISPID_PROPERTYPUT);				   
				   if(!f)
					   return E_NOTIMPL;
				   --c;
				   res=pdv++;
			   }

			   hr=safe_caller_t<>::call_prop(pcalback,context,flagmask_hi|DWORD(flags),c,pdv,res,pexi,id,pthis);
			   
			   return hr;         
		}
		base_callsback_dispatch_t(void* _pcalback=0,void* _context=0,DISPID id=DISPID_VALUE,DWORD _flagmask=DISPATCH_PUTSF)
		{
			init(_pcalback,_context,id,_flagmask);
		}
		
		
		T* init(void* _pcalback=0,void* _context=0,DISPID id=DISPID_VALUE,DWORD _flagmask=DISPATCH_PUTSF)
		{
	           *((base_callsback_dispatch_base_t*)this)=base_callsback_dispatch_base_t(_pcalback,_context,id,_flagmask);		

			return static_cast<T*>(this);
		}

		inline void* reset_calback(void* pnew=0)
		{
            return InterlockedExchangePointer(&pcalback,pnew);
		}

		inline void* get_calback()
		{
			return InterlockedCompareExchangePointer(&pcalback,0,0);
		}

          
		
	};

};//ipc_utils