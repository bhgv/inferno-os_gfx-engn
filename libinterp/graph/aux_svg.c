

#include <lib9.h>
#include <isa.h>
#include <interp.h>

#include "../e.h"
//#include "../emod.h"

#undef Unknown
#include "../../emu/port/dat.h"
#include "../../emu/port/fns.h"
#include "kernel.h"

#include "graph.h"

#include "cairo-features.h"
#include <cairo.h>
#include <svg-cairo.h>

#define _USE_MATH_DEFINES
#include <math.h>



cairo_surface_t *
_draw_svg(char* svg_buf, double *w_out, double *h_out)
{
	if (!svg_buf)
		return NULL;


/*	
	D();
		uint32_t *pixel = static_cast<uint32_t *>(mappedAddr_); // 使用mmap获取到的地址来访问内存
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



