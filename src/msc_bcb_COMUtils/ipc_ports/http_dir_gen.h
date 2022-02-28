#pragma once

#include "wchar_parsers.h"
#include "windows.h"
#include <set>
#include <shlwapi.h>
struct lessfile_t: public std::binary_function<WIN32_FIND_DATAW,WIN32_FIND_DATAW, bool>
{	// functor for operator<



	bool operator()(const WIN32_FIND_DATAW& l, const WIN32_FIND_DATAW& r) const
	{	// apply operator< to operands
		int ld=(l.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY);
		int rd=(r.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY);
         
		if(ld==rd) return wcscmp(l.cFileName,r.cFileName)<0;
		else return (ld>rd);
		//return 0;
	}
};

struct http_dir_gen_t
{




   char* prolog0;
   char* prolog1;
   char* prolog2;
   char* prolog3;
   char* epilog;
   char* fmt;
   char* icon[3];
   char* root;
   char_mutator<CP_UTF8> dir;

	v_buf<char> str;
	char* pstr;




	//
	typedef std::set<WIN32_FIND_DATAW, lessfile_t > filelist_t;
	filelist_t	filelist;
	//std::set<int, std::less<int> > filelist;



operator char*()
{
	char* p;
	return p=str.get();
}

inline int size(){ return str.size_b();}

inline v_buf<char>& rem_slash(v_buf<char>& buf)
{
	char* pstr=buf;
	if(pstr)
	{
		int i=strlen(pstr)-1;
		for(;i>=0;--i)
		{
			if(pstr[i]=='/') pstr[i]=0;
			else 
			{
				return buf.resize(i+1);
			}	
		}

	}
	
	return buf;
}

char* parent_dir(char* p)
{
   char *pp;
   if(pp=strrchr(p,'/')) pp[1]=0;
   else return 0;
   return p;
}

bool make_dirlist(wchar_t *dirname)
{
   const DWORD dwpp= *((DWORD*)L"..");  
   const WORD wp= *((WORD*)L".\0");  
   int fr=safe_len(root);

   WIN32_FIND_DATAW  wfd;
   
   v_buf<wchar_t> mask;
   v_buf_fix<char,4096> vbuf;
    char* pbuf=vbuf;

    mask<<dirname<<L"\\*.*";
   wchar_t *dirmask=mask;
   HANDLE hf=FindFirstFileW(dirmask,&wfd);
   if(hf==INVALID_HANDLE_VALUE) return false;
   str.printf(prolog0,(char*)dir);
   str<<prolog1<<prolog2<<prolog3;
   pstr=str;


   do 
   {
	   if(fr || ( ((dwpp!=*((DWORD*)wfd.cFileName)))&&(wp!=*((WORD*)wfd.cFileName)) ))
		   filelist.insert(wfd);


   } while(FindNextFileW(hf,&wfd));

   FindClose(hf);

   for(filelist_t::iterator i=filelist.begin();i!=filelist.end();++i)
   {
     bool freload=false;   
     const WIN32_FIND_DATAW&  wfd=*i;
     char_mutator<CP_UTF8> cm(wfd.cFileName);
	 char* pfn=cm;
	 int im=(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)?1:0;
	 int cb=4096;	 
	 if(im&&(dwpp==*((DWORD*)wfd.cFileName))) 
	 		 im=2;

	  v_buf<char> tmp;
	  char* pfnn=pfn,*p;

	  if(fr)
	  {
       if(im==2)
	   {
		   v_buf<char> tmp2;
            //pfnn= tmp<<root<<"/../";
		   //  pfnn= tmp<<"../";
			//char*p=strrchr(pfnn,'/');
			//if(p) p[1]=0; 
		   rem_slash(tmp2<<root);
		   if(!(p=parent_dir(tmp2))) continue;
           tmp<<"/"<<p;      
		   pfnn=tmp;


	   }
	   else
		   if(wp==*((WORD*)wfd.cFileName))
		   {
             pfnn="javascript:location.reload(true)";
			 freload=true;
		   }
		   //
		   else 
		   {
			   tmp<<"/";
			   if(root)
			   {
				   tmp<<root;
				   
				   //
				   /*
				   p=strrchr(root,'/');
                      if((!p)||(*p));
					   tmp<<"/";
					 // */

                       //
				   rem_slash(tmp);
				   tmp<<"/";
					     
		  	   }


			   pfnn=tmp<<pfn;
		   }
		   //		   else  pfnn= tmp<<pfn;

	  }

	 
	 //if(im==2)		 pbuf="../";	 else
	     


		 //if(fr)   pfnn= tmp<<root<<"/"<<pfn;
		  char *pfin;
	  	  if(freload) pfin=pfnn;	
		  else 	  url_escape2(pfnn,-1,pfin=pbuf,&cb);
	     	 
	 str<<v_buf<char>().printf(fmt,icon[im],pfin,pfn).get();

     pstr=str;

   };// while(FindNextFileW(hf,&wfd));

   str<<epilog;
   pstr=str;
   
   return true;
}


http_dir_gen_t(wchar_t *dirname,char* _root=""):dir(dirname)
{
	root=(_root)?_root:"";
    if(*root=='/') ++root;
   // prolog0="HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html>\n<html><head>\n<title>%s</title>\n";
	 prolog0="HTTP/1.0 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html>\n<html><head>\n<title>%s</title>\n";
	prolog1="<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'><style> h1 { border-bottom: 1px solid #c0c0c0; margin-bottom: 10px; padding-bottom: 10px; white-space: nowrap; } table { border-collapse: collapse; } tr.header { font-weight: bold; } td.detailsColumn { padding-left: 2em; text-align: right; white-space: nowrap; } a.icon { padding-left: 1.5em; text-decoration: none; } a.icon:hover { text-decoration: underline; } a.file { background : url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAIAAACQkWg2AAAABnRSTlMAAAAAAABupgeRAAABHUlEQVR42o2RMW7DIBiF3498iHRJD5JKHurL+CRVBp+i2T16tTynF2gO0KSb5ZrBBl4HHDBuK/WXACH4eO9/CAAAbdvijzLGNE1TVZXfZuHg6XCAQESAZXbOKaXO57eiKG6ft9PrKQIkCQqFoIiQFBGlFIB5nvM8t9aOX2Nd18oDzjnPgCDpn/BH4zh2XZdlWVmWiUK4IgCBoFMUz9eP6zRN75cLgEQhcmTQIbl72O0f9865qLAAsURAAgKBJKEtgLXWvyjLuFsThCSstb8rBCaAQhDYWgIZ7myM+TUBjDHrHlZcbMYYk34cN0YSLcgS+wL0fe9TXDMbY33fR2AYBvyQ8L0Gk8MwREBrTfKe4TpTzwhArXWi8HI84h/1DfwI5mhxJamFAAAAAElFTkSuQmCC ') left top no-repeat; } a.dir { background : url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAd5JREFUeNqMU79rFUEQ/vbuodFEEkzAImBpkUabFP4ldpaJhZXYm/RiZWsv/hkWFglBUyTIgyAIIfgIRjHv3r39MePM7N3LcbxAFvZ2b2bn22/mm3XMjF+HL3YW7q28YSIw8mBKoBihhhgCsoORot9d3/ywg3YowMXwNde/PzGnk2vn6PitrT+/PGeNaecg4+qNY3D43vy16A5wDDd4Aqg/ngmrjl/GoN0U5V1QquHQG3q+TPDVhVwyBffcmQGJmSVfyZk7R3SngI4JKfwDJ2+05zIg8gbiereTZRHhJ5KCMOwDFLjhoBTn2g0ghagfKeIYJDPFyibJVBtTREwq60SpYvh5++PpwatHsxSm9QRLSQpEVSd7/TYJUb49TX7gztpjjEffnoVw66+Ytovs14Yp7HaKmUXeX9rKUoMoLNW3srqI5fWn8JejrVkK0QcrkFLOgS39yoKUQe292WJ1guUHG8K2o8K00oO1BTvXoW4yasclUTgZYJY9aFNfAThX5CZRmczAV52oAPoupHhWRIUUAOoyUIlYVaAa/VbLbyiZUiyFbjQFNwiZQSGl4IDy9sO5Wrty0QLKhdZPxmgGcDo8ejn+c/6eiK9poz15Kw7Dr/vN/z6W7q++091/AQYA5mZ8GYJ9K0AAAAAASUVORK5CYII= ') left top no-repeat; } a.up { background : url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAmlJREFUeNpsU0toU0EUPfPysx/tTxuDH9SCWhUDooIbd7oRUUTMouqi2iIoCO6lceHWhegy4EJFinWjrlQUpVm0IIoFpVDEIthm0dpikpf3ZuZ6Z94nrXhhMjM3c8895977BBHB2PznK8WPtDgyWH5q77cPH8PpdXuhpQT4ifR9u5sfJb1bmw6VivahATDrxcRZ2njfoaMv+2j7mLDn93MPiNRMvGbL18L9IpF8h9/TN+EYkMffSiOXJ5+hkD+PdqcLpICWHOHc2CC+LEyA/K+cKQMnlQHJX8wqYG3MAJy88Wa4OLDvEqAEOpJd0LxHIMdHBziowSwVlF8D6QaicK01krw/JynwcKoEwZczewroTvZirlKJs5CqQ5CG8pb57FnJUA0LYCXMX5fibd+p8LWDDemcPZbzQyjvH+Ki1TlIciElA7ghwLKV4kRZstt2sANWRjYTAGzuP2hXZFpJ/GsxgGJ0ox1aoFWsDXyyxqCs26+ydmagFN/rRjymJ1898bzGzmQE0HCZpmk5A0RFIv8Pn0WYPsiu6t/Rsj6PauVTwffTSzGAGZhUG2F06hEc9ibS7OPMNp6ErYFlKavo7MkhmTqCxZ/jwzGA9Hx82H2BZSw1NTN9Gx8ycHkajU/7M+jInsDC7DiaEmo1bNl1AMr9ASFgqVu9MCTIzoGUimXVAnnaN0PdBBDCCYbEtMk6wkpQwIG0sn0PQIUF4GsTwLSIFKNqF6DVrQq+IWVrQDxAYQC/1SsYOI4pOxKZrfifiUSbDUisif7XlpGIPufXd/uvdvZm760M0no1FZcnrzUdjw7au3vu/BVgAFLXeuTxhTXVAAAAAElFTkSuQmCC ') left top no-repeat; } #listingParsingErrorBox { border: 1px solid black; background: #fae691; padding: 10px; display: none; }</style></head>";
   //prolog2="\n<body background='data:image/jpg;base64,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";
   //prolog3="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'>";
    prolog2="\n<body style='background-color:#E8ECE8'>"; 
	prolog3="";
   epilog="\n</body></html>";
   //fmt="\n<td class='detailsColumn'></td></tr><tr><td><a class='icon %s' href='%s'  target=_parent>%s</a></td><br>";
   fmt="\n<td class='detailsColumn'></td></tr><tr><td><a class='icon %s' href='%s'  target=_self>%s</a></td><br>";
   char* ii[3]={"file","dir","up"};
   memcpy(icon,ii,sizeof(ii));


  make_dirlist(dirname);
  

}

 int save_to_file(wchar_t *filename)
 {
	 FILE* hf=_wfopen(filename,L"wb");
     if(!hf) return 0;   
	 int cb=fputs(str.get(),hf);
	 fclose(hf);
	 return cb;
 }

};

struct http_magic_root_t
{
	v_buf<wchar_t> buf,buf2,tmp,buff;
	HRESULT hr;

	argv_file<wchar_t> argsf;
	const wchar_t* root_rel;
	wchar_t *pfile,*path,*pfilename;

	http_magic_root_t(){};

	http_magic_root_t(const wchar_t* filen,const wchar_t* prefix)
	{
        init(filen,prefix);
	}

http_magic_root_t& init(const wchar_t* filen,const wchar_t* prefix)
	{
		hr=E_FAIL;
		//char_mutator cm_ptfx(prefix);
	    bool f;
		int c=safe_len(filen,1);
		if(!c) return *this;
		 buf2.resize(c+1024);
		if(!GetFullPathNameW(filen,c+1024,path=buf2,&pfilename))
		{
			hr=GetLastError();
			 return *this;;
		}

		if(pfilename&&(pfilename[0]==L'.'))
		{
           argv_reg<wchar_t> argr(pfilename,HKEY_CLASSES_ROOT);
		    
			  if(safe_cmpni(argr[L""],L"hssh_html",safe_len(L"hssh_html"))==0)
			  { pfilename[0]=0;};
  		}


		buff.cat(buf2.get());
		if(pfilename)
		{
		*(pfilename-1)=0;
		  pfile=pfilename;

		}


		argsf.open((wchar_t*)filen);
		//argsf.set_prefix_name(v_buf<wchar_t>().cat(prefix).cat(L".").get());
		 hr=S_OK;
		if(!argsf.argc) 		  return *this;

		wchar_t* prefix_root;
		//prefix_root=(wchar_t*)prefix;
		//prefix_root=buf.printf(L"%s\.root_rel",prefix).get();

        v_buf<wchar_t> bbb;
         prefix_root=bbb.cat(prefix).cat(L".root_rel");
		root_rel=argsf[prefix_root].def(L"");
		if(!root_rel[0]) return *this;

		tmp.cat(path).cat(root_rel);
        buf.resize(c=(tmp.count()+1024));
		wchar_t*p=0,*pb,*pc;
		pb=buf;
		pc=tmp;
		c=buf.count();
		if(!GetFullPathNameW(pc,c,pb,&p))
			return *this;

	       //return *this;
		    wchar_t* pff=buff,*pp=buf;
            int cc=safe_len(pff);
            f=(pff==StrStrNIW(pff,pp,cc));
			if(f)
			{
			  c=safe_len(buf.get());	
			  path=buf;
			  if((c>0)&&((path[c-1]==L'/')||(path[c-1]==L'\\'))) path[c-1]=0; 
			  pfile=buff.get()+c;         

			}


		
			return *this;
	
	}

inline 	operator bool(){  return hr==S_OK; }

};