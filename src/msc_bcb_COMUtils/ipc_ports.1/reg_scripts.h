#pragma once
//reg_scripts.h

#include "com_script_engine.h"

template <int Vers=1>
struct com_register_t
{
  
 template <class C>
 static HRESULT dll_class_register(C c,wchar_t* model=0,wchar_t* prog_id=0)
 {
   
  wchar_t* pdllclass_reg =js_text(
	     
		 var clsid=$$[0],lib=$$[1], model=$$[2],progid=$$[3];
         //alert(clsid+" "+lib+" "+model+" "+progid);
	    if(clsid&&lib) {	  
			    var ws=new ActiveXObject("WScript.Shell"),
				hclsid='HKEY_CLASSES_ROOT\\CLSID\\' + clsid + '\\';


		 ws.RegWrite(hclsid,'','REG_SZ');
		 ws.RegWrite(hclsid+'InprocServer32\\',lib,'REG_EXPAND_SZ');
		 if(model)   ws.RegWrite(hclsid+'InprocServer32\\ThreadingModel',model,'REG_SZ');
	    
		if(progid)
		{
             ws.RegWrite(hclsid+'ProgId\\',progid,'REG_SZ');
             ws.RegWrite('HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\'+progid+'\\','','REG_SZ');
			 ws.RegWrite('HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\'+progid+'\\CLSID\\',clsid,'REG_SZ');
			 ws.RegWrite('HKEY_CLASSES_ROOT\\'+progid+'\\','','REG_SZ');
			 ws.RegWrite('HKEY_CLASSES_ROOT\\'+progid+'\\CLSID\\',clsid,'REG_SZ');
		}
		};
	 ) ;


	 HRESULT hr;
	 ipc_utils::clsid_t clsid(c);
	 variant_t vclsid,vmodel,vlib,vprogid;
	 if(SUCCEEDED(hr=clsid.hr)) vclsid=bstr_t(clsid);
	 else return hr;
	 if(model) vmodel=bstr_t(model);
	 if(prog_id) vprogid=bstr_t(prog_id);
	 vlib=bstr_t((wchar_t*)ipc_utils::module_t<>::module_name() );
	 //
	 hr=ipc_utils::com_apartments::script_engine::js_t(pdllclass_reg)(vclsid,vlib,vmodel,vprogid);
	 //hr=ipc_utils::com_apartments::script_engine::script_execute(pdllclass_reg,0,vclsid,vlib,vmodel,vprogid);
	 //hr=ipc_utils::com_apartments::script_engine::script_execute(L"globalEval($$[0],$$[1],$$[2],$$[3],$$[4])",0,pdllclass_reg,vclsid,vlib,vmodel,vprogid);
     return hr;

 }

 template <class C>
static  HRESULT dll_class_unregister(C c)
 {


	  wchar_t* pdllclass_unreg=js_text(
		 var clsid=$$[0];
	 var ws=new ActiveXObject("WScript.Shell"),
		 hclsid='HKEY_CLASSES_ROOT\\CLSID\\' + clsid+'\\';
	 if(clsid)
		 ws.RegDelete(hclsid);	  
	 );


	 HRESULT hr;
	 ipc_utils::clsid_t clsid(c);
	 
	 if(SUCCEEDED(hr=clsid.hr)) 
	 {
		 variant_t vclsid=bstr_t(clsid);
		 hr=ipc_utils::com_apartments::script_engine::js_t(pdllclass_unreg)(vclsid);

	 }
	 //else return hr;
	 //hr=ipc_utils::com_apartments::script_engine::script_execute(pdllclass_unreg,0,vclsid);
	 return hr;
 }

};