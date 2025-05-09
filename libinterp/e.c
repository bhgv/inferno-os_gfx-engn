
#include	<windows.h>



#include <lib9.h>
#include <isa.h>
#include <interp.h>

#include "e.h"
#include "emod.h"

#undef Unknown
#include "../emu/port/dat.h"
//#include "../emu/port/fns.h"
#include "kernel.h"

#include "cairo-features.h"
#include <cairo.h>
#include <svg-cairo.h>

#define _USE_MATH_DEFINES
#include <math.h>



#define D() print("%s> %s:%d\n", __FILE__, __func__, __LINE__)
#define DD(...) {print("%s> %s:%d ", __FILE__, __func__, __LINE__); print(__VA_ARGS__); print("\n");}


//graphic_el* groot = NULL;

//graphic_el* free_levels = NULL;


typedef union {
	float f;
	char  c[sizeof(float)];
} floatToChars_t;


Proc* getup();


static graphic_el** find_graph_el_by_id(int id, int *is_not_free);


static cairo_surface_t *
_draw_svg(char* svg_buf, double *w_out, double *h_out);


cairo_surface_t *sfc_main_screen = NULL;
uint             main_screen_flags = 0;



//{} Win test screen VV



static	HINSTANCE	inst;
static	HINSTANCE	previnst;
static	int		cmdshow;
static	HWND		window;
static	HDC		screen;
static	HPALETTE	palette;

static	HCURSOR		hcursor;


static	ulong	*data = NULL;


static	int		maxxsize = 1024;
static	int		maxysize = 800;

static	int	Xsize;
static	int	Ysize;



static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	PAINTSTRUCT paint;
	HDC hdc;
	LPMINMAXINFO mmi;
	LONG x, y, w, h, b;
	HCURSOR dcurs;
	POINT m;

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
#if 0
	case WM_MOUSEWHEEL:
		if((int)wparam>0)
			b = 8;
		else
			b = 16;
		m.x = LOWORD(lparam);
		m.y = HIWORD(lparam);
		ScreenToClient(hwnd, &m);
		goto mok;
	case WM_LBUTTONDBLCLK:
		b = (1<<8) | 1;
		goto process;
	case WM_MBUTTONDBLCLK:
		b = (1<<8) | 2;
		goto process;
	case WM_RBUTTONDBLCLK:
		b = (1<<8) | 4;
		goto process;
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		b = 0;
	process:
		m.x = LOWORD(lparam);
		m.y = HIWORD(lparam);
	mok:
		if(wparam & MK_LBUTTON)
			b |= 1;
		if(wparam & MK_MBUTTON)
			b |= 2;
		if(wparam & MK_RBUTTON) {
			if(wparam & MK_CONTROL)
				b |= 2;  //simulate middle button
			else
				b |= 4;  //right button
		}
		mousetrack(b, m.x, m.y, 0);
		break;
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
		maxxsize = Xsize + 40;
		maxysize = Ysize + 40;
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
//	Xsize -= Xsize % 4;	/* Round down */
//	if(Xsize > sx)
		Xsize = sx;
	sy = GetSystemMetrics(SM_CYFULLSCREEN) - bsh + 20;
//	if(Ysize > sy)
		Ysize = sy;

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
	bmi->bmiHeader.biWidth = Xsize;
	bmi->bmiHeader.biHeight = -Ysize;	/* - => origin upper left */
	bmi->bmiHeader.biPlanes = 1;	/* always 1 */
	bmi->bmiHeader.biBitCount = k;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = 0;	/* Xsize*Ysize*(k/8) */
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
	bits = CreateDIBSection(screen, bmi, DIB_RGB_COLORS, &data, nil, 0);
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
//	r->max.x = Xsize;
//	r->max.y = Ysize;
//	displaychan = c;
//	*chan = c;
//	*d = k;
//	*width = (Xsize/4)*(k/8);
//	*softscreen = 1;
	return (uchar*)data;
}


static void
flushscreen() //Rectangle r)
{
	RECT wr;

//	if(r.max.x<=r.min.x || r.max.y<=r.min.y)
//		return;
	wr.left = 0; //r.min.x;
	wr.top = 0; //r.min.y;
	wr.right = Xsize; //r.max.x;
	wr.bottom = Ysize; //r.max.y;
	InvalidateRect(window, &wr, 0);
}
/*
*/


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



enum {
	SHP_OP_MOVE_TO = 1,
	SHP_OP_LINE_TO,
	SHP_OP_CURVE_TO,
	SHP_OP_ARC_CW,
	SHP_OP_ARC_CCW,
	SHP_OP_toMASK,
	SHP_OP_unMASK,
	SHP_OP_CLOSE_PATH,
};



static cairo_surface_t *render_canvas(graphic_el* el);

static void
render_gr_stack_to_surface(cairo_surface_t *sfc, cairo_t *cr, graphic_el* gr_stack_root);


static cairo_surface_t *
render_shape(graphic_el* el)
{
	int i;
	cairo_surface_t *sfc;
	cairo_t *cr;

	double x_min, y_min, x_max, y_max;

//	floatToChars_t tmp;
//D();

	if (!el || !el->data || el->data_len <= 0) {
		if (el->canvas) {
			return render_canvas(el);
		}

		return NULL;
	}

//D();
	sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 2000 /*Xsize / 2*/, 2000 /*Ysize / 2*/);

	cr = cairo_create(sfc);

//	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
//	cairo_paint(cr);

//	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	int isMask = 0;

DD("data: %p, len: %d", el->data, el->data_len);

	cairo_new_path(cr);

	for (i = 0; i < el->data_len;) {
		char key = el->data[i++];

		switch (key) {
			case SHP_OP_MOVE_TO:
				{
					float x, y;
D();

					x = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);
					y = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					cairo_move_to(cr, x, y);
				}
				break;

			case SHP_OP_LINE_TO:
				{
					float x, y;
D();

					x = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);
					y = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					cairo_line_to(cr, x, y);
				}
				break;

			case SHP_OP_CURVE_TO:
				{
					float x1, y1, x2, y2, x3, y3;
D();

					// 1
					x1 = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);
					y1 = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					// 2
					x2 = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);
					y2 = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					// 3
					x3 = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);
					y3 = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					cairo_curve_to(cr, x1, y1, x2, y2, x3, y3);
				}
				break;

			case SHP_OP_ARC_CW:
				{
					float x, y, r, a_fr, a_to;
D();

					// xy
					x = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);
					y = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					// r
					r = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					// a: fr - to
					a_fr = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);
					a_to = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					cairo_arc(cr, x, y, r, a_fr, a_to);
				}
				break;

			case SHP_OP_ARC_CCW:
				{
					float x, y, r, a_fr, a_to;
D();

					// xy
					x = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);
					y = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					// r
					r = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					// a: fr - to
					a_fr = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);
					a_to = ((floatToChars_t*)&(el->data[i]))->f;
					i += sizeof(floatToChars_t);

					cairo_arc_negative(cr, x, y, r, a_fr, a_to);
				}
				break;

			case SHP_OP_CLOSE_PATH:
D();
				cairo_close_path (cr);
				break;

			case SHP_OP_toMASK:
D();
				isMask = 1;
				break;

			case SHP_OP_unMASK:
D();
				isMask = 0;
				break;

		}
	}

	cairo_path_t* path = cairo_copy_path(cr);

	cairo_set_line_width (cr, el->line_width);
	cairo_stroke_preserve(cr);

	cairo_stroke_extents(cr, &el->x_min, &el->y_min, &el->x_max, &el->y_max);

	el->w = el->x_max - el->x_min;
	el->h = el->y_max - el->y_min;

DD("w: %f (%f - %f), h: %f (%f - %f)", 
		(float)el->w, (float)el->x_max, (float)el->x_min,
		(float)el->h, (float)el->y_max, (float)el->y_min);
	cairo_destroy(cr);
	cairo_surface_destroy(sfc);


	sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, el->w, el->h);

	cr = cairo_create(sfc);

	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	double dx = el->x_min;
	double dy = el->y_min;
DD("path->num_data: %d", path->num_data);

#if 0
	for (i=0; i < path->num_data; i += path->data[i].header.length) {
DD("i: %d", i);
		cairo_path_data_t *data;

		data = &path->data[i];
		switch (data->header.type) {
		case CAIRO_PATH_MOVE_TO:
D();
			data[1].point.x -= dx /2.;
			data[1].point.y -= dy /2.;
			break;
		case CAIRO_PATH_LINE_TO:
D();
			data[1].point.x -= dx /2.;
			data[1].point.y -= dy /2.;
			break;
		case CAIRO_PATH_CURVE_TO:
D();
			data[1].point.x -= dx /2.;
			data[1].point.y -= dy /2.;
			data[2].point.x -= dx /2.;
			data[2].point.y -= dy /2.;
			data[3].point.x -= dx /2.;
			data[4].point.y -= dy /2.;
//			do_curve_to_things (data[1].point.x, data[1].point.y,
//								data[2].point.x, data[2].point.y,
//								data[3].point.x, data[3].point.y);
			break;
		case CAIRO_PATH_CLOSE_PATH:
D();
//			do_close_path_things ();
			break;
		}
D();
	}
#else
	cairo_translate (cr, -dx, -dy);
#endif

D();
//	cairo_move_to(cr, -el->x_min, el->y_min);
	cairo_append_path(cr, path);

D();
	if (isMask) {
D();
		cairo_move_to(cr, el->x_min, el->y_min);
		cairo_clip    (cr);
		cairo_new_path(cr); /* path not consumed by clip()*/

//--VV
		graphic_el* el2 = el->masked;
		for (; el2 != NULL; el2 = el2->next) {
			cairo_matrix_t matrix;
D();
			if (el2->flags & GR_EL_WAIT_TO_RENDER) {
D();
				if (el2->sfc) {
					cairo_surface_destroy(el2->sfc);
					el2->sfc = NULL;
				}

				if ((el2->type & 0xff) == GR_EL_SHAPE) {
D();
					el2->sfc = render_shape(el2);
				}
			}

D();
			if (el2->flags & GR_EL_RENDERED && el2->sfc != NULL) {
D();
#if 1
				cairo_pattern_t *pat = cairo_pattern_create_for_surface(el2->sfc);
				cairo_pattern_set_filter(pat, CAIRO_FILTER_GOOD);

DD("scale sxy: %f, %f", el2->sx, el2->sy);
				cairo_matrix_init_scale(&matrix, el2->sx, el2->sy);
				cairo_matrix_translate(&matrix, /*-el->x_min*/ - el2->x, /*-el->y_min*/ - el2->y);

				cairo_pattern_set_matrix(pat, &matrix);

				cairo_set_source(cr, pat);
				cairo_paint(cr);
				cairo_pattern_destroy(pat);
#else
//				cairo_scale (cr, el2->sx, el2->sy);
				cairo_set_source_surface (cr, el2->sfc, -el2->x, -el2->y);
				cairo_paint(cr);
#endif
//D();
			}
		}
//--AA

//		cairo_set_source_rgba(cr, el->line_r, el->line_g, el->line_b, el->line_a);
//		cairo_set_line_width (cr, el->line_width);
//		cairo_stroke         (cr);
D();

	} else {
D();
		cairo_set_source_rgba(cr, el->fill_r, el->fill_g, el->fill_b, el->fill_a);
		cairo_fill_preserve  (cr);
		cairo_set_source_rgba(cr, el->line_r, el->line_g, el->line_b, el->line_a);
		cairo_set_line_width (cr, el->line_width);
		cairo_stroke         (cr);
	}
D();

	cairo_path_destroy(path);
D();
	cairo_destroy(cr);
D();
	el->flags &= ~GR_EL_WAIT_TO_RENDER;
	el->flags |=  GR_EL_RENDERED;

D();
	return sfc;
}



static cairo_surface_t *
render_canvas(graphic_el* el)
{
	cairo_surface_t *sfc2 = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 2000, 2000);

	cairo_t *cr2 = cairo_create(sfc2);
	cairo_set_operator(cr2, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr2);
	cairo_destroy(cr2);

	double x1, y1, x2, y2;

	cr2 = cairo_create(sfc2);
DD("Before Draw Sub-Canvas");
	render_gr_stack_to_surface(sfc2, cr2, el->canvas);

//				cairo_fill_extents(cr2, &x1, &y1, &x2, &y2);

	cairo_destroy(cr2);

	graphic_el *el_it = el->canvas;
	x1 = 10000000.;
	y1 = 10000000.;
	x2 = -10000000.;
	y2 = -10000000.;
	for (; el_it != NULL; el_it = el_it->next) {
		if (el_it->x_min < el_it->x_max && el_it->y_min < el_it->y_max) {
			if (el_it->x_min < x1)
				x1 = el_it->x_min;
			if (el_it->x_max > x2)
				x2 = el_it->x_max;
			if (el_it->y_min < y1)
				y1 = el_it->y_min;
			if (el_it->y_max > y2)
				y2 = el_it->y_max;
		}
	}

	el->x_min = x1;
	el->x_max = x2;
	el->y_min = y1;
	el->y_max = y2;

	double w = x2 - x1;
	double h = y2 - y1;

DD("subCanvas: w: %f (%f - %f), h: %f (%f - %f)", 
					(float)w, (float)x2, (float)x1,
					(float)h, (float)y2, (float)y1);

	cairo_surface_t *sfc3 = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cairo_t *cr3 = cairo_create(sfc3);

	cairo_set_source_surface (cr3, sfc2, -x1, -y1);   //
	cairo_paint(cr3);								   //

	cairo_destroy(cr3);

	cairo_surface_destroy(sfc2); //

	return sfc3; //
}



static void
render_gr_stack_to_surface(cairo_surface_t *sfc, cairo_t *cr, graphic_el* gr_stack_root)
{
//		cairo_surface_t *svg_sfc = _draw_svg(svg_buf, &w_out, &h_out);
	graphic_el* el = gr_stack_root;
//	if (el == NULL)
//		return;

//	cairo_surface_t *surface;
//	cairo_t *cr;
//	float scale = 1.0f;

//	cr = cairo_create(sfc);

	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

//D();
	for (; el != NULL; el = el->next) {
		cairo_matrix_t matrix;
//D();
		if (el->flags & GR_EL_WAIT_TO_RENDER) {
//D();
			if (el->sfc) {
				cairo_surface_destroy(el->sfc);
				el->sfc = NULL;
			}

			if ((el->type & 0xff) == GR_EL_SHAPE) {
D();
				el->sfc = render_shape(el);
			} else if ((el->type & 0xff) == GR_EL_CANVAS) {
D();
				el->sfc = render_canvas(el);

				el->flags &= ~GR_EL_WAIT_TO_RENDER;
				el->flags |=  GR_EL_RENDERED;
			}
		}

//D();
		if (el->flags & GR_EL_RENDERED && el->sfc != NULL) {
//D();
			cairo_pattern_t *pat = cairo_pattern_create_for_surface(el->sfc);
			cairo_pattern_set_filter(pat, CAIRO_FILTER_GOOD);

DD("sxy: %f, %f", el->sx, el->sy);
			cairo_matrix_init_scale(&matrix, el->sx, el->sy);
//			cairo_matrix_init_scale(&matrix, scale, scale);
DD("min_xy: (%f, %f), yx: (%f, %f)", (float)el->x_min, (float)el->y_min, (float)el->x, (float)el->y);
			cairo_matrix_translate(&matrix, -el->x_min - el->x, -el->y_min - el->y);

			cairo_pattern_set_matrix(pat, &matrix);

			cairo_set_source(cr, pat);
			cairo_pattern_destroy(pat);
			cairo_paint(cr);
//D();
		}
	}

//	cairo_destroy(cr);
//D();
//	cairo_surface_destroy(sfc_main_screen);
}


static void
render_gr_stack_to_main_screen()
{
	if (sfc_main_screen == NULL)
		sfc_main_screen = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, Xsize, Ysize);

	if (sfc_main_screen == NULL)
		return;

	Proc* p = getup();

	cairo_t *cr = cairo_create(sfc_main_screen);
	render_gr_stack_to_surface(sfc_main_screen, cr, p->groot);
	cairo_destroy(cr);

//D();
	uint32 *pixel = data;

	unsigned char* buf = cairo_image_surface_get_data(sfc_main_screen);
	uint32_t *buf32 = (uint32_t*)buf;
	
//D();
	for (uint32_t x = 0; x < Ysize; x++) {
		for (uint32_t y = 0; y < Xsize; y++) {
			*pixel++ = *buf32++;
		}
//		pixel += (Xsize - w_out);
	}
//D();
}













static uchar	Level_map[] = E_Level_map;
static Type*	Tlevel;
static uchar	GrOp_map[] = E_GrOp_map;
static Type*	TGrOp;

static uchar	Shape_map[] = E_Shape_map;
static Type*	TShape;



static cairo_surface_t *
_draw_svg(char* svg_buf, double *w_out, double *h_out)
{
	if (!svg_buf)
		return NULL;


/*	
	D();
		uint32_t *pixel = static_cast<uint32_t *>(mappedAddr_); // дЅїз”ЁmmapиЋ·еЏ–е€°зљ„ењ°еќЂжќҐи®їй—®е†…е­
		if (pixel == nullptr) {
	D();
			DRAWING_LOGE("pixel is null");
			return;
		}
	
	D();
		uint32_t width = static_cast<uint32_t>(bufferHandle_->stride / 4);
*/

/*  //{} prepare other (external) surface
		cairo_surface_t *surface;
		cairo_t *cr;
		float scale = 1.0f;
		float r, g, b;
		uint32_t fbsizex = width;
		uint32_t fbsizey = height_;
		int startx, starty, sizex, sizey;
	
		surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, fbsizex, fbsizey);
		cr = cairo_create(surface);
	
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_paint(cr);
	
		cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
*/
	
		svg_cairo_t *svgc;
		
		unsigned int svg_width, svg_height;
	
		svg_cairo_status_t status;
		
		status = svg_cairo_create (&svgc);
		if (status) {
	D();
			fprintf (stderr, "Failed to create svg_cairo_t. Exiting.\n");
			return NULL;
		}
		
	D();
//		char svg_buf[]
/*
	svg_buf =
	"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
	"<svg\n"
	"	width=\"210mm\"\n"
	"	height=\"297mm\"\n"
	"	viewBox=\"0 0 210 297\"\n"
	"	version=\"1.1\"\n"
	"	id=\"svg1\"\n"
	"	xmlns=\"http://www.w3.org/2000/svg\"\n"
	"	xmlns:svg=\"http://www.w3.org/2000/svg\">\n"
	"  <defs\n"
	"	  id=\"defs1\" />\n"
	"  <g\n"
	"	  id=\"layer1\">\n"
	"	 <rect\n"
	"		style=\"fill:#ec8500;stroke:#d218d1;stroke-width:3;fill-opacity:1;stroke-opacity:1;stroke-dasharray:12,6,3,6;stroke-linejoin:round;stroke-dashoffset:0\"\n"
	"		id=\"rect1\"\n"
	"		width=\"38.831459\"\n"
	"		height=\"32.157303\"\n"
	"		x=\"0\"\n"
	"		y=\"0\" />\n"
	"  </g>\n"
	"</svg>\n"
		;
*/

DD("svg (%d): %s", strlen(svg_buf), svg_buf);
		status = svg_cairo_parse_buffer (svgc, svg_buf, strlen(svg_buf));
		if (status) {
	D();
			return NULL;
		}
		
	D();
		svg_cairo_get_size (svgc, &svg_width, &svg_height);
	
		
		cairo_surface_t *svg_sfc;
		cairo_t *svg_cr;
	
		svg_sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, svg_width, svg_height);
		svg_cr = cairo_create(svg_sfc);
	
	//	  cairo_set_operator(svg_cr, CAIRO_OPERATOR_CLEAR);
	//	cairo_paint(svg_cr);
	
		cairo_set_operator(svg_cr, CAIRO_OPERATOR_OVER);
	
	D();
		
		
		/*
		if (width < 0 && height < 0) {
			width = (svg_width * scale + 0.5);
			height = (svg_height * scale + 0.5);
		} else if (width < 0) {
			scale = (double) height / (double) svg_height;
			width = (svg_width * scale + 0.5);
		} else if (height < 0) {
			scale = (double) width / (double) svg_width;
			height = (svg_height * scale + 0.5);
		} else {
			scale = MIN ((double) width / (double) svg_width, (double) height / (double) svg_height);
			/ * Center the resulting image * /
	//		  dx = (width - (int) (svg_width * scale + 0.5)) / 2;
	//		  dy = (height - (int) (svg_height * scale + 0.5)) / 2;
		}
	*/
	
	//		cairo_identity_matrix(cr);
	
	
	//	  cairo_translate (cr, x_, y_);
	//	  cairo_scale (cr, scale, scale);
	
		status = svg_cairo_render (svgc, svg_cr);

/* //{} draw moved and scaled to other surface
	D();
		cairo_matrix_t matrix;
		cairo_pattern_t *pat = cairo_pattern_create_for_surface(svg_sfc);
		cairo_pattern_set_filter(pat, CAIRO_FILTER_GOOD);
	
		cairo_matrix_init_scale(&matrix, scale, scale);
		cairo_matrix_translate(&matrix, -x_, -y_);
	
		cairo_pattern_set_matrix(pat, &matrix);
		
		cairo_set_source(cr, pat);
		cairo_pattern_destroy(pat);
		cairo_paint(cr);
*/

		cairo_destroy(svg_cr);

	if (w_out)
		*w_out = (double)svg_width;
	if (h_out)
		*h_out = (double)svg_height;
		
#if 0
		srand(time(NULL));
	
			r = (rand() % 100) / 100.0;
			g = (rand() % 100) / 100.0;
			b = (rand() % 100) / 100.0;
			startx = rand() % fbsizex;
			starty = rand() % fbsizey;
			sizex = rand() % (fbsizex - startx);
			sizey = rand() % (fbsizey - starty);
	
			//if (ts) {
			//	int pressed = 0;
	
				/* Pressure is our identication whether we act on axis... */
				//while (ts_read(ts, &sample, 1)) {
				//	if (sample.pressure > 0)
				//		pressed = 1;
				//}
	
				//if (pressed) {
				//	scale *= 1.05f;
				//	cairo_translate(cr, sample.x, sample.y);
				//	cairo_scale(cr, scale, scale);
					//r = g = b = 0;
				//	startx = -5;
				//	starty = -5;
				//	sizex = 10;
				//	sizey = 10;
				//} else {
					scale = 1.0f;
				//}
	
			cairo_set_source_rgb(cr, r, g, b);
			cairo_rectangle(cr, startx, starty, sizex, sizey);
			cairo_stroke_preserve(cr);
			cairo_fill(cr);
#endif

//D();

/* //{} draw to buf		
		unsigned char* buf = cairo_image_surface_get_data(surface);
		uint32_t *buf32 = (uint32_t*)buf;
		
		for (uint32_t x = 0; x < width; x++) {
			for (uint32_t y = 0; y < height_; y++) {
				*pixel++ = *buf32++;
			}
		}
	
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
*/
	return svg_sfc;
}






void
emodinit(void)
{
        builtinmod("$E", Emodtab, Emodlen);
		D();
//		Tlevel = dtype(freeheap, sizeof(struct E_Level), Levelmap, sizeof(Levelmap));
		Tlevel = dtype(freeheap, E_Level_size, Level_map, sizeof(Level_map));
		TGrOp = dtype(freeheap, E_GrOp_size, GrOp_map, sizeof(GrOp_map));

		TShape = dtype(freeheap, E_Shape_size, Shape_map, sizeof(Shape_map));

		D();
		Proc *p      = getup();
		p->groot     = nil;
		p->groot_len = 0;
		p->free_levels = nil;

D();
//		groot = NULL;
}



static void proc_user_cleaner_iterator(graphic_el *el);

static graphic_el* del_graphic_el(graphic_el *el)
{
	if (!el)
		return NULL;

	graphic_el *el_nxt = el->next;

	if (el->sfc) {
		cairo_surface_destroy(el->sfc);
	}

	if (el->data)
		free(el->data);

	//if (el->svg)
	//	free(el->svg);

	if (/*(el->type & GR_EL_MASK) &&*/ el->masked) {
		proc_user_cleaner_iterator(el->masked);
		el->masked = NULL;
	}
	if (el->canvas) {
		proc_user_cleaner_iterator(el->canvas);
		el->canvas = NULL;
	}

	free(el);

	return el_nxt;
}


static void proc_user_cleaner_iterator(graphic_el *el)
{
	for (; el != NULL && el != nil;) {
		el = del_graphic_el(el);
	}
}

void proc_user_cleaner(Proc *p)
{
//D();
	if (!p)
		return;

//D();
	graphic_el *el = p->groot;
	proc_user_cleaner_iterator(el);
	p->groot = NULL;

//D();
	el = p->free_levels;
	proc_user_cleaner_iterator(el);
	p->free_levels = NULL;
//D();
}



static void
reset_rendered_flag(graphic_el **el)
{
	if (el && *el) {
		(*el)->flags &= ~GR_EL_RENDERED;
		(*el)->flags |=  GR_EL_WAIT_TO_RENDER;
	}

	main_screen_flags &= ~GR_EL_RENDERED;
	main_screen_flags |=  GR_EL_WAIT_TO_RENDER;
}





static graphic_el** do_find_graph_el_by_id(graphic_el **root_el_nest, int id)
{
	if (!root_el_nest || id <= 0)
		return NULL;

	
	graphic_el **el_nest = root_el_nest;
	graphic_el *el       = *el_nest;
	if (!el)
		return el_nest;

	
	for (; el != NULL && el->id != id;) {
		if (el->masked) {
			graphic_el **masked_el_nest = do_find_graph_el_by_id(&el->masked, id);
			if (masked_el_nest && *masked_el_nest)
				return masked_el_nest;
		}
		if (el->canvas) {
			graphic_el **canvas_el_nest = do_find_graph_el_by_id(&el->canvas, id);
			if (canvas_el_nest && *canvas_el_nest)
				return canvas_el_nest;
		}
		el_nest = &el->next;
		el = *el_nest;
	}
	return el_nest;
}

static graphic_el** find_graph_el_by_id(int id, int *is_not_free)
{
	Proc *p = getup();
	int not_free = 0;

	graphic_el **el_nest = do_find_graph_el_by_id(&p->free_levels, id);
	if (!el_nest)
		return NULL;

	graphic_el *el = *el_nest;

	if (el == NULL) {
		not_free = 1;

		el_nest = do_find_graph_el_by_id(&p->groot, id);
	}

	if (is_not_free)
		*is_not_free = not_free;

	return el_nest;
}


static graphic_el* set_graph_el_to_pos(graphic_el** parent_el_nest, graphic_el *el, int n, int is_ins)
{
//D();
	if (el == NULL || n < 0)
		return NULL;

//D();
	graphic_el **el_nest;
	graphic_el **parent;

	if (!parent_el_nest || !*parent_el_nest) {
		Proc *p = getup();
D();
		el_nest = &p->groot;
		parent  = &p->groot;
	} else if (((*parent_el_nest)->type & 0xff) == GR_EL_CANVAS) {
D();
		el_nest = &((*parent_el_nest)->canvas);
		parent  = &((*parent_el_nest)->canvas);
	} else {
D();
		el_nest = &((*parent_el_nest)->masked);
		parent  = &((*parent_el_nest)->masked);
	}

	graphic_el *el2 = *el_nest;
	
//D();
	// go to level
	for (; n >= 0; n--) {
D();
		if (el2 == NULL) {
D();
			el2 = malloc(sizeof(graphic_el));
			memset(el2, 0, sizeof(graphic_el));

			int id = ((int)((ulong)el2 >> 2) ^ (int)((ulong)el2 >> 32)) & 0xfffffff;
			
			el2->id   = id;
			el2->type = GR_EL_SHAPE;

			*el_nest = el2;
D();
		}

		if (n == 0)
			break;
//D();
		el_nest = &el2->next;
		el2 = *el_nest;
D();
	}
	
//D();
	if (el->id != el2->id) {
//D();
		el->root = parent;

		if (!is_ins) {
D();
			el->next = el2->next;
//D();
			*el_nest = el;
		
//D();
			if (el2->id <= 0) {
//D();
				free(el2);
				return NULL;
			} else {
D();
				Proc *p = getup();
				el2->next = p->free_levels;
				p->free_levels = el2;

				return el2;
			}
		} else {
D();
			*el_nest = el;
			el->next = el2;
//D();
		}

		reset_rendered_flag(&el);
	}
D();
	return el;
}




#if 0
void E_init(void* par)
{
	F_E_init *f = par;
//	Proc *prc = getup();
	D();

	Array *ar;

	ar = H2D(Array*, heaparray(&Tptr, 16));
	D();
	*f->ret = ar;

	D();
}
#endif



void Level_new(void* par)
{
	F_Level_new *f = par;
	E_Level *lvl;

D();
//	lvl = H2D(E_Level**, heap(&Tptr));
	lvl = H2D(E_Level*, heap(Tlevel));

	D();
	lvl->graphics.fill = H;

	D();
	destroy(*f->ret);
	*f->ret = lvl;

	D();




	Proc *p = getup();

	graphic_el *l = malloc(sizeof(graphic_el));
	memset(l, 0, sizeof(graphic_el));

	l->qid = (uint)((ulong)l >> 2);

	l->next = p->free_levels;
	p->free_levels = l;

}


//static cairo_surface_t *renderSVG(char* svg) {
//	return NULL;
//}



static void
crRefShapeObj(int id, E_Shape** p)
{
	E_Shape* shp;

	if (!p)
		return;

//	 ref = H2D(E_Shape**, heap(Tptr));
	shp = H2D(E_Shape*,  heap(TShape));
	shp->id = id;

	
	destroy(*p);
	*p = shp;

//	return shp;
}



void E_rootCanvas(void* par)
{
	F_E_rootCanvas *f = par;

/*
	Proc *p 	 = getup();

	graphic_el *el = malloc(sizeof(graphic_el));
	memset(el, 0, sizeof(graphic_el));
		
	int id = ((int)((ulong)el >> 2) ^ (int)((ulong)el >> 32)) & 0xfffffff;

	el->id = id;
	el->type = GR_EL_SHAPE;
	el->data = NULL;
	el->masked = NULL;
	el->flags = GR_EL_WAIT_TO_RENDER;

	el->sfc = NULL;

	el->angle = 0.;
	el->x = 0.;
	el->y = 0.;
	el->sx = 1.;
	el->sx = 1.;

	reset_rendered_flag(el);
D();
	el->next = p->free_levels;
	p->free_levels = el;

	if (!p->proc_user_cleaner)
		p->proc_user_cleaner = proc_user_cleaner;

*/
	crRefShapeObj(0, f->ret);
}



static graphic_el* new_graphic_el(int type, int isAddToFreeList)
{
	graphic_el *el = malloc(sizeof(graphic_el));
	memset(el, 0, sizeof(graphic_el));

	int id = ((int)((ulong)el >> 2) ^ (int)((ulong)el >> 32)) & 0xfffffff;

	el->id = id;
	el->type = type;

	el->angle = 0.;
	el->x = 0.;
	el->y = 0.;
	el->sx = 1.;
	el->sy = 1.;

	el->line_b = 1.;

	el->line_a = 1.;
	el->fill_a = 1.;

	el->line_width = 1.;

//	el->flags &= ~GR_EL_WAIT_TO_RENDER;
//	el->flags |=  GR_EL_RENDERED;	
	el->flags = GR_EL_WAIT_TO_RENDER;

	Proc *p 	 = getup();

	if (isAddToFreeList) {
		el->next = p->free_levels;
		p->free_levels = el;
	}

	if (!p->proc_user_cleaner)
		p->proc_user_cleaner = proc_user_cleaner;

//	main_screen_flags &= ~GR_EL_RENDERED;
//	main_screen_flags |=  GR_EL_WAIT_TO_RENDER;

	return el;
}



void E_newCanvas(void* par)
{
	F_E_newCanvas *f = par;

	graphic_el* el = new_graphic_el(GR_EL_CANVAS, 1);

	crRefShapeObj(el->id, f->ret);
D();
}


static graphic_el* do_new_svg_el(char* svg_str, int isAddToFreeList)
{
	graphic_el* el = new_graphic_el(GR_EL_SVG, isAddToFreeList);

	el->data = strdup(svg_str);

	el->sfc = _draw_svg(svg_str, &el->w, &el->h);

	el->x_min = 0.;
	el->x_max = el->w;
	el->y_min = 0.;
	el->y_max = el->h;

	el->flags &= ~GR_EL_WAIT_TO_RENDER;
	el->flags |=  GR_EL_RENDERED;

	return el;
}


static E_Shape* do_new_svg(char* svg_str, E_Shape* o_old)
{
	E_Shape* o_out = o_old;

	graphic_el* el = do_new_svg_el(svg_str, 1);	

	crRefShapeObj(el->id, &o_out);

	return o_out;
}


static E_Shape* do_new_shape(E_Shape* o_old)
{
	E_Shape* o_out = o_old;

	graphic_el* el = new_graphic_el(GR_EL_SHAPE, 1);

	crRefShapeObj(el->id, &o_out);

	return o_out;
}


static uint chunk_enum_graph_ops(uchar *chunk, uint max)
{
	uint i = 0;

	for (; i < max; i++) {
		uchar op = chunk[i];
		switch (op) {
			case SHP_OP_MOVE_TO:
				i += 2*sizeof(float);
				break;

			case SHP_OP_LINE_TO:
				i += 2*sizeof(float);
				break;

			case SHP_OP_CURVE_TO:
				i += 3*2*sizeof(float);
				break;

			case SHP_OP_ARC_CW:
				i += (2 + 1 + 2)*sizeof(float);
				break;

			case SHP_OP_ARC_CCW:
				i += (2 + 1 + 2)*sizeof(float);
				break;

			case SHP_OP_toMASK:
				break;

			case SHP_OP_unMASK:
				break;

			case SHP_OP_CLOSE_PATH:
				break;

			default:
				return i;

		}
	}
	return i;
}


enum {
	GR_CHUNK_SVG   = 1,
	GR_CHUNK_SHAPE,
};


typedef struct {
	uchar ver;
	uint  len;
	uint  cksum;
	uchar type;
} chunk_hdr_t;

typedef struct {
	uint len;
	uint cksum;

	uint gops_len;

	float fill_r, fill_g, fill_b, fill_a,
		  stroke_r, stroke_g, stroke_b, stroke_a,
		  stroke_w,
		  angle;
} chunk_shape_hdr_t;


static graphic_el* chunk_new_shape_el(uchar typ, uchar* p, uint len, uint* cnt_used);


static graphic_el* chunk_parse_shape_el(uchar typ_par, uchar *chunk, uint max, uint* cnt_used)
{
	uint i;
	graphic_el*  el = new_graphic_el(typ_par == 'c' ? GR_EL_CANVAS : GR_EL_SHAPE, 0);
	graphic_el** masked_nest    = &el->masked;
	graphic_el** subcanvas_nest = &el->canvas;

	uchar* p = chunk + sizeof(chunk_shape_hdr_t);

	chunk_shape_hdr_t* hdr = (chunk_shape_hdr_t*)chunk;

	uint len = hdr->len;
	uint cksum = hdr->cksum;

	el->fill_r = hdr->fill_r;
	el->fill_g = hdr->fill_g;
	el->fill_b = hdr->fill_b;
	el->fill_a = hdr->fill_a;

	el->line_r = hdr->stroke_r;
	el->line_g = hdr->stroke_g;
	el->line_b = hdr->stroke_b;
	el->line_a = hdr->stroke_a;

	el->line_width = hdr->stroke_w;

	el->angle  = hdr->angle;

	uint clk_cksum = 0;
	for (i = 0; i < len; i++) {
		uchar c = p[i];
		clk_cksum <<= ((uint)c) & 3;
		clk_cksum ^= clk_cksum >> 24;
		clk_cksum ^= (uint)c;
	}
	if (clk_cksum != cksum) {
		if (cnt_used)
			del_graphic_el(el); // <-TODO
			*cnt_used = 0;
		return NULL;
	}

	if (cnt_used)
		*cnt_used += sizeof(chunk_shape_hdr_t);

	uchar* gops = p;
	uint   gops_len = hdr->gops_len; //chunk_enum_graph_ops(gops, len);
	if (gops_len > 0) {
		el->data = malloc(gops_len);
		memcpy(el->data, gops, gops_len);
		el->data_len = gops_len;

		p += gops_len;

		len -= gops_len;
		if (cnt_used)
			*cnt_used += gops_len;
	}
	for (i = 0; i < len; /*i++*/) {
		uint  cnt_used_int = 0;
		uchar typ = p[i];
		graphic_el* sub_el;

		i++;
		//el = NULL;
		if (cnt_used)
			(*cnt_used)++;

		switch (typ) {
			case 'c':
				sub_el = chunk_new_shape_el(typ, &p[i], len - i, &cnt_used_int);
				i += cnt_used_int;

				if (cnt_used)
					*cnt_used += cnt_used_int;

				*subcanvas_nest =  sub_el;
				 subcanvas_nest = &sub_el->next;
				break;

			case 'm':
				sub_el = chunk_new_shape_el(typ, &p[i], len - i, &cnt_used_int);
				i += cnt_used_int;

				if (cnt_used)
					*cnt_used += cnt_used_int;

				*masked_nest =  sub_el;
				 masked_nest = &sub_el->next;
				break;

			default:
				if (cnt_used)
					(*cnt_used)--;
				break;
		}
	}

	return el;
}


static graphic_el* chunk_new_shape_el(uchar typ, uchar* p, uint len, uint* cnt_used)
{
	graphic_el* el = NULL;

	typ = p[0];
	p++;
	len--;
	if (cnt_used)
		(*cnt_used)++;

	switch (typ) {
		case 'v':
			el = do_new_svg_el(p, 0);
			if (cnt_used)
				*cnt_used += strlen(p) + 1;
			break;

		case 'c':
		case 's':
			el = chunk_parse_shape_el(typ, p, len, cnt_used);
			break;

//		default:
//			return;
	}

	return el;
}




static uchar* encToChunk_el(graphic_el *el, uchar* buf, uint len, uint* buf_size, uchar *type);

static uchar* encToChunk_el_list(uchar prefix, graphic_el *el, uchar* buf, uint len, uint* buf_size)
{
//DD("el: %p", el);
//	uint len_b = *buf_size;

	for (; el != NULL; el = el->next) {
		uint len_i = *buf_size;			 //{} ???
//DD("el: %p, len: %d", el, len_i);

		if (prefix != 0) {
			buf = realloc(buf, len + 1);

			buf[len_i] = prefix;
			len_i++;
			*buf_size = len_i;
			//(*buf_size)++; // = len_i;
		}

		buf = encToChunk_el(el, buf, len_i, buf_size, NULL);
	}

//	uint len_e = *buf_size;

/*
	uint i;
	uchar* p = &buf[len_b];

	len = len_e - len_b;

	uint clk_cksum = 0;
	for (i = 0; i < len; i++) {
		uchar c = p[i];
		clk_cksum <<= ((uint)c) & 3;
		clk_cksum ^= clk_cksum >> 24;
		clk_cksum ^= (uint)c;
	}
*/

	return buf;
}


static uchar* encToChunk_el(graphic_el *el, uchar* buf, uint len, uint* buf_size, uchar *type)
{
	if (!el) {
		return NULL;
	}

	uchar t = 0;
	uint  l = 0;

	switch (el->type & 0xff) {
		case GR_EL_SVG:
			t = 'v';
			l = strlen(el->data);

			buf = realloc(buf, *buf_size + l + 1 + 1);
			buf[*buf_size] = t;
			*buf_size += 1;

			memcpy(&buf[*buf_size], el->data, l);
			*buf_size += l;

			buf[*buf_size] = 0;
			*buf_size += 1;
			break;

		case GR_EL_CANVAS:
			t = 'c';
//			break;
		case GR_EL_SHAPE:
			if (!t)
				t = 's';
			{
				uint  i;
				uchar* gops;

				l = el->data_len;
				buf = realloc(buf, *buf_size + sizeof(chunk_shape_hdr_t) + 1 + l);

				buf[*buf_size] = t;
				(*buf_size)++;

				uint hdr_offs = *buf_size;
				chunk_shape_hdr_t* hdr = (chunk_shape_hdr_t*)&buf[hdr_offs];
				memset(hdr, 0, sizeof(chunk_shape_hdr_t));
				//hdr->len = l;
				//hdr->cksum = 0;

				*buf_size += sizeof(chunk_shape_hdr_t);

				uint pre_len = *buf_size;

				gops = &buf[*buf_size];

				if (l > 0) {
					memcpy(gops, el->data, l);

					*buf_size += l;
				}

				hdr->gops_len = l;

/*
				uint clk_cksum = 0;
				for (i = 0; i < l; i++) {
					uchar c = gops[i];
					clk_cksum <<= ((uint)c) & 3;
					clk_cksum ^= clk_cksum >> 24;
					clk_cksum ^= (uint)c;
				}
				hdr->cksum = clk_cksum;
*/

				hdr->fill_r = el->fill_r;
				hdr->fill_g = el->fill_g;
				hdr->fill_b = el->fill_b;
				hdr->fill_a = el->fill_a;
				
				hdr->stroke_r = el->line_r;
				hdr->stroke_g = el->line_g;
				hdr->stroke_b = el->line_b;
				hdr->stroke_a = el->line_a;
				
				hdr->stroke_w = el->line_width;
				
				hdr->angle = el->angle;

				if (el->masked) {
					buf = encToChunk_el_list('m', el->masked, buf, *buf_size, buf_size);
				}
				if (el->canvas) {
					buf = encToChunk_el_list('c', el->canvas, buf, *buf_size, buf_size);
				}

				uint post_len = *buf_size;

				len = post_len - pre_len;

				hdr = (chunk_shape_hdr_t*)&buf[hdr_offs];
				hdr->len = len;

				uchar* p = &buf[pre_len];
				
				uint clk_cksum = 0;
				for (i = 0; i < len; i++) {
					uchar c = p[i];
					clk_cksum <<= ((uint)c) & 3;
					clk_cksum ^= clk_cksum >> 24;
					clk_cksum ^= (uint)c;
				}
				hdr->cksum = clk_cksum;
			}
			break;

//		case GR_EL_PNG:
//			break;
	}

	if (type)
		*type = t;

	return buf;
}


void Shape_encToChunk(void* par)
{
	F_Shape_encToChunk *f = par;

	graphic_el *el;

	if (f->this->id > 0) {
		graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
		if (!el_nest) {
			//*f->ret = -1;
			return;
		}
		el = *el_nest;
	} else {
		Proc *p = getup();
		el = p->groot;
	}

	if (!el) {
		//*f->ret = -1;
		return;
	}

	chunk_hdr_t *hdr;

	uint buf_size = sizeof(chunk_hdr_t);
	uchar* buf = malloc(buf_size);
	memset(buf, 0, buf_size);

	hdr = (chunk_hdr_t*)buf;
	hdr->ver = 1;

	if (f->this->id == 0) {
		uint  i;
		uchar* gops;

		hdr->type = 'c';

		buf = realloc(buf, 1 + sizeof(chunk_shape_hdr_t));
		buf[buf_size] = 'c';
		buf_size++;

		uint hdr_el_offs = buf_size;
		chunk_shape_hdr_t* hdr_el = (chunk_shape_hdr_t*)&buf[hdr_el_offs];
		memset(hdr_el, 0, sizeof(chunk_shape_hdr_t));
		hdr_el->len = 0;
		hdr_el->cksum = 0;

		buf_size += sizeof(chunk_shape_hdr_t);

		hdr_el->fill_a   = 1.;
		hdr_el->stroke_a = 1.;

		hdr_el->angle = 0.;

		uint pre_len = buf_size;

		buf = encToChunk_el_list('c', el, buf, buf_size, &buf_size/*, &hdr->type*/);
		
		uint post_len = buf_size;
		hdr_el = (chunk_shape_hdr_t*)&buf[hdr_el_offs];
		hdr_el->len += post_len - pre_len;

		uchar* p = &buf[hdr_el_offs] + sizeof(chunk_shape_hdr_t);
		uint clk_cksum = 0;
		for (i = 0; i < hdr_el->len; i++) {
			uchar c = p[i];
			clk_cksum <<= ((uint)c) & 3;
			clk_cksum ^= clk_cksum >> 24;
			clk_cksum ^= (uint)c;
		}
		hdr_el->cksum = clk_cksum;
	} else {
		buf = encToChunk_el(el, buf, buf_size, &buf_size, &hdr->type);
	}

	hdr = (chunk_hdr_t*)buf;
	hdr->len = buf_size - sizeof(chunk_hdr_t);

	if (buf_size > 0) {
		Array *ar;
		ar = H2D(Array*, heaparray(&Tbyte, buf_size));

		memcpy(ar->data, buf, buf_size);

		free(buf);

		destroy(*f->ret);
		*f->ret = ar;
	}

}






void E_newFromChunk(void* par)
{
	F_E_newFromChunk *f = par;

	Array* chunk = f->chunk;
	if (chunk == NULL || chunk == nil || chunk == H)
		return;

	uchar *dat = chunk->data;
	uint   len = (uint)chunk->len;
	if (len <= sizeof(chunk_hdr_t) || dat == NULL || dat == H)
		return;

	chunk_hdr_t* hdr = (chunk_hdr_t*)dat;
	uchar ver = hdr->ver;
	uchar typ = hdr->type;

	uchar* p    = dat + sizeof(chunk_hdr_t);
		   len -= sizeof(chunk_hdr_t);


	graphic_el* el = chunk_new_shape_el(typ, p, len, NULL);

	if (el) {
		Proc *p 	 = getup();
			
		el->next = p->free_levels;
		p->free_levels = el;

		crRefShapeObj(el->id, f->ret);
	}
/*
	switch (typ) {
		case 'v':
			*f->ret = do_new_svg(p, *f->ret);
			break;

		case 's':
			{
				*f->ret = chunk_parse_shape(p, len - sizeof(chunk_hdr_t), *f->ret);
			}
			break;

		default:
			return;
	}
	if (o)
		*f->ret = o;
*/
}



void E_newSVG(void* par)
{
	F_E_newSVG *f = par;

	char *str;
	str = string2c(f->svg);

	*f->ret = do_new_svg(str, *f->ret);
D();
}



typedef struct png_closure_t {
    int fd;

	vlong offs;
} png_closure_t;

static cairo_status_t
png_chank_read_cb(void *closure, unsigned char *data, unsigned int length)
{
	png_closure_t* d = (png_closure_t*)closure;

	d->offs += kpread(d->fd, data, length, d->offs);

//    if (bytes_read != length)
//		return CAIRO_STATUS_READ_ERROR;

    return CAIRO_STATUS_SUCCESS;
}

void E_newPNG(void* par)
{
	F_E_newPNG *f = par;

	char *str;
	str = string2c(f->png_file_name);

	graphic_el *el = new_graphic_el(GR_EL_PNG, 1); 

	el->data = strdup(str);

	png_closure_t d;
	d.fd = kopen(el->data, OREAD);
	d.offs = 0;
	el->sfc = cairo_image_surface_create_from_png_stream(png_chank_read_cb, &d);
	kclose(d.fd);

#if 1
	cairo_status_t status;

    status = cairo_surface_status(el->sfc);
    if (status) {
//		cairo_test_status_t ret;

		cairo_surface_destroy (el->sfc);
		el->sfc = NULL;

		free(el->data);
		free(el);

	//	ret = cairo_test_status_from_status (ctx, status);
//		if (ret != CAIRO_TEST_NO_MEMORY) {
//		    cairo_test_log (ctx,
//				    "Error: failed to create surface from PNG: %s - %s\n",
//				    filename,
//				    cairo_status_to_string (status));
//		}

		return;
    }
#endif

	el->flags &= ~GR_EL_WAIT_TO_RENDER;
	el->flags |=  GR_EL_RENDERED;	

	crRefShapeObj(el->id, f->ret);
}



void E_newShape(void* par)
{
	F_E_newShape *f = par;

//	graphic_el *el = new_graphic_el(GR_EL_SHAPE);

//	crRefShapeObj(el->id, f->ret);
	*f->ret = do_new_shape(*f->ret);
D();
}



void Shape_MoveTo(void* par)
{
	F_Shape_MoveTo *f = par;
	
D();
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
D();
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		*f->ret = -1;
		return;
	}

	int st = el->data_len;
	el->data_len += 1 + 2 * sizeof(floatToChars_t);

	el->data = realloc(el->data, el->data_len);

	el->data[st++] = SHP_OP_MOVE_TO;

    int i;
	floatToChars_t tmp;

	tmp.f = (float)f->x;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	tmp.f = (float)f->y;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_LineTo(void* par)
{
	F_Shape_LineTo *f = par;
	
D();
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
D();
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		*f->ret = -1;
		return;
	}

	int st = el->data_len;
	el->data_len += 1 + 2 * sizeof(floatToChars_t);

	el->data = realloc(el->data, el->data_len);

	el->data[st++] = SHP_OP_LINE_TO;

    int i;
	floatToChars_t tmp;

	tmp.f = (float)f->x;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	tmp.f = (float)f->y;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_CurveTo(void* par)
{
	F_Shape_CurveTo *f = par;
	
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		*f->ret = -1;
		return;
	}

	int st = el->data_len;
	el->data_len += 1 + 3 * 2 * sizeof(floatToChars_t);

	el->data = realloc(el->data, el->data_len);

	el->data[st++] = SHP_OP_CURVE_TO;

    int i;
	floatToChars_t tmp;

	// xy 1
	tmp.f = (float)f->x1;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	tmp.f = (float)f->y1;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	// xy 2
	tmp.f = (float)f->x2;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	tmp.f = (float)f->y2;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	// xy 3
	tmp.f = (float)f->x3;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	tmp.f = (float)f->y3;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_ArcCW(void* par)
{
	F_Shape_ArcCW *f = par;
	
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		*f->ret = -1;
		return;
	}

	int st = el->data_len;
	el->data_len += 1 + (2 + 1 + 2) * sizeof(floatToChars_t);

	el->data = realloc(el->data, el->data_len);

	el->data[st++] = SHP_OP_ARC_CW;

    int i;
	floatToChars_t tmp;

	// xy
	tmp.f = (float)f->x;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	tmp.f = (float)f->y;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	// radius
	tmp.f = (float)f->radius;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	// angles: from - to
	tmp.f = (float)f->angle_from * M_PI / 180.0;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	tmp.f = (float)f->angle_to * M_PI / 180.0;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	reset_rendered_flag(&el);
	*f->ret = 0;
D();
}



void Shape_ArcCCW(void* par)
{
	F_Shape_ArcCCW *f = par;
	
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		*f->ret = -1;
		return;
	}

	int st = el->data_len;
	el->data_len += 1 + (2 + 1 + 2) * sizeof(floatToChars_t);

	el->data = realloc(el->data, el->data_len);

	el->data[st++] = SHP_OP_ARC_CCW;

    int i;
	floatToChars_t tmp;

	// xy
	tmp.f = (float)f->x;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	tmp.f = (float)f->y;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	// radius
	tmp.f = (float)f->radius;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	// angles: from - to
	tmp.f = (float)f->angle_from * M_PI / 180.0;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	tmp.f = (float)f->angle_to * M_PI / 180.0;
	for (i = 0; i < sizeof(floatToChars_t); i++) {
		el->data[st++] = tmp.c[i];
	}

	reset_rendered_flag(&el);
	*f->ret = 0;
D();
}



void Shape_setAsMask(void* par)
{
	F_Shape_setAsMask *f = par;
	
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		*f->ret = -1;
		return;
	}

	int st = el->data_len;
	el->data_len += 1;

	el->data = realloc(el->data, el->data_len);

	int isMask = f->is_mask;

	el->data[st++] = isMask ? SHP_OP_toMASK : SHP_OP_unMASK;

	if (isMask) {
		el->type |= GR_EL_MASK;
	} else {
		el->type &= ~GR_EL_MASK;
	}

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_ClosePath(void* par)
{
	F_Shape_ClosePath *f = par;

	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		*f->ret = -1;
		return;
	}

	int st = el->data_len;
	el->data_len += 1;

	el->data = realloc(el->data, el->data_len);

	el->data[st++] = SHP_OP_CLOSE_PATH;

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_LineWidth(void* par)
{
	F_Shape_LineWidth *f = par;
	
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		*f->ret = -1;
		return;
	}

	el->line_width = (float)f->w;

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_LineColor(void* par)
{
	F_Shape_LineColor *f = par;
	
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		*f->ret = -1;
		return;
	}

	el->line_r = (float)f->r;
	el->line_g = (float)f->g;
	el->line_b = (float)f->b;
	el->line_a = (float)f->a;

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_FillColor(void* par)
{
	F_Shape_FillColor *f = par;
	
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		*f->ret = -1;
		return;
	}

	el->fill_r = (float)f->r;
	el->fill_g = (float)f->g;
	el->fill_b = (float)f->b;
	el->fill_a = (float)f->a;

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_setAngle(void* par)
{
	F_Shape_setAngle *f = par;
	
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		*f->ret = -1;
//		return;
//	}

	float a = f->angle * M_PI / 180.0;
	if (a != el->angle) {
		el->angle = a;

		reset_rendered_flag(el->root);
	}
	*f->ret = 0;
}



void Shape_setScale(void* par)
{
	F_Shape_setScale *f = par;
	
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		*f->ret = -1;
//		return;
//	}

	if (el->sx != f->sx || el->sy != f->sy) {
		el->sx = f->sx;
		el->sy = f->sy;

		reset_rendered_flag(el->root);
	}
	*f->ret = 0;
}



void Shape_setOffset(void* par)
{
	F_Shape_setOffset *f = par;
	
	graphic_el **el_nest = find_graph_el_by_id(f->this->id, NULL);
	if (!el_nest) {
		*f->ret = -1;
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		*f->ret = -1;
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		*f->ret = -1;
//		return;
//	}

	if (el->x != f->x || el->y != f->y) {
		el->x = f->x;
		el->y = f->y;

		reset_rendered_flag(el->root);
	}
	*f->ret = 0;
}



void Shape_setDrawZ(void* par)
{
	F_Shape_setDrawZ *f = par;

	int parent_id = f->this->id;
	int id        = f->shape->id;
	int n  = f->n; 

	if (id <= 0) {
		f->ret->t0 = -1;
		f->ret->t1 = -1;
		return;
	}

	graphic_el **parent_el_nest = find_graph_el_by_id(parent_id, NULL);
	graphic_el **el_nest        = find_graph_el_by_id(id, NULL);
	if (el_nest == NULL) {
		f->ret->t0 = -1;
		f->ret->t1 = -1;
		return;
	}

	graphic_el *el = *el_nest;
	if (el == NULL) {
		f->ret->t0 = 0;
		f->ret->t1 = -1;
		return;
	}

if (parent_el_nest && *parent_el_nest) {
	DD("chld: type = 0x%x (0x%x)", (*parent_el_nest)->type, GR_EL_CANVAS);
}else {
	DD("main: id: %d", parent_id);
}

	if (el && (parent_id == 0 
				|| (*parent_el_nest && (    ((*parent_el_nest)->type & GR_EL_MASK)
				                         || (((*parent_el_nest)->type & 0xff) == GR_EL_CANVAS)
				                       )
	               )
	           )
	) {
		*el_nest = el->next;

D();
		graphic_el* old_shape = set_graph_el_to_pos(parent_el_nest, el, n, 0);
		if (old_shape) {
			f->ret->t0 = old_shape->id; //el2->id;
		} else {
			f->ret->t0 = 0;
		}
	}

	f->ret->t1 = 0;
}


void Shape_insDrawZ(void* par)
{
	F_Shape_insDrawZ *f = par;
	
	int parent_id = f->this->id;
	int id        = f->shape->id;
	int n  = f->n; 

	if (id <= 0) {
		f->ret->t0 = -1;
		f->ret->t1 = -1;
		return;
	}

	graphic_el **parent_el_nest = find_graph_el_by_id(parent_id, NULL);
	graphic_el **el_nest        = find_graph_el_by_id(id, NULL);
	if (el_nest == NULL) {
		f->ret->t0 = -1;
		f->ret->t1 = -1;
		return;
	}

	graphic_el *el = *el_nest;
	if (el == NULL) {
		f->ret->t0 = 0;
		f->ret->t1 = -1;
		return;
	}

D();
	if (el && (parent_id == 0 
				|| (*parent_el_nest && (*parent_el_nest)->type & GR_EL_MASK)
				|| (*parent_el_nest && (*parent_el_nest)->type & 0xff == GR_EL_CANVAS)
	)) {
		*el_nest = el->next;

D();
		graphic_el* old_shape = set_graph_el_to_pos(parent_el_nest, el, n, 1);
		if (old_shape) {
			f->ret->t0 = old_shape->id; //el2->id;
		} else {
			f->ret->t0 = 0;
		}
	}

	f->ret->t1 = 0;
}


void Shape_getDrawZ(void* par)
{
	F_Shape_getDrawZ *f = par;
	
	int parent = f->this->id;
//	int id     = f->shape->id;
}


void Shape_rmDrawZ(void* par)
{
	F_Shape_rmDrawZ *f = par;
	
	int parent = f->this->id;
//	int id     = f->shape->id;
}




void E_moveTo(void* par)
{
	F_E_moveTo *f = par;

	
//D();
	graphic_el **el_nest = find_graph_el_by_id(f->id, NULL);
	if (!el_nest)
		return;
//D();
	graphic_el *el = *el_nest;
	if (!el)
		return;

//D();
	el->x = f->x;
	el->y = f->y;
//D();

	main_screen_flags &= ~GR_EL_RENDERED;
	main_screen_flags |=  GR_EL_WAIT_TO_RENDER;
}


#if 0
void E_setToPos(void* par)
{
	F_E_setToPos *f = par;
	int id = f->id;
	int n  = f->n; 

//D();
	if (id <= 0) {
		*f->ret = -1;
		return;
	}

//D();
	graphic_el **el_nest = find_graph_el_by_id(id, NULL);
	if (el_nest == NULL) {
		*f->ret = -1;
		return;
	}		

//D();
	graphic_el *el = *el_nest;
	if (el == NULL) {
		*f->ret = 0;
		return;
	}		

//D();
	if (el) {
		*el_nest = el->next;
//D();

		int ret_id = set_graph_el_to_pos(el, n, 0);
/*
		Proc *p = getup();
	
		el_nest = &p->groot;
		graphic_el *el2 = p->groot;

		for (; n >= 0; n--) {
			if (el2 == NULL) {
				el2 = malloc(sizeof(graphic_el));
				memset(el2, 0, sizeof(graphic_el));

				*el_nest = el2;
			}

			el_nest = &el2->next;
			el2 = *el_nest;
		}

		if (el->id != el2->id) {
			el->next = el2->next;
			*el_nest = el;

			if (el2->id <= 0) {
				free(el2);
				*f->ret = 0;
				return;
			}

			el2->next = p->free_levels;
			p->free_levels = el2;
*/
//D();
		if (ret_id > 0) {
			*f->ret = ret_id; //el2->id;
			return;
		}
	}

	
	*f->ret = 0;
//D();
}


void E_insToPos(void* par)
{
	F_E_insToPos *f = par;
	
	int id = f->id;
	int n  = f->n; 

	if (id <= 0) {
		*f->ret = -1;
		return;
	}

	graphic_el **el_nest = find_graph_el_by_id(id, NULL);
	if (el_nest == NULL) {
		*f->ret = -1;
		return;
	}		

	graphic_el *el = *el_nest;
	if (el == NULL) {
		*f->ret = 0;
		return;
	}		

	if (el) {
		*el_nest = el->next;

		int ret_id = set_graph_el_to_pos(el, n, 1);

		if (ret_id > 0) {
			*f->ret = ret_id; //el2->id;
			return;
		}
	}	
		
	*f->ret = 0;
}

void E_getFromPos(void* par)
{
	F_E_getFromPos *f = par;
}

void E_getLevelsTopPos(void* par)
{
}

void E_rmFromPos(void* par)
{
	F_E_rmFromPos *f = par;
}
#endif

void E_destroyObj(void* par)
{
	F_E_destroyObj *f = par;
}



void E_getLevelsList(void* par)
{
	struct F_E_getLevelsList *p = par;
	Proc *prc = getup();

	Array *lvls = *p->ret;

print("lev lst ar: %p, len: %d\n", lvls->data, lvls->len);

	lvls->data = prc->groot;
	lvls->len  = prc->groot_len;
}







void E_doRender(void* par)
{
D();
	if (main_screen_flags & GR_EL_WAIT_TO_RENDER) {
D();
		render_gr_stack_to_main_screen();

		main_screen_flags &= ~GR_EL_WAIT_TO_RENDER;
		main_screen_flags |=  GR_EL_RENDERED;
	}

D();
	flushscreen();
D();
}












void E_increment(void* par)
{
	F_E_increment *p = par;

	int i = p->i;

//	void *tmp = *p->ret;
//	*p->ret = H;
//	destroy(tmp);
	*p->ret = i + 1;
}


void E_say(void* par)
{
	F_E_say *p = par;

	String *s_inf = p->s;

	char *s = string2c(s_inf);

	print(">::> %s\n", s);
}



void E_tst(void* par)
{
}


