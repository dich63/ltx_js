#pragma once

#include "os_utils.h"
#include "buf_fm.h"
#include "free_com_factory.h"


inline bool  IsComPresents()
{
void *p=0;
HRESULT hr=0;
//
hr = ::CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER,__uuidof(IGlobalInterfaceTable),&p);
//hr = ::CoCreateInstance(__uuidof(GlobalInterfaceTable), NULL, CLSCTX_INPROC_SERVER,__uuidof(IGlobalInterfaceTable),&p);
return false;
return S_OK==hr;
//
hr= CoInitialize(NULL);CoUninitialize();
return hr;
//
//
return S_OK==hr;
}

#define nDBG 4
/*
struct dtest
{
  DWORD m[nDBG];

};
*/
struct scriptor_loader
{
  //public:
    _bstr_t m_fn[nDBG], m_mon[nDBG];
    buf_fm_T<char,CREATE_ALWAYS> tmpmap[nDBG];
	
     HRESULT m_hr;
//	 dtest mmm;
     //CComPtr<IDispatch> m_cache_scriptor[nDBG];  
     CComAutoCriticalSection m_cs;

//	 
	 scriptor_loader():m_hr(CO_E_NOTINITIALIZED)
	 {
         // mmm=dtest();
	 };
	 scriptor_loader(char* psrc)
	 {
        reload( psrc);
	 }
inline CComAutoCriticalSection& CriticalSection()
{
	return m_cs;
}
inline HRESULT  reload(char* psrc)
	{
		
			
        CS_AUTOLOCK(m_cs);
		if(!psrc) return m_hr=E_POINTER;
		m_hr=0;
        long cb=strlen(psrc)+256;
		char *flag[2]={"false","true"},*pstart;
for(unsigned i=0;i<nDBG;i++)
{
        m_mon[i]=BSTR(NULL);
		//m_cache_scriptor[i].Release();

}
		
		if(!(pstart=StrStrA(psrc,"<component")))
			   return E_FAIL;
		
         long cb_head=long(pstart)-long(psrc); 

		for(unsigned i=0;i<nDBG;i++)
		{
         	//
			tmpmap[i].m_lock=1;
          
        _bstr_t tmp=new_tmp_file(L"~za",&m_hr); 
		if(FAILED(m_hr)) return m_hr;
	    tmpmap[i].open(tmp,cb,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,CREATE_ALWAYS);//,FILE_FLAG_DELETE_ON_CLOSE);
		if(m_fn[i].length()) DeleteFileW(m_fn[i]);
		m_fn[i]=tmp;
		char* p=((char*)tmpmap[i]);
		if(!p) {  m_hr=E_FAIL;break;};
        lstrcpynA(tmpmap[i],psrc,cb_head);
		if(cb_head) p+=cb_head-1			;
		sprintf(p,"\n\<\?component error=\"%s\" debug=\"%s\" \?\>\n",flag[(i>>1)],flag[i&1]);
        lstrcatA(p,pstart);
		m_mon[i]="script:"+m_fn[i];
		//		if(FAILED(m_hr=get_scriptor(i,&(m_cache_scriptor[i])))) break;
       };
		return m_hr;
	 };


~scriptor_loader()
{ 
	//
	

	CS_AUTOLOCK(m_cs);
	bool f=false;//IsComPresents();
	for(int i=0;i<nDBG;i++) 
	{ 
		
		//if(f)    {m_cache_scriptor[i].Release();}
		//else      {m_cache_scriptor[i].Detach();};
		tmpmap[i].unmap();
		DeleteFileW(m_fn[i]); 
	}
};



inline HRESULT   get_scriptor(int indx,IDispatch** ppdisp)
{
	CS_AUTOLOCK(m_cs);
	HRESULT hr;
	if(FAILED(m_hr)) return m_hr;
    if(!ppdisp) return E_POINTER;
    //hr=CoGetObject(m_mon[indx],NULL,__uuidof(IDispatch),(void**)ppdisp);
	hr=free_com_factory::CoGetObject(m_mon[indx],NULL,ppdisp);   
	
	if(FAILED(hr)) // Fucking Vista!!! syphilis  preservation
	{
		int tid=free_com_factory::MTA_tid();
		hr=free_com_factory::CoGetObject(m_mon[indx],NULL,ppdisp,tid);   
	}
	  return hr;
}

inline operator HRESULT()
{
	CS_AUTOLOCK(m_cs);
   return m_hr;
}


};




//template <int RES_ID,const wchar_t* SECTION,const wchar_t* PREFIX>
struct script_res
{
   buf_fm_T<char,OPEN_EXISTING> m_fm;
   buf_res_T<char> m_res;
   char* m_str;
   
  //__MS_RESOURCE_POLNOE_GAVNO_NULL

   script_res(int RES_ID, wchar_t* SECTION,wchar_t* PREFIX)
{
	m_str=NULL;
	_bstr_t fn=get_private_profile_string(SECTION,_bstr_t(PREFIX)+L".path.name",L"");
	
	if(fn.length())
	{
		_bstr_t sp=get_private_profile_string(SECTION,_bstr_t(PREFIX)+L".path.searchpath",L"");

		FileSearcher fs(fn,sp);
		if(fs)
		{
	        m_str=m_fm.open(fs.FileName());
			return;
		}

	}
    m_str=m_res.open(HINSTANCE_IN_CODE,MAKEINTRESOURCEW(RES_ID),RT_HTML);     
}

 inline operator bstr_t()
 {
	   
	   //return ms_resource_gavno_resolver(m_str);
	 //SysAllocStringLen()
	 int cb=m_res.m_count;
	 if(cb)
	 {
	 std::vector<char> tmp(cb+2);
	 std::copy(m_str,m_str+cb,tmp.begin());
	 char* ps=(char*)(&tmp[0]);

	 return bstr_t(ps);

	 }
	 else return L"";
	 



 }

/*
inline operator char*()
{
   return m_str;
}
*/
inline operator bool()
{
	return m_str!=NULL;
}
};



#define FAST_RELOAD 1

inline  HRESULT regenerate_wch(scriptor_loader& scr_files, int resid ,DWORD rmode=FAST_RELOAD)
{
	   HRESULT hr;

	   if((rmode&1)&& SUCCEEDED(scr_files))
		   return S_OK;

        CS_AUTOLOCK(scr_files.CriticalSection());

		if((rmode&1)&& SUCCEEDED(scr_files))
			return S_OK;

	   
        script_res srcres(resid,L"SCRIPTS",L"scriptor");

/*
  _bstr_t fn=get_private_profile_string(L"SCRIPT",L"path.name",L"");
  _bstr_t sp=get_private_profile_string(L"SCRIPT",L"path.searchpath",L"");
  
  if(fn.length())
  {
    FileSearcher fs(fn,sp);
    if(fs)
	{
       buf_fm_T<char,OPEN_EXISTING> src(fs.FileName());
	   if(src)
	   {
         if(SUCCEEDED(scr_files.reload(src))) return S_OK;
	   }
         
	}

  }
   buf_res_T<char>  srcres(HINSTANCE_IN_CODE,MAKEINTRESOURCEW(resid),RT_HTML);
   */

      hr=scr_files.reload(bstr_t(srcres));
	return hr;
}
