#pragma once
//dispatch_function.h

#include "safe_caller.h"
#include "dummy_dispatch_aggregator.h"
#include "ipc_ports/com_apartments.h"
//#include <video/singleton_utils.h>

namespace ipc_utils{
	//	template <class T ,class IDispatchX=IDispatch,class RefCounter=RefCounterMT_t<1> >
	//	struct IDispatch_base_t:IDispatch_impl_base_t<T,IUnknown_base_impl<T,IDispatchX,RefCounter > >{};

	struct dispparams_helper_t
	{
		DISPPARAMS* pdp;
		DISPPARAMS tmp;
		std::vector<VARIANT> vv;

		dispparams_helper_t(DISPPARAMS* pds,int freverse=0)
		{
			int c;
			if(freverse&&(c=pds->cArgs)){
				vv.resize(c);
				tmp=*pds;
				pdp=&tmp;
				pdp->rgvarg=&vv[0];
				std::reverse_copy(pds->rgvarg,pds->rgvarg+c,pdp->rgvarg);
		 }
			else      pdp=pds;
		}
		operator DISPPARAMS*()
		{
			return pdp;
		}

	};

	template <class RefCounter=RefCounterMT_t<1> ,class IDispatchX=IDispatch>
	struct calback_impl_t:	
		IDispatch_impl_base_t<calback_impl_t<RefCounter,IDispatchX>,IUnknown_base_impl< calback_impl_t<RefCounter,IDispatchX> ,IDispatchX,RefCounter > >	
		,base_callsback_dispatch_t<calback_impl_t<RefCounter,IDispatchX>,IFakeUnknown_t>

	{
		//typedef typename base_callsback_dispatch_t<calback_impl_t<RefCounter,IDispatchX>,IDispatchX> base_callsback_t;

		inline HRESULT findname(LPOLESTR name,DISPID& dispid){

			return base_callsback_t::findname(name,dispid);
		}

		inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID lsid,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT* pu){

			dispparams_helper_t dph(dp,freverse);
			return base_callsback_t::impl_Invoke(id,riid,lsid,flags,dph,res,pexi,pu);
		}


		calback_impl_t(void* _pcalback=0,void* _context=0,void* _on_exit=0,DISPID id=DISPID_VALUE,DWORD _flagmask=DISPATCH_PUTSF)
			:base_callsback_dispatch_t(_pcalback,_context,id,_flagmask),on_exit(_on_exit),freverse(0){

				ipc_utils::module_t<>::DLL_AddRef(_on_exit);
				ipc_utils::module_t<>::DLL_AddRef(_pcalback);
		};

		void* on_exit;
		int freverse;


		~calback_impl_t()
		{

			if(on_exit) safe_caller_t<>::safecall(on_exit,context);

			ipc_utils::module_t<>::DLL_AddRef(pcalback);
			ipc_utils::module_t<>::DLL_AddRef(on_exit);
		}
		static HRESULT CreateInstanceV(void* pcalback,void* context,void* on_exit,VARIANT* pres)
		{
			if(!pres) return E_POINTER;
			return toVARIANT(new calback_impl_t(pcalback,context,on_exit),pres,true);
		}
	};

	//template <class T,class IDispatchX=IDispatch >
	//struct IDispatch_impl_t:IDispatch_impl_base_t<T, base_aggregator_t<T,IDispatchX> >
	struct i_fake_t{};

	template <class IDispatchX=IDispatch>
	struct calback_aggregator2_t:IDispatch_impl_t<calback_aggregator2_t<IDispatchX>,IDispatchX>
	{

	};




	template <class IDispatchX=IDispatch>
	struct calback_aggregator_t:IDispatch_impl_t<calback_aggregator_t<IDispatchX>,IDispatchX>
		,base_callsback_dispatch_t<calback_aggregator_t<IDispatchX>,IFakeUnknown_t>
	{

		inline HRESULT findname(LPOLESTR name,DISPID& dispid){

			return base_callsback_t::findname(name,dispid);
		}

		inline	HRESULT impl_Invoke(DISPID id,REFIID riid,LCID lsid,WORD flags,DISPPARAMS*dp,VARIANT*res,EXCEPINFO*pexi, UINT* pu){

			dispparams_helper_t dph(dp,freverse);
			return base_callsback_t::impl_Invoke(id,riid,lsid,flags,dph,res,pexi,pu);
		}



		calback_aggregator_t(void* _pcalback=0,void* _context=0,void* _on_exit=0,DISPID id=DISPID_VALUE,DWORD _flagmask=DISPATCH_PUTSF)
			:base_callsback_dispatch_t(_pcalback,_context,id,_flagmask),on_exit(_on_exit),freverse(0){};

		void* on_exit;
		int freverse;

		~calback_aggregator_t()
		{
			if(on_exit) safe_caller_t<>::safecall(on_exit,context);
		}

		struct creator_calback_aggregator_t
		{

			typedef com_apartments::mta_singleton_t::apartment_t apartment_t;

			DWORD exflags;
			void* pcalback;
			void* context;
			void* on_exit;
			apartment_t* papartment;
			DWORD dwCookie;
			HRESULT hr;
			int fstart;


			struct closer_t
			{
				void* pcalback;
				void* pon_exit;
				apartment_t* papartment;
				closer_t(void* _pcalback,void* _on_exit,apartment_t* _papartment):pcalback(_pcalback),pon_exit(_on_exit),papartment(_papartment)
				{
					on_exit&&ipc_utils::module_t<>::DLL_AddRef(pon_exit);
					pcalback&&ipc_utils::module_t<>::DLL_AddRef(pcalback);
				}
				~closer_t()
				{
					if(papartment) apartment_t::on_exit(papartment);
					pcalback&&ipc_utils::module_t<>::DLL_Release(pcalback);
					on_exit&&ipc_utils::module_t<>::DLL_Release(pon_exit);

				}
				static void on_exit(void * pc)
				{
					delete (closer_t*)pc;
				}

			};
			HRESULT operator()(DWORD t) 
			{
				WORD flags=0xFFFF&exflags;
				WORD fapart=exflags>>16;
				COMInitF_t ci;
				aggregator_container_helper  container(new closer_t(pcalback,on_exit,papartment),NULL,&closer_t::on_exit);
				fstart=1;
				if((fapart&FLAG_CBD_FTM)||(fapart&FLAG_CBD_GIT)) container.add_FMT();
				//if(fapart&FLAG_CBD_EC) container.add_EC();
				smart_ptr_t<IUnknown> aggr;
				WORD frev=FLAG_CBD_REVERSED&fapart;
				if(fapart&FLAG_CBD_GIT){
					calback_impl_t<>* p=new calback_impl_t<>(pcalback,context,on_exit,DISPID_VALUE,exflags);
					p->freverse=frev; 
					smart_ptr_t<IDispatch> disp(p,0);
					OLE_CHECK_hr(create_GITDispatch_aggregator_helper(disp,&aggr.p));			
				}
				else {

					calback_aggregator_t *p=new calback_aggregator_t(pcalback,context,on_exit,DISPID_VALUE,exflags);
					p->freverse=frev;
					aggr=aggregator_helper_creator_t<calback_aggregator_t >::New(p);
					if(!aggr.p) return hr=E_OUTOFMEMORY;
				}		   
				

				if(fapart&FLAG_CBD_EC) mta_singleton_t::add_external_connector(container);
				hr=container.add(__uuidof(IDispatch),aggr).unwrap(&dwCookie);
				return hr;
			}

			~creator_calback_aggregator_t(){ if(!fstart) delete papartment;}

		};


		static HRESULT CreateInstance(DWORD exflags,void* pcalback,void* context,void* on_exit,DWORD* pdwCookie)//REFIID riid,void** ppObj)
		{
			HRESULT hr;
			OLE_CHECK_PTR(pdwCookie);
			WORD fapart=exflags>>16;
			DWORD t=0;
			creator_calback_aggregator_t::apartment_t* papartment;
			creator_calback_aggregator_t::apartment_t&	mta=com_apartments::mta_singleton_t::apartment();

			creator_calback_aggregator_t creator={exflags,pcalback,context,on_exit,0};
			if((fapart&FLAG_CBD_APP)&&((~fapart)&FLAG_CBD_FTM))
			{
				if(fapart&FLAG_CBD_STA) creator.papartment=papartment=new creator_calback_aggregator_t::apartment_t(COINIT_APARTMENTTHREADED);
				else papartment=&mta;

				hr=papartment->call_sync(creator,t,E_UNEXPECTED);			

			}
			else	
				hr=creator(t);

			if(SUCCEEDED(hr)) *pdwCookie=creator.dwCookie;
			return hr;

			//return toVARIANT(new calback_impl_t(pcalback,context,on_exit),pres,true);
		}

		static HRESULT CreateInstance(DWORD exflags,void* pcalback,void* context,void* on_exit,REFIID riid,void** ppObj)
		{
			OLE_CHECK_PTR(ppObj);
			HRESULT hr;
			DWORD dw;
			OLE_CHECK_hr(CreateInstance(exflags,pcalback,context,on_exit,&dw));
			hr=GIT_t::get_instance().unwrap(dw,riid,ppObj);
			return hr;

		}

		static HRESULT CreateInstanceV(DWORD exflags,void* pcalback,void* context,void* on_exit,VARIANT* pv){
			OLE_CHECK_PTR(pv);
			HRESULT hr;
			DWORD dw;
			OLE_CHECK_hr(CreateInstance(exflags,pcalback,context,on_exit,&dw));
			hr=GIT_t::get_instance().unwrap(dw,pv);
			return hr;

		}

	};







}//namespace ipc_utils