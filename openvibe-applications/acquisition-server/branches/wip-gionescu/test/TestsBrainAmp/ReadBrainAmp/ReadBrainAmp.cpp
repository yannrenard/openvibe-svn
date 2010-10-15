// ReadBrainAmp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <vector>
#include "ReadBrainAmp.h"

#define MAX_LOADSTRING	100
#define MAX_THREADS		2
#define BUF_SIZE		255

typedef enum
{	MESS_DRAW_INFO	= WM_USER,
	MESS_DRAW_ERROR,
} mess_type;

typedef struct MyData {
    int				index;
	ParamsClient*	params;
} MYDATA, *PMYDATA;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM						MyRegisterClass(HINSTANCE hInstance);
BOOL						InitInstance(HINSTANCE, int);
LRESULT CALLBACK			WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK			About(HWND, UINT, WPARAM, LPARAM);


std::string					myCmdLine;
HWND						myWnd;
WORD						myX;
WORD						myY;
ParamsBrainAmpClient		myParamsClient;
std::ostringstream			myInfo;

void ErrorHandler(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code.

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message.

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR) lpMsgBuf) + lstrlen((LPCTSTR) lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR) lpDisplayBuf, TEXT("Error"), MB_OK); 

	// Free error-handling buffer allocations.

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

bool Config()
{
	if(myCmdLine.length() == 0)
	{	myParamsClient.SetParams("localhost 700 32");
	}
	else
	{	std::istringstream	iss(myCmdLine);
		std::string			host, hostPort, chankSize, sendWait;
		iss	>> host >> hostPort >> chankSize >> sendWait;

		myInfo	<< "Parameters"				<< std::endl	<< std::endl
				<< "	host       = "		<< host			<< std::endl
				<< "	hostPort   = "		<< hostPort		<< std::endl 
				<< "	chankSize  = "		<< chankSize	<< std::endl
				<< "	sendWait   = "		<< sendWait		<< std::endl;

		::WndProc(myWnd, MESS_DRAW_INFO, 0, 0);

		std::ostringstream	oss;
		oss << host << ' ' << hostPort << ' ' << chankSize << ' ' << sendWait;
		myParamsClient.SetParams(oss.str());
	}

	return true;
}

void StartClient()
{
	myParamsClient.StartThreads(myWnd, myX, myY);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR    lpCmdLine,
                     int       nCmdShow)
{
	myCmdLine	= lpCmdLine;
	myCmdLine   += "   ";

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_READBRAINAMP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_READBRAINAMP));

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_READBRAINAMP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_READBRAINAMP);
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
   hInst = hInstance; // Store instance handle in our global variable

   myWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!myWnd)
   {
      return FALSE;
   }

   ShowWindow(myWnd, nCmdShow);
   UpdateWindow(myWnd);
   ShowWindow(myWnd, SW_MAXIMIZE);

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int			wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC			hdc;
	static	int	first = 0;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		case IDM_CONFIG_LOCAL:
		case IDM_CONFIG_SERVER:
			Config();
			break;
		case IDM_START:
			StartClient();
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		if(first++ == 1)
			Config();
		break;
	case MESS_DRAW_INFO:
	case MESS_DRAW_ERROR:
		{	wchar_t buff[5000];
			if(MultiByteToWideChar(CP_ACP, 0, myInfo.str().c_str(), myInfo.str().length()+1, buff, myInfo.str().length()+1) != 0)
			{	hdc = GetDC(hWnd);	
				
				RECT rect = { 0, 0, 5000, 5000 };
			
				DrawText(hdc, buff, myInfo.str().length(), &rect, DT_WORDBREAK);

				ReleaseDC(hWnd, hdc);
		}	}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		myX	= LOWORD(lParam);
		myY	= HIWORD(lParam);
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
