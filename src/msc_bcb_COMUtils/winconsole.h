#pragma once

#include <windows.h>
#include <comdef.h>

//#include "os_utils.h"
#include "conio.h"
#include "autocloser.h"

//inline _bstr_t vbprintf(_bstr_t fmt, va_list argptr)
//#define HAC INVALID_HANDLE_VALUE
#define M_AC 0x10000000
#define M_FC 0x000F0000
#define M_BC 0x00F00000
#define M_CC (M_BC|M_FC)
#define M_CCC (M_CC|M_AC)


#define HAC (console_active_buffer_t().hf)
#define HIB (console_input_buffer_t().hf)

inline HANDLE get_console_active_buffer()
{
    HANDLE   hf=CreateFileW(L"CONOUT$",GENERIC_READ|GENERIC_WRITE,
	  FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  return hf;
}

inline HANDLE get_console_input()
{
	HANDLE   hf = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	return hf;
}



struct console_input_buffer_t
{
	HANDLE   hf;
	console_input_buffer_t() :hf(get_console_active_buffer()) {}
	~console_input_buffer_t() { if (hf != INVALID_HANDLE_VALUE) CloseHandle(hf); }
	operator HANDLE() { return hf; };
};

struct console_active_buffer_t
{
	HANDLE   hf;
	console_active_buffer_t():hf(get_console_active_buffer()){}
	~console_active_buffer_t(){		if(hf!=INVALID_HANDLE_VALUE) CloseHandle(hf); }
	operator HANDLE(){return hf;};
};



inline HANDLE create_console_buffer()
{
	return  CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,CONSOLE_TEXTMODE_BUFFER,NULL);
}






inline  CONSOLE_SCREEN_BUFFER_INFO  console_info(HANDLE hout=HAC)
{
       CONSOLE_SCREEN_BUFFER_INFO scbi;
	   GetConsoleScreenBufferInfo(hout,&scbi);
	   return scbi;
}

inline COORD  console_xy()
{

   return console_info().dwCursorPosition;
}

struct coord : public COORD
{
	 
	coord(SHORT x,SHORT y){X=x;Y=y;};
  coord(){X=0;Y=0;};
   coord(COORD& c)
   {
      *((COORD*)this)=c;
   };
   coord(LONG32 dw)
   {
	   *((COORD*)this)=*((COORD*)&dw);
   };

inline coord operator=(COORD c)
{
	*((COORD*)this)=c;
	return *this;
}
inline operator DWORD()
{
	return *((DWORD*)this);
}
};

inline int copy_console_buffers(HANDLE hDest,HANDLE hSrc)
{
	//_auto_stdcall<HANDLE,BOOL,&CloseHandle> sH;
	//CHAR_INFO* pci=calloc(sizeof(CHAR_INFO),64*1024);   
	BOOL f;
	_auto_cdecl<void*,void,&::free,CHAR_INFO*> sCI(calloc(sizeof(CHAR_INFO),64*1024));
	CONSOLE_SCREEN_BUFFER_INFO scbi;
	SMALL_RECT sr={0,0,scbi.dwSize.X,scbi.dwSize.Y};
	GetConsoleScreenBufferInfo(hSrc,&scbi);
    f=ReadConsoleOutput(hSrc,sCI,scbi.dwSize,coord(0,0),&sr); 
	return int(f);
};







struct console_op
{
  HANDLE m_hout;
  CONSOLE_SCREEN_BUFFER_INFO m_scbi;
  console_op(HANDLE hout=HAC):m_hout(hout){};
inline  CONSOLE_SCREEN_BUFFER_INFO& info() 
{
       GetConsoleScreenBufferInfo(m_hout,&m_scbi);
	   return m_scbi;
}

};


struct consoleXY
{
HANDLE m_hout;
consoleXY(HANDLE hout=HAC):m_hout(hout){};

consoleXY(SHORT x,SHORT y,HANDLE hout=HAC):m_hout(hout)
{
//SetConsoleCursorPosition(m_hout,coord(x,y));

  *this=coord(x,y);
}

inline consoleXY& bottom(int x=0,int y=0)
{
	
    SMALL_RECT wr=console_info().srWindow;
     y+=wr.Bottom-wr.Top;
	*this=coord(x,y);
	return *this;
};

operator coord()
{
	CONSOLE_SCREEN_BUFFER_INFO scbi;
	GetConsoleScreenBufferInfo( m_hout,&scbi);
     return scbi.dwCursorPosition;
}
inline consoleXY& operator=(COORD c)
{
	SetConsoleCursorPosition( m_hout,c);
	return *this;
};
inline consoleXY& inc(SHORT dx,SHORT dy=0)
{
   coord c=coord(*this);
   c.X+=dx;c.Y+=dy;
   return (*this=c);
}
inline consoleXY& inc(coord dc)
{
 return inc(dc.X,dc.Y);
}

};

inline void clrscr(bool cc=false,HANDLE hout=GetStdHandle(STD_OUTPUT_HANDLE))
{
;
CONSOLE_SCREEN_BUFFER_INFO scbi;
GetConsoleScreenBufferInfo(hout,&scbi);
DWORD len=scbi.dwSize.X*scbi.dwSize.Y;
WORD attr=scbi.wAttributes;
DWORD wr;
COORD c={0,0};
FillConsoleOutputCharacterW(hout,L' ', len, c, &wr);
FillConsoleOutputAttribute(hout,attr, len, c, &wr);
if(cc) SetConsoleCursorPosition (hout,c);
};


inline bool set_console_size(COORD c,HANDLE hout=GetStdHandle(STD_OUTPUT_HANDLE))
{

SMALL_RECT r={0,0,c.X-1,c.Y-1} ;
//flag=GetConsoleScreenBufferInfo(hout,&csbi);
bool flag;
flag=SetConsoleScreenBufferSize(hout,c);
flag=SetConsoleWindowInfo(hout,1,&r);
flag=SetConsoleScreenBufferSize(hout,c);
flag=SetConsoleWindowInfo(hout,1,&r);
return flag;
}

inline int conputs(short attr,  wchar_t* pstr)//, bool keep_str=true)
{
	DWORD cb0;
	if(!pstr) return -1;
/*
  
   DWORD cb=wcslen(pstr),cb0;
  FillConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE),attr,cb,c,&cb0);
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),pstr,cb,&cb0,NULL); 
WriteConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE),pstr,cb,c,&cb0);
*/
  //_bstr_t buf;
  //if(keep_str) pstr=buf=pstr;
//
	coord c(5,5);
	//consoleXY();
int inc=0;
while((*pstr==L'\n')||(*pstr==L'\r')) 
  {    pstr++; 
       if(*pstr==L'\r')    consoleXY(0,coord(consoleXY()).Y);
 	   if(*pstr==L'\n')
		   WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),L"\n",1,&cb0,NULL); 
  };

wchar_t *p,*tail;
//consoleXY()=c;
while(*pstr)
{
  DWORD cb=0;
  inc=0;
  p=pstr;
  while((*pstr)&&(*pstr!=L'\n')) 
    {pstr++;cb++;};
  while(*pstr==L'\n')
  {
      
  pstr++;inc++;
  }
  

  //
  c=consoleXY();
 // _cwprintf(L"c x=%d y=%d\n",c.X,c.Y);
  FillConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE),attr,cb,c,&cb0);
  WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),p,cb,c,&cb0);
  while(inc--)  WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),L"\n",1,&cb0,NULL);  
  
  //token= wcstok_s(NULL,"\n",&tail);
};

//*/
 return 0;

}

inline DWORD console_pid()
{
	HWND hwin=GetConsoleWindow();
	if(!IsWindow(hwin)) return 0;
	DWORD pid=0;
	GetWindowThreadProcessId(hwin,&pid);
	return pid;

}

inline DWORD reattr(DWORD attr, DWORD o_attr=  console_info().wAttributes)
{
	if(M_FC&attr)
		attr=(0xFFFF00F0&attr)|(o_attr&0x0F);
	if(M_BC&attr)
		attr=(0xFFFF000F&attr)|(o_attr&0xF0);
  return attr;
}
  struct wchar_rep_t
  {
    int r; 
    wchar_t c;
  };

inline coord cons_fill(wchar_t c,int n=1,coord cxy=consoleXY(),HANDLE hout=GetStdHandle(STD_OUTPUT_HANDLE))
{
   DWORD tt;
    if(FillConsoleOutputCharacterW(hout,c,n,cxy,&tt))
	          cxy.X+=tt;
	return cxy;

}

inline coord cons_fill(wchar_rep_t* pcr,int nwr=1,coord cxy=consoleXY(),HANDLE hout=GetStdHandle(STD_OUTPUT_HANDLE))
{
	for(int n=0;n<nwr;n++)
		cxy=cons_fill(pcr[n].c,pcr[n].r,cxy,hout);
	return cxy;
}
inline coord con_progress_bar(double val,double maxval,int consize,wchar_t f=L'+',wchar_t b='o',DWORD attrf=M_FC|M_BC,DWORD attrb=M_FC|M_BC,coord cxy=consoleXY(),HANDLE hout=GetStdHandle(STD_OUTPUT_HANDLE))
{
  
	  consoleXY xy(hout);
     CONSOLE_SCREEN_BUFFER_INFO scbi;
     GetConsoleScreenBufferInfo(hout,&scbi);
	 DWORD tt;
            attrf=reattr(attrf,scbi.wAttributes);
			attrb=reattr(attrb,scbi.wAttributes);
     //SetConsoleTextAttribute(hout,attrf); 
     FillConsoleOutputAttribute(hout,attrb,consize,cxy,&tt);
     if(b) FillConsoleOutputCharacterW(hout,b,consize,cxy,&tt);
	 if(val>maxval) val=maxval;
	 double ccc=(val*consize)/maxval;
	 if(ccc-consize>0.5) consize=ccc+1;
	 else 	 consize=ccc;
	 //SetConsoleTextAttribute(hout,attrb); 
	 FillConsoleOutputAttribute(hout,attrf,consize,cxy,&tt);
     if(f) FillConsoleOutputCharacterW(hout,f,consize,cxy,&tt);
     //
	 //SetConsoleTextAttribute(hout,scbi.wAttributes);

   return  scbi.dwCursorPosition;
}



inline int conputs_hn(DWORD attr,wchar_t* pstr,int len=-1,HANDLE hout=HAC)
{
	if(!pstr) return 0;
	
	attr&=(~M_AC);
	//  _auto_stdcall<HANDLE,BOOL,&CloseHandle> sH;
	//  if(hout==HAC) 
	//	    hout=sH.attach(get_console_active_buffer());

    long ll=(len<0)?wcslen(pstr):len;
	DWORD ccw=0;
if((attr&(M_FC|M_BC))==(M_FC|M_BC)){
      WriteConsoleW(hout,pstr,ll,&ccw,0);
    }
else {
    	short o_attr=  console_info(hout).wAttributes;
	 if(M_FC&attr)
		attr=(0xFFFF00F0&attr)|(o_attr&0x0F);
	if(M_BC&attr)
		attr=(0xFFFF000F&attr)|(o_attr&0xF0);
	
	SetConsoleTextAttribute(hout,0x0000FFFF&attr);
	bool fA=(ll>0)&&(pstr[ll-1]==L'\n');
	if(fA) --ll;
	WriteConsoleW(hout,pstr,ll,&ccw,0);
	SetConsoleTextAttribute(hout,o_attr);
	if(fA){ WriteConsoleW(hout,L"\n",1,(DWORD*)&ll,0);ccw+=ll;};
  }
	return ccw;
}

inline int conputs_h(DWORD attr,  wchar_t* pstr,HANDLE hout=HAC)
{
	bool fcalign=(M_AC&attr);
	attr&=(~M_AC);
//  _auto_stdcall<HANDLE,BOOL,&CloseHandle> sH;
//  if(hout==HAC) 
//	    hout=sH.attach(get_console_active_buffer());
   DWORD ccw=0;
   short o_attr=  console_info(hout).wAttributes;
   if(M_FC&attr)
	   attr=(0xFFFF00F0&attr)|(o_attr&0x0F);
   if(M_BC&attr)
	   attr=(0xFFFF000F&attr)|(o_attr&0xF0);
   
   long ll=wcslen(pstr);

   if(fcalign)
   {
	   coord c=consoleXY(hout);
       coord d=console_info(hout).dwSize;
	   COORD cln={0,c.Y};
	   DWORD tt;
	   FillConsoleOutputCharacter(hout,L' ',d.X,cln,&tt);

	   double dd=(d.X-ll)/2.0;
	   if(dd<0) dd=0;
	       consoleXY cc(hout);
		   cc=coord(dd,c.Y); 

   }
   SetConsoleTextAttribute(hout,0x0000FFFF&attr);
   WriteConsoleW(hout,pstr,ll,&ccw,0);
   SetConsoleTextAttribute(hout,o_attr);
  return ccw;
}



/*
clsln(HANDLE hout=HAC)
{
	_auto_stdcall<HANDLE,BOOL,&CloseHandle> sH;
	if(hout==HAC) 
		hout=sH.attach(get_console_active_buffer());
    FillConsoleOutputCharacter()

}
*/
inline _bstr_t bgets(int attr=M_FC|M_BC,HANDLE hecho=INVALID_HANDLE_VALUE)
{
	_bstr_t rstr;
	wchar_t c[2]={0,0};
	while(L'\r'!=(c[0]=_getwch()))
	{
		if( hecho!=INVALID_HANDLE_VALUE) conputs_h(attr,c,hecho);        
		rstr+=c;
	}
	return rstr;
}


inline bstr_t vbstr_printf(bstr_t fmt, va_list argptr)
{
	_bstr_t res;
	int cb=_vscwprintf(fmt,argptr);
	BSTR pbuf=SysAllocStringLen(NULL,cb+1);
	res.Attach(pbuf);
	vswprintf_s(pbuf,cb+1, fmt, argptr);
	return (wchar_t*)res; 
}
inline bstr_t bstr_printf(bstr_t fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	return vbstr_printf(fmt,argptr); 
}


inline void conprintf_h(HANDLE hout,DWORD attr,bstr_t fmt,...)
{
	va_list argptr;
	va_start(argptr, fmt);
	conputs_h(attr,vbstr_printf(fmt,argptr),hout); 
}

inline void conprintf(short attr,bstr_t fmt,...)
{

	va_list argptr;
	va_start(argptr, fmt);
	conputs(attr,vbstr_printf(fmt,argptr)); 
}
//inline _bstr_t vbprintf(_bstr_t fmt, va_list argptr)