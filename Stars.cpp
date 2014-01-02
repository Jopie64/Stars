// Stars.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Stars.h"
#include "StarsWnd.h"
#include "JOpenGl.h"
#include <sstream>

#define MAX_LOADSTRING 100

using namespace JStd::Wnd;
using namespace std;
using namespace JStd::GL;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
PWindow				InitInstance(HINSTANCE, int);
PGlWnd 				InitInstanceGl(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_STARS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	JStd::GL::PGlWnd pwnd;
	try
	{
		pwnd = InitInstanceGl(hInstance, nCmdShow);
	}
	catch (exception& e)
	{
		wostringstream os;
		os << L"Unable to initialize" << endl << e.what();
		MessageBox(NULL, os.str().c_str(), L"Initialization failed.", MB_ICONERROR);
		return FALSE;
	}

	CStarsWnd W_Stars(pwnd);
	W_Stars.InitAndRun();



	// Main message loop:
	float theta = 0.0f;
	return JStd::Wnd::RunLoop(LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_STARS)),
		[&]()
	{
		W_Stars.RenderFrame();

		pwnd->SwapBuffers();

		theta += 0.1f;
	});
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STARS));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_STARS);
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
PWindow InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
	JStd::Wnd::WndInit init;
	init.ClassName = szWindowClass;
	init.WindowName = szTitle;
	init.dwStyle = WS_OVERLAPPEDWINDOW;
	init.hInstance = hInstance;
	PWindow PWnd = Wnd::Create(init);

	if (!PWnd)
	{
		return FALSE;
	}

	ShowWindow(PWnd->H(), nCmdShow);
	UpdateWindow(PWnd->H());

	return PWnd;
}

PGlWnd InitInstanceGl(HINSTANCE hInstance, int nCmdShow)
{
	using namespace JStd::GL;
	hInst = hInstance; // Store instance handle in our global variable
	JStd::GL::WndInit init;
	init.WindowName = szTitle;
	init.dwStyle |= WS_OVERLAPPEDWINDOW;
	init.hInstance = hInstance;
	PGlWnd PWnd = CreateGlWindow(init);

	if (!PWnd)
	{
		return FALSE;
	}

	ShowWindow(PWnd->H(), nCmdShow);
	UpdateWindow(PWnd->H());

	return PWnd;
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
	int wmId, wmEvent;
//	PAINTSTRUCT ps;
//	HDC hdc;

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
		}
		break;
//	case WM_PAINT:
//		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
//		EndPaint(hWnd, &ps);
//		break;
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
