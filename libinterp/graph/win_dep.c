

#ifndef WINVER                  // Specifies that the minimum required platform is Windows 7.
#define WINVER 0x0601           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows 7.
#define _WIN32_WINNT 0x0601     // Change this to the appropriate value to target other versions of Windows.
#endif     

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif


#include	<windows.h>
//#include <windows.h>    // included for Windows Touch
#include <windowsx.h>   // included for point conversion



#include <lib9.h>
#include <isa.h>
#include <interp.h>

#include "../e.h"
//#include "../emod.h"

#undef Unknown
#include "../../emu/port/dat.h"
//#include "../emu/port/fns.h"
#include "kernel.h"

#include "graph.h"

#include "cairo-features.h"
#include <cairo.h>
#include <svg-cairo.h>

#define _USE_MATH_DEFINES
#include <math.h>


graphic_el* groot = NULL;

//graphic_el* free_levels = NULL;


//{} Win test screen VV



static	HINSTANCE	inst;
static	HINSTANCE	previnst;
static	int		cmdshow;
static	HWND		window;
static	HDC		screen;
static	HPALETTE	palette;

static	HCURSOR		hcursor;


ulong	*graph_scrn_data = NULL;


static	int		maxxsize = 1024;
static	int		maxysize = 800;

int	Xsize_;
int	Ysize_;


// You will use this array to switch the color / track ids
int idLookup[MAXPOINTS];

//int points[MAXPOINTS][2];


void graph_scrn_init()
{
	for (int i = 0; i < MAXPOINTS; i++) {
		idLookup[i] = -1;
	}
}



// This function is used to return an index given an ID
static int GetContactIndex(int dwID){
  for (int i = 0; i < MAXPOINTS; i++) {
      if (idLookup[i] == dwID) {
          return i;
      }
  }

  for (int i = 0; i < MAXPOINTS; i++) {
      if (idLookup[i] == -1) {
          idLookup[i] = dwID;
          return i;
      }
  }
  // Out of contacts
  return -1;
}

// Mark the specified index as initialized for new use
static BOOL RemoveContactIndex(int index) {
    if (index >= 0 && index < MAXPOINTS) {
        idLookup[index] = -1;
        return TRUE;
    }

    return FALSE;
}


static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	PAINTSTRUCT paint;
	HDC hdc;
	LPMINMAXINFO mmi;
	LONG x, y, w, h, b;
	HCURSOR dcurs;
	POINT m;

	char* m_op_s = "";

	b = 0;

	switch(msg) {
	case WM_SETCURSOR:
		/* User set */
		if(hcursor != NULL) {
			SetCursor(hcursor);
			break;
		}
		/* Pick the default */
		dcurs = LoadCursor(NULL, IDC_ARROW);
		SetCursor(dcurs);
		break;

	case WM_TOUCH:
		m_op_s = "M_TOUCH";
		{
		    int bHandled = FALSE;
		    UINT cInputs = LOWORD(wparam);
		    TOUCHINPUT pInputs[MAXPOINTS];

			if (cInputs > MAXPOINTS)
				cInputs = MAXPOINTS;

			//Array* ar = H2D(E_XY_coord*, heaparray(TXY_coord, MAXPOINTS));
			E_XY_coord tp[MAXPOINTS]; // = (E_XY_coord*)ar->graph_scrn_data;

			for (int i = 0; i < MAXPOINTS; i++) {
				tp[i].x = -1;
				tp[i].y = -1;
			}

			if (GetTouchInputInfo((HTOUCHINPUT)lparam, cInputs, pInputs, sizeof(TOUCHINPUT))){
				for (UINT i = 0; i < cInputs; i++){
					POINT ptInput;
					TOUCHINPUT ti = pInputs[i];

					int index = GetContactIndex(ti.dwID);
					if (ti.dwID != 0 && index < MAXPOINTS){
						ptInput.x = TOUCH_COORD_TO_PIXEL(ti.x);
						ptInput.y = TOUCH_COORD_TO_PIXEL(ti.y);
						ScreenToClient(hwnd, &ptInput);

						if (ti.dwFlags & TOUCHEVENTF_UP){
							tp[index].x = -1;
							tp[index].y = -1;
							// Remove the old contact index to make it available for the new incremented dwID.
							// On some touch devices, the dwID value is continuously incremented.
							RemoveContactIndex(index);				  
						} else {
							tp[index].x = ptInput.x;
							tp[index].y = ptInput.y;
						}
					}
				}            
				bHandled = TRUE;
			} else {
				/* handle the error here */
			}

			if (bHandled){
				// if you handled the message, close the touch input handle and return
				CloseTouchInputHandle((HTOUCHINPUT)lparam);

				graphic_el* el = NULL;

				for (int i = 0; i < MAXPOINTS; i++) {
					el = findGraphElByXY((double)tp[i].x, (double)tp[i].y);
					if (el != NULL && el != H)
						break;
				}

				if (el) {
					graphic_el* el_par = el->root;
					double dx = el->x, dy = el->y;

					for (; el_par; el_par = el_par->root) {
						dx += el_par->x;
						dy += el_par->y;
					}

					csend_touch_evt_(el, tp);
				}
		        break;
			}else{
		        // if you didn't handle the message, let DefWindowProc handle it
//		        return DefWindowProc(hwnd, WM_TOUCH, wparam, lparam);
		        break;
			}
		}

	case WM_MOUSEWHEEL:
		if ((int)wparam > 0) {
			b = 1 << M_WHEEL_FWD; //8;
			m_op_s = "M_WHEEL_FWD";
		} else {
			b = 1 << M_WHEEL_BkWD; //16;
			m_op_s = "M_WHEEL_BkWD";
		}
		m.x = LOWORD(lparam);
		m.y = HIWORD(lparam);
		ScreenToClient(hwnd, &m);
		goto mok;

	case WM_LBUTTONDBLCLK:
		b = 1 << M_LB_DBLCLK; //(1<<8) | 1;
		m_op_s = "M_LB_DBLCLK";
		goto process;
	case WM_MBUTTONDBLCLK:
		b = 1 << M_MB_DBLCLK; //(1<<8) | 2;
		m_op_s = "M_MB_DBLCLK";
		goto process;
	case WM_RBUTTONDBLCLK:
		b = 1 << M_RB_DBLCLK; //(1<<8) | 4;
		m_op_s = "M_RB_DBLCLK";
		goto process;

	case WM_MOUSEMOVE:
		b = 1 << M_MOVE;
		m_op_s = "M_MOVE";
		goto process;
	case WM_LBUTTONUP:
		b = 1 << M_LB_UP;
		m_op_s = "M_LB_UP";
		goto process;
	case WM_MBUTTONUP:
		b = 1 << M_MB_UP;
		m_op_s = "M_MB_UP";
		goto process;
	case WM_RBUTTONUP:
		b = 1 << M_RB_UP;
		m_op_s = "M_RB_UP";
		goto process;
	case WM_LBUTTONDOWN:
		b = 1 << M_LB_DWN;
		m_op_s = "M_LB_DWN";
		goto process;
	case WM_MBUTTONDOWN:
		b = 1 << M_MB_DWN;
		m_op_s = "M_MB_DWN";
		goto process;
	case WM_RBUTTONDOWN:
		b = 1 << M_RB_DWN;
		m_op_s = "M_RB_DWN";
		goto process;
//		b = 0;
	process:
		m.x = LOWORD(lparam);
		m.y = HIWORD(lparam);
	mok:
		if(wparam & MK_LBUTTON)
			b |= 1 << M_MK_LB;
		if(wparam & MK_MBUTTON)
			b |= 1 << M_MK_MB;
		if(wparam & MK_RBUTTON) {
			if(wparam & MK_CONTROL)
				b |= 1 << M_MK_MB;  //simulate middle button
			else
				b |= 1 << M_MK_RB;  //right button
		}
//		mousetrack(b, m.x, m.y, 0);

		{
		graphic_el* el = findGraphElByXY((double)m.x, (double)m.y);

		if (el) {
			graphic_el* el_par = el->root;
			double dx = el->x, dy = el->y;

			for (; el_par; el_par = el_par->root) {
				dx += el_par->x;
				dy += el_par->y;
			}

//			printfToGraphEl(el, ":MOUSE: XY: l:(%f, %f) g:(%d, %d) op: %s (0x%X)", 
//				(double)m.x - dx, (double)m.y - dy,
//				(int)m.x, (int)m.y, 
//				m_op_s, (uint)b);

			csend_mouse_evt_(el, m_op_s, (double)m.x - dx, (double)m.y - dy,  (double)m.x, (double)m.y);
			
		}
		}
		break;

#if 0
	case WM_SYSKEYDOWN:
		if(gkscanq)
			scancode(wparam, lparam, 0);
		break;
	case WM_SYSKEYUP:
		if(gkscanq)
			scancode(wparam, lparam, 1);
		else if(wparam == VK_MENU)
			gkbdputc(gkbdq, Latin);
		break;
	case WM_KEYDOWN:
		if(gkscanq) {
			scancode(wparam, lparam, 0);
			break;
		}
		switch(wparam) {
		default:
			return 0;
		case VK_HOME:
			wparam = Home;
			break;
		case VK_END:
			wparam = End;
			break;
		case VK_UP:
			wparam = Up;
			break;
		case VK_DOWN:
			wparam = Down;
			break;
		case VK_LEFT:
			wparam = Left;
			break;
		case VK_RIGHT:
			wparam = Right;
			break;
		case VK_PRIOR:	/* VK_PAGE_UP */
			wparam = Pgup;
			break;
		case VK_NEXT:		/* VK_PAGE_DOWN */
			wparam = Pgdown;
			break;
		case VK_PRINT:
			wparam = Print;
			break;
		case VK_SCROLL:
			wparam = Scroll;
			break;
		case VK_PAUSE:
			wparam = Pause;
			break;
		case VK_INSERT:
			wparam = Ins;
			break;
		case VK_DELETE:
			wparam = Del;
			break;
/*
		case VK_TAB:
			if(GetKeyState(VK_SHIFT)<0)
				wparam = BackTab;
			else
				wparam = '\t';
			break;
*/
		}
		gkbdputc(gkbdq, wparam);
		break;
	case WM_KEYUP:
		if(gkscanq)
			scancode(wparam, lparam, 1);
		break;
	case WM_CHAR:
		if(gkscanq)
			break;
		switch(wparam) {
		case '\n':
		  	wparam = '\r';
		  	break;
		case '\r':
		  	wparam = '\n';
		  	break;
		case '\t':
			if(GetKeyState(VK_SHIFT)<0)
				wparam = BackTab;
			else
				wparam = '\t';
			break;
		}
		if(lparam & KF_ALTDOWN) 
		    	wparam = APP | (wparam & 0xFF);
		gkbdputc(gkbdq, wparam);
		break;
#endif
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
//		cleanexit(0);
		break;
#if 0
	case WM_PALETTECHANGED:
		if((HWND)wparam == hwnd)
			break;
	/* fall through */
	case WM_QUERYNEWPALETTE:
		hdc = GetDC(hwnd);
		SelectPalette(hdc, palette, 0);
		if(RealizePalette(hdc) != 0)
			InvalidateRect(hwnd, nil, 0);
		ReleaseDC(hwnd, hdc);
		break;
#endif
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &paint);
		SelectPalette(hdc, palette, 0);
		RealizePalette(hdc);
		x = paint.rcPaint.left;
		y = paint.rcPaint.top;
		w = paint.rcPaint.right - x;
		h = paint.rcPaint.bottom - y;
		
		BitBlt(hdc, x, y, w, h, screen, x, y, SRCCOPY);
		EndPaint(hwnd, &paint);
		break;
	case WM_GETMINMAXINFO:
		mmi = (LPMINMAXINFO)lparam;
		mmi->ptMaxSize.x = maxxsize;
		mmi->ptMaxSize.y = maxysize;
		mmi->ptMaxTrackSize.x = maxxsize;
		mmi->ptMaxTrackSize.y = maxysize;
		break;
	case WM_SYSCHAR:
	case WM_COMMAND:
	case WM_CREATE:
	case WM_SETFOCUS:
	case WM_DEVMODECHANGE:
	case WM_WININICHANGE:
	case WM_INITMENU:
	default:
//		if(isunicode)
//			return DefWindowProcW(hwnd, msg, wparam, lparam);
		return DefWindowProcA(hwnd, msg, wparam, lparam);
	}
	return 0;
}



static DWORD WINAPI
winproc(LPVOID x)
{
	MSG msg;
	RECT size;
	WNDCLASSW wc;
	WNDCLASSA wca;
	DWORD ws;

//	if(!previnst)
	{
		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = WindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = inst;
		wc.hIcon = LoadIcon(inst, MAKEINTRESOURCE(100));
		wc.hCursor = NULL;
		wc.hbrBackground = GetStockObject(WHITE_BRUSH);

		wc.lpszMenuName = 0;
		wc.lpszClassName = L"itst";

/*
		if(RegisterClassW(&wc) == 0)
*/
		{
			wca.style = wc.style;
			wca.lpfnWndProc = wc.lpfnWndProc;
			wca.cbClsExtra = wc.cbClsExtra;
			wca.cbWndExtra = wc.cbWndExtra;
			wca.hInstance = wc.hInstance;
			wca.hIcon = wc.hIcon;
			wca.hCursor = wc.hCursor;
			wca.hbrBackground = wc.hbrBackground;

			wca.lpszMenuName = 0;
			wca.lpszClassName = "itst";
//			isunicode = 0;

			RegisterClassA(&wca);
		}
	}

	size.left = 0;
	size.top = 0;
	size.right = maxxsize;
	size.bottom = maxysize;

	ws = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;

	if(AdjustWindowRect(&size, ws, 0)) {
		maxxsize = size.right - size.left;
		maxysize = size.bottom - size.top;
	}else{
		maxxsize = Xsize_ + 40;
		maxysize = Ysize_ + 40;
	}

#if 0
	if(isunicode) {
		window = CreateWindowExW(
			0,			/* extended style */
			L"inferno",		/* class */
			L"Inferno",		/* caption */
			ws,	/* style */
			CW_USEDEFAULT,		/* init. x pos */
			CW_USEDEFAULT,		/* init. y pos */
			maxxsize,		/* init. x size */
			maxysize,		/* init. y size */
			NULL,			/* parent window (actually owner window for overlapped) */
			NULL,			/* menu handle */
			inst,			/* program handle */
			NULL			/* create parms */
			);
	}else
#endif
	{
		window = CreateWindowExA(
			0,			/* extended style */
			"itst",		/* class */
			"itst",		/* caption */
			ws,	/* style */
			CW_USEDEFAULT,		/* init. x pos */
			CW_USEDEFAULT,		/* init. y pos */
			maxxsize,		/* init. x size */
			maxysize,		/* init. y size */
			NULL,			/* parent window (actually owner window for overlapped) */
			NULL,			/* menu handle */
			inst,			/* program handle */
			NULL			/* create parms */
			);
	}

	if(window == nil){
		fprint(2, "can't make window\n");
		ExitThread(0);
	}

	RegisterTouchWindow(window, 0);

	SetForegroundWindow(window);
	ShowWindow(window, cmdshow);
	UpdateWindow(window);
	// CloseWindow(window);

/*
	if(isunicode) {
		while(GetMessageW(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}else
*/
	{
		while(GetMessageA(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}
//	attached = 0;
	ExitThread(msg.wParam);
	return 0;
}


static uchar*
attachscreen(/*Rectangle *r, ulong *chan, int *d, int *width, int *softscreen*/)
{
	int i, k;
	ulong c;
	DWORD h;
	RECT bs;
	RGBQUAD *rgb;
	HBITMAP bits;
	BITMAPINFO *bmi;
	LOGPALETTE *logpal;
	PALETTEENTRY *pal;
	int bsh, bsw, sx, sy;

//	if(attached)
//		goto Return;

	/* Compute bodersizes */
	memset(&bs, 0, sizeof(bs));
	AdjustWindowRect(&bs, WS_OVERLAPPEDWINDOW, 0);
	bsw = bs.right - bs.left;
	bsh = bs.bottom - bs.top;
	sx = GetSystemMetrics(SM_CXFULLSCREEN) - bsw;
//	Xsize_ -= Xsize_ % 4;	/* Round down */
//	if(Xsize_ > sx)
		Xsize_ = sx;
	sy = GetSystemMetrics(SM_CYFULLSCREEN) - bsh + 20;
//	if(Ysize_ > sy)
		Ysize_ = sy;

	logpal = malloc(sizeof(LOGPALETTE) + 256*sizeof(PALETTEENTRY));
	if(logpal == nil)
		return nil;
	logpal->palVersion = 0x300;
	logpal->palNumEntries = 256;
	pal = logpal->palPalEntry;

/*
	c = displaychan;
	if(c == 0)
		c = autochan();
	k = 8;
	if(TYPE(c) == CGrey){
		graphicsgmap(pal, NBITS(c));
		c = GREY8;
	}else{
		if(c == RGB15)
			k = 16;
*/
//		else if(c == RGB24)
//			k = 24;
//		else if(c == XRGB32)
			k = 32;
//		else
//			c = CMAP8;
//		graphicscmap(pal);
//	}

	palette = CreatePalette(logpal);

//	if(k == 8)
//		bmi = malloc(sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));
//	else
		bmi = malloc(sizeof(BITMAPINFOHEADER));
	if(bmi == nil)
		return nil;
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth = Xsize_;
	bmi->bmiHeader.biHeight = -Ysize_;	/* - => origin upper left */
	bmi->bmiHeader.biPlanes = 1;	/* always 1 */
	bmi->bmiHeader.biBitCount = k;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = 0;	/* Xsize_*Ysize_*(k/8) */
	bmi->bmiHeader.biXPelsPerMeter = 0;
	bmi->bmiHeader.biYPelsPerMeter = 0;
	bmi->bmiHeader.biClrUsed = 0;
	bmi->bmiHeader.biClrImportant = 0;	/* number of important colors: 0 means all */

/*
	if(k == 8){
		rgb = bmi->bmiColors;
		for(i = 0; i < 256; i++){
			rgb[i].rgbRed = pal[i].peRed;
			rgb[i].rgbGreen = pal[i].peGreen;
			rgb[i].rgbBlue = pal[i].peBlue;
		}
	}
*/

	screen = CreateCompatibleDC(NULL);
	if(screen == nil){
		fprint(2, "screen dc nil\n");
		return nil;
	}

	if(SelectPalette(screen, palette, 1) == nil){
		fprint(2, "select pallete failed\n");
	}
	i = RealizePalette(screen);
	GdiFlush();
	bits = CreateDIBSection(screen, bmi, DIB_RGB_COLORS, &graph_scrn_data, nil, 0);
	if(bits == nil){
		fprint(2, "CreateDIBSection failed\n");
		return nil;
	}

	SelectObject(screen, bits);
	GdiFlush();
	CreateThread(0, 16384, winproc, nil, 0, &h);
//	attached = 1;


    Return:
//	r->min.x = 0;
//	r->min.y = 0;
//	r->max.x = Xsize_;
//	r->max.y = Ysize_;
//	displaychan = c;
//	*chan = c;
//	*d = k;
//	*width = (Xsize_/4)*(k/8);
//	*softscreen = 1;
	return (uchar*)graph_scrn_data;
}


void flushscreen() //Rectangle r)
{
	RECT wr;

//	if(r.max.x<=r.min.x || r.max.y<=r.min.y)
//		return;
	wr.left = 0; //r.min.x;
	wr.top = 0; //r.min.y;
	wr.right = Xsize_; //r.max.x;
	wr.bottom = Ysize_; //r.max.y;
	InvalidateRect(window, &wr, 0);
}
/*
*/


int main(int __argc, char** __argv);


int WINAPI
WinMain(HINSTANCE winst, HINSTANCE wprevinst, LPSTR cmdline, int wcmdshow)
{
	inst = winst;
	previnst = wprevinst;
	cmdshow = wcmdshow;

	attachscreen();

	/* cmdline passed into WinMain does not contain name of executable.
	 * The globals __argc and __argv to include this info - like UNIX
	 */
	main(__argc, __argv);
	return 0;
}


//{} Win test screen AA



