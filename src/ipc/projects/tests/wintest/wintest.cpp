// wintest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "wintest.h"
#include "ipc_ports/dispatch_call_helpers.h"
#include "mshtmhst.h"
#include <vector>
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

ipc_utils::COMInitA_t ci;

int resid[256];
int inn=0;
BOOL CALLBACK EnumResNameProc(          HMODULE hModule,
							  LPCTSTR lpszType,
							  LPTSTR lpszName,
							  LONG_PTR lParam
							  )
{

	if(IS_INTRESOURCE(lpszName))
		resid[inn++]=int(lpszName);
	//...

	return true;
};

struct resources_t
{

	void enumproc( wchar_t* resname )
	{
		if(IS_INTRESOURCE(resname))
			resid.push_back(int(resname));

	}


static	BOOL CALLBACK EnumResNameProc(          HMODULE hModule,
		LPCTSTR lpszType,
		LPTSTR lpszName,
		LONG_PTR lParam
		)
{
	((resources_t*)lParam)->enumproc( lpszName );

		return true;
};

	resources_t(HMODULE hlib){
		
		resid.reserve(1024);
		bool f=EnumResourceNamesW(hlib,RT_HTML,&EnumResNameProc,LONG_PTR(this));
		if(!f) {hr=HRESULT_FROM_WIN32(GetLastError());return;};


		
	}
std::vector<int> resid;
HRESULT hr;
};



int testResEnum()
{
 bool f;
	HINSTANCE hlib=LoadLibraryA("ltx_js.dll");
	f=EnumResourceNamesW(hlib,RT_HTML,&EnumResNameProc,0);

	return inn;

}

IDispatchEx * pdiex;
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

	 SYSTEM_LOGICAL_PROCESSOR_INFORMATION slpi;
	 DWORD ls=sizeof(slpi);
	 SYSTEM_INFO si;
	 GetSystemInfo(&si);

	GetLogicalProcessorInformation(&slpi,&ls);
	testResEnum();
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINTEST));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINTEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINTEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
inline void test_script(int ihwnd)
{
	int flags=HTMLDLG_MODELESS | HTMLDLG_ALLOW_UNKNOWN_THREAD | HTMLDLG_VERIFY;
	//
	ipc_utils::com_scriptor_t scriptcaller(L"ltx.bind:script:debug=3 ;");
	//	scriptcaller(L"webform(arguments[0],'',0,arguments[1],arguments[2])")(L"http://localhost:7777/::ltxsilk=::/Windows%20Desktop%20Feeling%20for%20Websites/script/w7d.htm_hssh",ihwnd,flags);
	//variant_t res=scriptcaller(L"editor({hwnd:$$[0],file:$$[1],modal:0})")(ihwnd,L"s:/scripts/RPC.js");
	
	//	
	scriptcaller(L"webform(arguments[0],'',0,arguments[1],arguments[2])")(L"http://localhost:7777/::ltxsilk=::/windows_js_1.3/samples/inframe.htm_hssh",ihwnd,flags);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			test_script(int(hWnd));
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
