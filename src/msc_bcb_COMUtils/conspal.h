#pragma once

#define WIN32_LEAN_AND_MEAN // this will assume smaller exe... NOT ...
#define  _WIN32_WINNT 0x0500

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>

//**************************************************************
//  dos.pal
// 000   00 00 00 00 00 2A 00 2A  00 00 2A 2A 2A 00 00 2A   
// 010   00 2A 2A 15 00 2A 2A 2A  15 15 15 15 15 3F 15 3F   
// 020   15 15 3F 3F 3F 15 15 3F  15 3F 3F 3F 15 3F 3F 3F   
// 030   81 C7 D9
//**************************************************************
/*
COLORREF curr_attr[16] ;
static TCHAR tempstr[81] ;

static unsigned default_attr[16] = {
	0x00000000, 0x00800000, 0x00008000, 0x00808000,
	0x00000080, 0x00800080, 0x00008080, 0x00C0C0C0,
	0x00808080, 0x00FF0000, 0x0000FF00, 0x00FFFF00,
	0x000000FF, 0x00FF00FF, 0x0000FFFF, 0x00FFFFFF } ;

	unsigned default_attr2[16] = {
		0x11111000, 0x00811100, 0x00008000, 0x00808000,
		0x00222220, 0x03333380, 0x0333323, 0x0eCeCeCe,
		0x0ee0e0e0, 0x00F40404, 0x0044FF00, 0x00F44F00,
		0x088880FF, 0x555555FF, 0x0444444F, 0x00F444FF } ;

		//  undocumented console font info

		unsigned default_attr3[16] = {0x00404000
			,0x00800000
			,0x00008000
			,0x00808000
			,0x00000080
			,0x00800080
			,0x00008080
			,0x00dcfaff
			,0x00808080
			,0x00ff0000
			,0x0000ff00
			,0x00ffff00
			,0x000000ff
			,0x00ff00ff
			,0x0000ffff
			,0x00feffff};
*/

#define WM_SETCONSOLEINFO		(WM_USER+201)
struct cons_pall_xp
{

	enum
	{
       
	};
#pragma pack(push, 1)
	typedef struct _CONSOLE_INFO_inn
	{
		ULONG    Length;
		COORD    ScreenBufferSize;
		COORD    WindowSize;
		ULONG    WindowPosX;
		ULONG    WindowPosY;

		COORD    FontSize;
		ULONG    FontFamily;
		ULONG    FontWeight;
		WCHAR    FaceName[32];

		ULONG    CursorSize;
		ULONG    FullScreen;
		ULONG    QuickEdit;
		ULONG    AutoPosition;
		ULONG    InsertMode;

		USHORT   ScreenColors;
		USHORT   PopupColors;
		ULONG    HistoryNoDup;
		ULONG    HistoryBufferSize;
		ULONG    NumberOfHistoryBuffers;

		COLORREF ColorTable[16];

		ULONG    CodePage;
		HWND     Hwnd;

		WCHAR    ConsoleTitle[0x100];

	} CONSOLE_INFO;
#pragma pack(pop)



	static bool GetConsoleSizeInfo(CONSOLE_INFO *pci)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

		bool f=GetConsoleScreenBufferInfo(hConsoleOut, &csbi);
		if(f)
		{
			pci->ScreenBufferSize = csbi.dwSize;
			pci->WindowSize.X     = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			pci->WindowSize.Y     = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			pci->WindowPosX       = csbi.srWindow.Left;
			pci->WindowPosY       = csbi.srWindow.Top;


		}
		return f;

	}
	
	static bool GetConsoleFontInfo(CONSOLE_INFO *pci)
	{
		CONSOLE_FONT_INFO cfi ;
		COORD fsize ;
		HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
 
	bool f=	GetCurrentConsoleFont(hConsoleOut, FALSE, &cfi) ;
	if(f)
	{
		fsize = GetConsoleFontSize(hConsoleOut, cfi.nFont) ;

		pci->FontSize.X = fsize.X ;
		pci->FontSize.Y = fsize.Y ;

		// wsprintf(tempstr, "Font Size= X%u Y%u\n", fsize.X, fsize.Y) ;
		// OutputDebugString(tempstr) ; 
		// set these to zero to keep current settings 
		// pci->FontFamily = 0x30;//FF_MODERN|FIXED_PITCH;//0x30;
		// pci->FontWeight = 0x400;
		// lstrcpyW(pci->FaceName, L"Terminal");
		//
		//
		lstrcpyW(pci->FaceName, L"Lucida Console");
		pci->FontFamily = 0;//0x30;//FF_MODERN|FIXED_PITCH;//0x30;
		pci->FontWeight = 0;//0x400;
		//pci->FaceName[0]          = L'\0';
	}
	 return f;
	}

static	BOOL SetConsoleInfo(HWND hwndConsole, CONSOLE_INFO *pci)
	{
		DWORD   dwConsoleOwnerPid;
		HANDLE  hProcess;
		HANDLE	hSection, hDupSection;
		PVOID   ptrView = 0;
		HANDLE  hThread;

		//
		//	Open the process which "owns" the console
		//	
		GetWindowThreadProcessId(hwndConsole, &dwConsoleOwnerPid);

		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwConsoleOwnerPid);
		if (hProcess == NULL)
		{
			return FALSE;
		}

		//
		// Create a SECTION object backed by page-file, then map a view of
		// this section into the owner process so we can write the contents 
		// of the CONSOLE_INFO buffer into it
		//
		hSection = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, pci->Length, 0);
		if (hSection == NULL) {
			
			return FALSE;
		}

		//
		//	Copy our console structure into the section-object
		//
		ptrView = MapViewOfFile(hSection, FILE_MAP_WRITE|FILE_MAP_READ, 0, 0, pci->Length);
		if (ptrView == NULL) {
			
			return FALSE;
		}

		memcpy(ptrView, pci, pci->Length);

		if (!UnmapViewOfFile(ptrView)) {
			
			return FALSE;
		}

		//
		//	Map the memory into owner process
		//
		if (!DuplicateHandle(GetCurrentProcess(), hSection, hProcess, &hDupSection, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
			
			return FALSE;
		}

		//  Send console window the "update" message
		SendMessage(hwndConsole, WM_SETCONSOLEINFO, (WPARAM)hDupSection, 0);

		//
		// clean up
		//
		hThread = CreateRemoteThread(hProcess, 0, 0, 
			(LPTHREAD_START_ROUTINE)CloseHandle, hDupSection, 0, 0);
		if (hThread == NULL) {
			
			return FALSE;
		}

		CloseHandle(hThread);
		CloseHandle(hSection);
		CloseHandle(hProcess);

		return TRUE;
	};



static	bool SetConsolePalette(COLORREF* palette,WORD sc=MAKEWORD(0x7, 0x0),WORD pc=MAKEWORD(0x5, 0xf))
	{
		CONSOLE_INFO ci = { sizeof(ci) };
		int i;
		HWND hwndConsole = GetConsoleWindow();

		// get current size/position settings rather than using defaults..
		if(!GetConsoleSizeInfo(&ci)) return false;

		if(!GetConsoleFontInfo(&ci)) return false;;

		// set these to zero to keep current settings 
		ci.CursorSize           = 25;
		ci.FullScreen           = FALSE;
		ci.QuickEdit            = TRUE;
		ci.AutoPosition         = 0x10000;
		ci.InsertMode           = TRUE;
		ci.ScreenColors         = sc;
		ci.PopupColors          = pc;

		ci.HistoryNoDup         = FALSE;
		ci.HistoryBufferSize    = 50;
		ci.NumberOfHistoryBuffers  = 4;

		// colour table
		for(i = 0; i < 16; i++)
			ci.ColorTable[i] = palette[i];

		ci.CodePage             = 0;//0x352;
		ci.Hwnd						= hwndConsole;

		ci.CodePage=GetConsoleCP();
		CONSOLE_CURSOR_INFO cci;
		if(GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cci))
		{
			ci.CursorSize=cci.dwSize;
		};


		lstrcpyW(ci.ConsoleTitle, L"");

		return SetConsoleInfo(hwndConsole, &ci);
	}

  cons_pall_xp(COLORREF* palette=0,WORD sc=MAKEWORD(0x7, 0x0),WORD pc=MAKEWORD(0x5, 0xf))
  {
	  unsigned default_attr3[16] = {0x00404000
		  ,0x00800000
		  ,0x00008000
		  ,0x00808000
		  ,0x00000080
		  ,0x00800080
		  ,0x00008080
		  ,0x00dcfaff
		  ,0x00808080
		  ,0x00ff0000
		  ,0x0000ff00
		  ,0x00ffff00
		  ,0x000000ff
		  ,0x00ff00ff
		  ,0x0000ffff
		  ,0x00feffff};

       if(palette==0)  palette=(COLORREF*)default_attr3;
  
	  SetConsolePalette(palette,sc,pc);             
  }
};
