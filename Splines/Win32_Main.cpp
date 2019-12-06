//	----==== WIN32_MAIN.CPP ====----
//
//	Author:			Jeff Kiah
//					y2kiah@hotmail.com
//	Version:		2
//	Date:			6/04
//	Description:	This is the Win32 setup code to create a window that will use OpenGL.				
//	-------------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN		// this keeps MFC (Microsoft Foundation Classes) from being included

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "win32_main.h"
#include "utilitycode/keyboardmanager.h"
#include "utilitycode/screenmanager.h"
#include "utilitycode/mousemanager.h"
#include "utilitycode/lookupmanager.h"
#include "utilitycode/timer.h"

#include "bsplinetest.h"

// temp until font manager
#include "utilitycode/glfont.h"


/*-----------------
---- VARIABLES ----
-----------------*/

HINSTANCE		hInst;			// Instance handle
MSG				msg;			// Windows message
HWND			hWnd;			// Window handle
HDC				hDC;			// Device context
HGLRC			hRC;			// Render context

bool			active;			// Whether the window created is the currently active window
const char		APP_NAME[] = "B-Spline Demo";	// Name of the application

// temp until font manager up and running
GLFont			*font;


/*-----------------
---- FUNCTIONS ----
-----------------*/


//-------------------------------------------------------------------------------------------
//	The purpose of this function is to handle Windows messages and to give the WinMain loop
//	an exit strategy. It will return true if the application should quit and false otherwise.
//-------------------------------------------------------------------------------------------
bool handleMsg(void)
{
	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
		if (msg.message == WM_QUIT) return true;
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return false;
}


//-------------------------------------------------------------------------------------------
//	The WndProc function is used to handle windows message processing. The special type
//	LRESULT CALLBACK is not a return type but a special function indicating that this
//	function will be repeatedly be called at any point determined by the operating system.
//	The programmer never actually makes a call to this function.
//-------------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_CREATE:
			break;

		case WM_DESTROY:
		case WM_CLOSE:
			// Post the WM_QUIT message to quit the application
			PostQuitMessage(0); 
			break;

		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE || HIWORD(wParam) != 0) {	// inactive or minimized
				active = false;
			} else if (!active && HIWORD(wParam) == 0) {	// wasn't active and isn't minimized
				active = true;
			}
			break;

		case WM_SYSCOMMAND:
			switch (wParam) {
				case SC_SCREENSAVE:
					break;
				case SC_MONITORPOWER:
					break;
				case SC_SIZE:
					break;
				default:
					return DefWindowProc(hwnd,msg,wParam,lParam);
			}
			break;
	
		case WM_PAINT:
			break;
		case WM_SIZE:
			break;

		default:
			// this function call makes sure every windows message that is not processed above
			// will be processed in the default manner
			return DefWindowProc(hwnd,msg,wParam,lParam);
	}

	return 0;
}


void killWindow(void)
{
	if (screen.getFullScreen()) {
		ChangeDisplaySettings(NULL,0);
		ShowCursor(TRUE);
	}

	if (hRC) {
		if (!wglMakeCurrent(NULL,NULL))
			MessageBox(0,"Release of DC and RC failed","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);

		if (!wglDeleteContext(hRC))
			MessageBox(0,"Release rendering context failed","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);

		hRC = NULL;
	}

	if (hDC && !ReleaseDC(hWnd,hDC)) {
		MessageBox(0,"Release device context failed","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		hDC = NULL;
	}

	if (hWnd && !DestroyWindow(hWnd))
	{
		MessageBox(NULL,"Could not release hWnd","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		hWnd = NULL;
	}

	if (!UnregisterClass("WNDCLASS1",hInst)) {
		MessageBox(NULL,"Could not unregister class","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		hInst = NULL;
	}
}


bool initWindow(void)
{
	WNDCLASSEX	wc;
	DWORD		dwExStyle;
	DWORD		dwStyle;
	UINT		pixelFormat;
	
	hInst = GetModuleHandle(NULL);

    wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "WNDCLASS1";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(0,"Failed to register the window class","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	if (screen.getFullScreen()) {

		DEVMODE dmScreenSettings;

		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));		
		dmScreenSettings.dmSize	= sizeof(DEVMODE);
		dmScreenSettings.dmPelsWidth = screen.getResX();
		dmScreenSettings.dmPelsHeight = screen.getResY();
		dmScreenSettings.dmBitsPerPel = screen.getColorDepth();
		dmScreenSettings.dmDisplayFrequency = screen.getRefresh();
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
		int check = ChangeDisplaySettings(&dmScreenSettings,CDS_TEST);
		
		switch (check) {
			case DISP_CHANGE_SUCCESSFUL:
				ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN);
				break;

			case DISP_CHANGE_FAILED:
				killWindow();
				MessageBox(0,"Failed to change to desired settings","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
				return false;

			case DISP_CHANGE_BADMODE:
				killWindow();
				MessageBox(0,"Fullscreen video mode not supported","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
				return false;

			case DISP_CHANGE_RESTART:
				killWindow();
				MessageBox(0,"Must restart to get fullscreen video mode","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
				return false;
		}
	}

	if (screen.getFullScreen()) {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
		dwStyle = WS_POPUP;
	} else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW;
		dwStyle = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX;
	}

	hWnd = CreateWindowEx(dwExStyle,"WNDCLASS1",APP_NAME,dwStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,0,0,screen.getResX(),screen.getResY(),NULL,NULL,hInst,NULL);
	if (!hWnd) {
		killWindow();
		MessageBox(0,"Window creation error","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	// The Pixel Format Descriptor structure is used to specify the display settings that
	// you would like. If the requested display setting is not available the application will
	// quit with an appropriate message. It is possible to find all available display settings
	// using the EnumDisplaySettings function. This would be useful if you wanted to give
	// users a list to choose from in a menu, or if you wanted to choose the closest possible
	// display setting to the requested rather than just quitting.
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		screen.getColorDepth(),
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		screen.getZDepth(),
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	hDC = GetDC(hWnd);
	if (!hDC) {
		killWindow();
		MessageBox(NULL,"Cannot create a device context","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (!pixelFormat) {
		killWindow();
		MessageBox(0,"Cannot find suitable pixel format","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	if (!SetPixelFormat(hDC, pixelFormat, &pfd)) {
		killWindow();
		MessageBox(0,"Cannot set the pixel format","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	hRC = wglCreateContext(hDC);
	if (!hRC) {
		killWindow();
		MessageBox(0,"Cannot create an OpenGL rendering context","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	if (!wglMakeCurrent(hDC, hRC)) {
		killWindow();
		MessageBox(0,"Cannot activate OpenGL rendering context","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	wglMakeCurrent(hDC, hRC);

	return true;
}


void initOpenGL(void)
{
	///// Init OpenGL stuff /////

	glViewport(0,0,screen.getResX(),screen.getResY());
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-10,10,-10,10,1,100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);
	glClearColor(1,1,1,1);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // This is set up for the splat textures
	glAlphaFunc(GL_NOTEQUAL,0);

	if (screen.getNiceGraphics()) { 
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
		glHint(GL_FOG_HINT, GL_NICEST);
	} else {
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT,GL_FASTEST);
		glHint(GL_FOG_HINT, GL_FASTEST);
	}

	// Set up what to use
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_NORMALIZE);
	glDisable(GL_DITHER);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_LOGIC_OP);

	// Set drawing options
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
}


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int iCmdShow)
{
	// Allocate the singleton classes in the correct order
	Timer				timerInst;
	ScreenManager		screenInst(640,480,75,32,16,true,false);
	MouseManager		mouseInst(screen.getResX(),screen.getResY());
	KeyboardManager		kbInst;
	LookupManager		lookupInst(80);		// precision of 1/80th of a degree is good for 3D games

	// Open a new window for OpenGL drawing
	if (!initWindow()) return -1;

	// Set up the timer
	timer.initTimer();

	// Set up the mouse
	mouse.switchMode(MMODE_FREE);
	mouse.setSensitivity(1.0f);

	// Load a font

	// Set up OpenGL options before using
	initOpenGL();

	// set up scene
	setupControlPoints();

	// set up font TEMP TEMP TEMP
	font = new GLFont(screen.getResY());
	font->loadTrueTypeFont("Courier New",14,0,0,0,0);

	// Start the message loop
	while (1) {
		if (handleMsg()) break;		// Handle windows messages, quit on true return

		timer.calcTimeFix();

		// If the window is not active it will still continue to handle messages
		// but will not execute any application functions
		if (active) {
			// press ESC to close the app
			if (kb.keyDown(VK_ESCAPE)) SendMessage(hWnd,WM_CLOSE,0,0);

			// get some keyboard input
			if (kb.buttonPressed(VK_RETURN)) setupControlPoints();

			// This is where you make a call to execute your game functions
			renderScene();

			// Get frames per second
			timer.calcFPS();			
		}
	}

	// delete font TEMP TMEP TEMP
	delete font;

	killWindow();

	return msg.wParam;
}
