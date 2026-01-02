

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



typedef struct {
    char *stylename;
    char *fontname;
    char *fontname_b;
    char *fontname_i;
    char *fontname_bi;
    float  fontsize;
    uint32_t color1;
    uint32_t color2;
    uint32_t border_color;
    uint32_t shadow_color;
    int  bold;
    int  italic;
    int  underline;
    int  strikeout;
    int  fontscalex;
    int  fontscaley;
    int  spacing;
    float  angle;

    int borderstyle;
    float outline;
    float shadow;
    int alignment;
    float marginleft;
    float marginright;
    float marginvertical;
    char  *encoding;
    float alphaLevel;
} style_t;




#if 1
cairo_surface_t *sfc_main_screen = NULL;
uint             main_screen_flags = 0;


static cairo_surface_t *render_canvas(graphic_el* el);


int _text_with_draw_cb(cairo_t* tcr, char *text, style_t* stl);


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
	sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 2000 /*Xsize_ / 2*/, 2000 /*Ysize_ / 2*/);

	cr = cairo_create(sfc);

//	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
//	cairo_paint(cr);

//	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	int isMask = 0;
	int isPaint = 1;

//DD("data: %p, len: %d", el->data, el->data_len);

	cairo_new_path(cr);

	for (i = 0; i < el->data_len;) {
		char key = el->data[i++];

		switch (key) {
			case SHP_OP_MOVE_TO:
				{
					float x, y;
//D();

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
//D();

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
//D();

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
//D();

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
//D();

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
//D();
				cairo_close_path (cr);
				break;

			case SHP_OP_TEXT:
				{
					float x, y, r, a_fr, a_to;
//D();

					char* txt = &(el->data[i]);
					for (; el->data[i] != '\0'; i++);
					
//					style_t* stl = new_style();
//cairo_translate(cr, -20., -25.);
//					_text_with_draw_cb(cr, txt, stl);
//					free(stl);

					isPaint = 0;

					//cairo_arc_negative(cr, x, y, r, a_fr, a_to);
				}
				break;

			case SHP_OP_toMASK:
//D();
				isMask = 1;
				break;

			case SHP_OP_unMASK:
//D();
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

//DD("w: %f (%f - %f), h: %f (%f - %f)", 
//		(float)el->w, (float)el->x_max, (float)el->x_min,
//		(float)el->h, (float)el->y_max, (float)el->y_min);
	cairo_destroy(cr);
	cairo_surface_destroy(sfc);


	sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, el->w, el->h);

	cr = cairo_create(sfc);

	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	double dx = el->x_min;
	double dy = el->y_min;
//DD("path->num_data: %d", path->num_data);

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

//D();
//	cairo_move_to(cr, -el->x_min, el->y_min);
	cairo_append_path(cr, path);

//D();
	if (isMask) {
//D();
		cairo_move_to(cr, el->x_min, el->y_min);
		cairo_clip    (cr);
		cairo_new_path(cr); /* path not consumed by clip()*/

//--VV
		graphic_el* el2 = el->masked;
		for (; el2 != NULL; el2 = el2->next) {
			cairo_matrix_t matrix;
//D();
			if (el2->flags & GR_EL_WAIT_TO_RENDER) {
//D();
				if (el2->sfc) {
					cairo_surface_destroy(el2->sfc);
					el2->sfc = NULL;
				}

				if ((el2->type & 0xff) == GR_EL_SHAPE) {
//D();
					el2->sfc = render_shape(el2);
				}
			}

//D();
			if (el2->flags & GR_EL_RENDERED && el2->sfc != NULL) {
//D();
#if 1
				cairo_pattern_t *pat = cairo_pattern_create_for_surface(el2->sfc);
				cairo_pattern_set_filter(pat, CAIRO_FILTER_GOOD);

//DD("scale sxy: %f, %f", el2->sx, el2->sy);
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
//D();

	} else if (isPaint) {
//D();
		cairo_set_source_rgba(cr, el->fill_r, el->fill_g, el->fill_b, el->fill_a);
		cairo_fill_preserve  (cr);
		cairo_set_source_rgba(cr, el->line_r, el->line_g, el->line_b, el->line_a);
		cairo_set_line_width (cr, el->line_width);
		cairo_stroke         (cr);
	}
//D();

	cairo_path_destroy(path);
//D();
	cairo_destroy(cr);
//D();
	el->flags &= ~GR_EL_WAIT_TO_RENDER;
	el->flags |=  GR_EL_RENDERED;

//D();
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
//DD("Before Draw Sub-Canvas");
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

//DD("subCanvas: w: %f (%f - %f), h: %f (%f - %f)", 
//					(float)w, (float)x2, (float)x1,
//					(float)h, (float)y2, (float)y1);

	cairo_surface_t *sfc3 = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cairo_t *cr3 = cairo_create(sfc3);

	cairo_set_source_surface (cr3, sfc2, -x1, -y1);   //
	cairo_paint(cr3);								   //

	cairo_destroy(cr3);

	cairo_surface_destroy(sfc2); //

	return sfc3; //
}



static void thread_draw_gr_stack_el(void* par)
{
	graphic_el* el = par;
	if (!el)
		return;

	if (el->sfc) {
		cairo_surface_destroy(el->sfc);
		el->sfc = NULL;
	}

//DD("el->type & 0xff = %x, el->data : %p", el->type & 0xff, el->data);

	switch (el->type & 0xff) {
	case GR_EL_SHAPE:
//D();
		el->sfc = render_shape(el);
		break;
	
	case GR_EL_SVG:
//D();
		el->sfc = _draw_svg(el->data, &el->w, &el->h);

		el->x_min = 0.;
		el->x_max = el->w;
		el->y_min = 0.;
		el->y_max = el->h;

		el->flags &= ~GR_EL_WAIT_TO_RENDER;
		el->flags |=  GR_EL_RENDERED;
//D();
		break;

	case GR_EL_TEXT:
//D();
		double w, h;
		el->sfc = _draw_text(el, &w, &h);

		el->w = (double)w;
		el->h = (double)h;

		el->x_min = 0.;
		el->y_min = 0.;

		el->x_max = el->w;
		el->y_max = el->h;

		el->flags &= ~GR_EL_WAIT_TO_RENDER;
		el->flags |=  GR_EL_RENDERED;
//D();
		break;

	case GR_EL_CANVAS:
//D();
		el->sfc = render_canvas(el);

		el->flags &= ~GR_EL_WAIT_TO_RENDER;
		el->flags |=  GR_EL_RENDERED;
		break;

	}
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

	
	for (; el != NULL; el = el->next) {
//		cairo_matrix_t matrix;
//D();
		if (el->flags & GR_EL_WAIT_TO_RENDER) {
//D();
			kproc("", thread_draw_gr_stack_el, (void*)el, KPDUPPG | KPDUPFDG | KPDUPENVG);
		}
	}

//D();
	el = gr_stack_root;

	for (; el != NULL; el = el->next) {
		cairo_matrix_t matrix;
//D();
		if (el->flags & GR_EL_WAIT_TO_RENDER) {
//D();
			while (el->flags & GR_EL_WAIT_TO_RENDER)
				osyield();
/*
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
*/
		}

//D();
		if (el->flags & GR_EL_RENDERED && el->sfc != NULL) {

			if ((el->type & 0xff) == GR_EL_TEXT) {
				DD("(el->type & 0xff): %d, el->sfc: %p", (el->type & 0xff), el->sfc);
			}
//D();
//			double whlf = el->w / 2.; ///*el->x_min +*/ (double)cairo_image_surface_get_width(el->sfc) / 2.;
//			double hhlf = el->h / 2.; ///*el->y_min +*/ (double)cairo_image_surface_get_height(el->sfc) / 2.;
			double whlf = /*el->x_min +*/ (double)cairo_image_surface_get_width(el->sfc) / 2.;
			double hhlf = /*el->y_min +*/ (double)cairo_image_surface_get_height(el->sfc) / 2.;

			double xc, yc;
//			cairo_matrix_transform_point(&matrix, &xc, &yc);
			xc = - el->x_min - whlf/**/ - el->x;
			yc = - el->y_min - hhlf/**/ - el->y;

#if 0
			//cairo_translate(cr, -whlf, -hhlf);
			cairo_scale    (cr, el->sx, el->sy);
			cairo_rotate   (cr, -el->angle);
			//cairo_translate(cr, xc, yc);

			cairo_set_source_surface(cr, el->sfc, -xc, -yc);
#else
			cairo_pattern_t *pat = cairo_pattern_create_for_surface(el->sfc);
			cairo_pattern_set_filter(pat, CAIRO_FILTER_GOOD);

//DD("sxy: %f, %f", el->sx, el->sy);
//DD("-------> WHimg: (%f, %f), WH(%f, %f)", (float)whlf * 2., (float)hhlf * 2., (float)el->w, (float)el->h);

			cairo_matrix_init_translate(&matrix, whlf, hhlf);
			cairo_matrix_scale(&matrix, el->sx, el->sy);
//			cairo_matrix_init_scale(&matrix, scale, scale);
//DD("min_xy: (%f, %f), yx: (%f, %f)", (float)el->x_min, (float)el->y_min, (float)el->x, (float)el->y);
			cairo_matrix_rotate(&matrix, el->angle);



//			cairo_matrix_t mat2;
//			cairo_matrix_init_translate(&mat2, -el->x_min - whlf - el->x, -el->y_min - hhlf - el->y);
//			cairo_matrix_init_translate(&mat2, xc, yc);
			cairo_matrix_translate(&matrix, xc, yc);

//			cairo_matrix_multiply(&matrix, &matrix, &mat2);

			cairo_pattern_set_matrix(pat, &matrix);

			cairo_set_source(cr, pat);
			cairo_pattern_destroy(pat);
#endif
			cairo_paint(cr);
//D();
		}
	}

//	cairo_destroy(cr);
//D();
//	cairo_surface_destroy(sfc_main_screen);
}


void render_gr_stack_to_main_screen()
{
	if (sfc_main_screen == NULL)
		sfc_main_screen = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, Xsize_, Ysize_);

	if (sfc_main_screen == NULL)
		return;

	Proc* p = getup();

	cairo_t *cr = cairo_create(sfc_main_screen);
	render_gr_stack_to_surface(sfc_main_screen, cr, p->groot);
	cairo_destroy(cr);

//D();
	uint32 *pixel = graph_scrn_data;

	unsigned char* buf = cairo_image_surface_get_data(sfc_main_screen);
	uint32_t *buf32 = (uint32_t*)buf;
	
//D();
	for (uint32_t x = 0; x < Ysize_; x++) {
		for (uint32_t y = 0; y < Xsize_; y++) {
			*pixel++ = *buf32++;
		}
//		pixel += (Xsize_ - w_out);
	}
//D();
}

#endif


void reset_rendered_flag(graphic_el **el)
{
	if (el && *el) {
		(*el)->flags &= ~GR_EL_RENDERED;
		(*el)->flags |=  GR_EL_WAIT_TO_RENDER;

		graphic_el **e = (*el)->root;
		for (; e && *e; e = (*e)->root) {
			(*e)->flags &= ~GR_EL_RENDERED;
			(*e)->flags |=  GR_EL_WAIT_TO_RENDER;
		}
	}

	main_screen_flags &= ~GR_EL_RENDERED;
	main_screen_flags |=  GR_EL_WAIT_TO_RENDER;
}


