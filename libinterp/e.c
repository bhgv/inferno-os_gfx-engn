


#include <lib9.h>
#include <isa.h>
#include <interp.h>

#include "e.h"
#include "emod.h"

#undef Unknown
#include "../emu/port/dat.h"
//#include "../emu/port/fns.h"
#include "kernel.h"

#include "graph/graph.h"

#define _USE_MATH_DEFINES
#include <math.h>


static uchar	Level_map[] = E_Level_map;
Type*	Tlevel;


static uchar	GraphOp_MOVE_TO_map[] = E_GraphOp_MOVE_TO_map;
Type*	TGraphOp_MOVE_TO;
static uchar	GraphOp_LINE_TO_map[] = E_GraphOp_LINE_TO_map;
Type*	TGraphOp_LINE_TO;
static uchar	GraphOp_CURVE_TO_map[] = E_GraphOp_CURVE_TO_map;
Type*	TGraphOp_CURVE_TO;
static uchar	GraphOp_ARC_CW_map[] = E_GraphOp_ARC_CW_map;
Type*	TGraphOp_ARC_CW;
static uchar	GraphOp_ARC_CCW_map[] = E_GraphOp_ARC_CCW_map;
Type*	TGraphOp_ARC_CCW;
static uchar	GraphOp_CLOSE_PATH_map[] = E_GraphOp_CLOSE_PATH_map;
Type*	TGraphOp_CLOSE_PATH;

static uchar	GraphOp_TEXT_map[] = E_GraphOp_TEXT_map;
Type*	TGraphOp_TEXT;

static uchar	ShapeEvent_MOUSE_map[] = E_ShapeEvent_MOUSE_map;
Type*	TShapeEvent_MOUSE;
static uchar	ShapeEvent_TOUCH_map[] = E_ShapeEvent_TOUCH_map;
Type*	TShapeEvent_TOUCH;

static uchar	Shape_map[] = E_Shape_map;
Type*	TShape;

static uchar	XY_coord_map[] = E_XY_coord_map;
Type*	TXY_coord;



String* s_moveTo;
String* s_lineTo;
String* s_curveTo;
String* s_arcCW;
String* s_arcCCW;
String* s_text;
String* s_closePath;




//static void heap_ref_inc(Heap* ho)
//{
//	ho->ref++;
//	Setmark(ho);
//}



void
emodinit(void)
{
        builtinmod("$E", Emodtab, Emodlen);
D();
//		Tlevel = dtype(freeheap, sizeof(struct E_Level), Levelmap, sizeof(Levelmap));
		Tlevel = dtype(freeheap, E_Level_size, Level_map, sizeof(Level_map));

		TGraphOp_MOVE_TO = dtype(freeheap, E_GraphOp_MOVE_TO_size, GraphOp_MOVE_TO_map, sizeof(GraphOp_MOVE_TO_map));
		TGraphOp_LINE_TO = dtype(freeheap, E_GraphOp_LINE_TO_size, GraphOp_LINE_TO_map, sizeof(GraphOp_LINE_TO_map));
		TGraphOp_CURVE_TO = dtype(freeheap, E_GraphOp_CURVE_TO_size, GraphOp_CURVE_TO_map, sizeof(GraphOp_CURVE_TO_map));
		TGraphOp_ARC_CW = dtype(freeheap, E_GraphOp_ARC_CW_size, GraphOp_ARC_CW_map, sizeof(GraphOp_ARC_CW_map));
		TGraphOp_ARC_CCW = dtype(freeheap, E_GraphOp_ARC_CCW_size, GraphOp_ARC_CCW_map, sizeof(GraphOp_ARC_CCW_map));
		TGraphOp_CLOSE_PATH = dtype(freeheap, E_GraphOp_CLOSE_PATH_size, GraphOp_CLOSE_PATH_map, sizeof(GraphOp_CLOSE_PATH_map));

		TGraphOp_TEXT = dtype(freeheap, E_GraphOp_TEXT_size, GraphOp_TEXT_map, sizeof(GraphOp_TEXT_map));

//		TGraphOp = dtype(freeheap, E_GraphOp_MOVE_TO_size, GraphOp_map, sizeof(GraphOp_map));

		TShapeEvent_MOUSE = dtype(freeheap, E_ShapeEvent_MOUSE_size, ShapeEvent_MOUSE_map, sizeof(ShapeEvent_MOUSE_map));
		TShapeEvent_TOUCH = dtype(freeheap, E_ShapeEvent_TOUCH_size, ShapeEvent_TOUCH_map, sizeof(ShapeEvent_TOUCH_map));

		TShape = dtype(freeheap, E_Shape_size, Shape_map, sizeof(Shape_map));

		TXY_coord = dtype(freeheap, E_XY_coord_size, XY_coord_map, sizeof(XY_coord_map));



		s_moveTo    = c2string("moveTo",    strlen("moveTo"));
		poolimmutable(D2H(s_moveTo));
		s_lineTo    = c2string("lineTo",    strlen("lineTo"));
		poolimmutable(D2H(s_lineTo));
		s_curveTo   = c2string("curveTo",   strlen("curveTo"));
		poolimmutable(D2H(s_curveTo));
		s_arcCW     = c2string("arcCW",     strlen("arcCW"));
		poolimmutable(D2H(s_arcCW));
		s_arcCCW    = c2string("arcCCW",    strlen("arcCCW"));
		poolimmutable(D2H(s_arcCCW));
		s_closePath = c2string("closePath", strlen("closePath"));
		poolimmutable(D2H(s_closePath));

		s_text    = c2string("text",    strlen("text"));
		poolimmutable(D2H(s_text));

		graph_scrn_init();

//D();
		Proc *p      = getup();
		p->groot     = nil;
		p->groot_len = 0;
		p->free_levels = nil;

//D();
//		groot = NULL;
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
//	lvl->graphics.fill = H;

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



void E_rootCanvas(void* par)
{
	F_E_rootCanvas *f = par;

	crRefShapeObj(0, f->ret);
}


void E_newCanvas(void* par)
{
	F_E_newCanvas *f = par;

	graphic_el* el = new_graphic_el(GR_EL_CANVAS, 1);

	crRefShapeObj(el->id, f->ret);

	el->shp = *f->ret;
D();
}


void Shape_encToChunk(void* par)
{
	F_Shape_encToChunk *f = par;

	graphic_el *el;

	if (f->this == H || f->this == nil)
		return;

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

		el->shp = *f->ret;
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
//D();
}


void E_newPNG(void* par)
{
	F_E_newPNG *f = par;

	char *str;
	str = string2c(f->png_file_name);

	graphic_el *el = new_graphic_el(GR_EL_PNG, 1); 

	el->data = strdup(str);

	el->sfc = _draw_png(el->data, &el->w, &el->h);

#if 1
    if (el->sfc == NULL) {
		free(el->data);
		free(el);

		return;
    }
#endif

	el->flags &= ~GR_EL_WAIT_TO_RENDER;
	el->flags |=  GR_EL_RENDERED;	

	crRefShapeObj(el->id, f->ret);

	el->shp = *f->ret;
}



void E_newText(void* par)
{
	F_E_newText *f = par;

	char *str;
	str = string2c(f->txt);

	graphic_el *el = new_graphic_el(GR_EL_TEXT, 1); 

	el->data = strdup(str);

#if 0
	el->text_block_flags = 1 << TEXT_BLOCK_ALIGN_CENTER;

	double w, h;
	el->sfc = _draw_text(el, &w, &h);

	el->w = (double)w;
	el->h = (double)h;

	el->x_min = 0.;
	el->y_min = 0.;

	el->x_max = el->w;
	el->y_max = el->h;

#if 1
    if (el->sfc == NULL) {
		free(el->data);
		free(el);

		return;
    }
#endif

	el->flags &= ~GR_EL_WAIT_TO_RENDER;
	el->flags |=  GR_EL_RENDERED;	
#endif

	crRefShapeObj(el->id, f->ret);

	el->shp = *f->ret;
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
	
	if (f->this == H || f->this == nil)
		return;

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

//D();
	add_op_to_shape(f->this, E_GraphOp_MOVE_TO, f->x, f->y, 0., 0., 0., 0.);

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_LineTo(void* par)
{
	F_Shape_LineTo *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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

//D();
	add_op_to_shape(f->this, E_GraphOp_LINE_TO, f->x, f->y, 0., 0., 0., 0.);

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_CurveTo(void* par)
{
	F_Shape_CurveTo *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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

//D();
	add_op_to_shape(f->this, E_GraphOp_CURVE_TO, f->x1, f->y1, f->x2, f->y2, f->x3, f->y3);

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_ArcCW(void* par)
{
	F_Shape_ArcCW *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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

//D();
	add_op_to_shape(f->this, E_GraphOp_ARC_CW, f->x, f->y, f->radius, f->angle_from, f->angle_to, 0.);

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_ArcCCW(void* par)
{
	F_Shape_ArcCCW *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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

//D();
	add_op_to_shape(f->this, E_GraphOp_ARC_CCW, f->x, f->y, f->radius, f->angle_from, f->angle_to, 0.);

	reset_rendered_flag(&el);
	*f->ret = 0;
}



#include "graph/utf8.h"

#if 0
void Shape_Text(void* par)
{
	F_Shape_Text *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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

	char* txt     = string2c(f->txt);
	int   txt_len = utf8size(txt);

	int st = el->data_len;
	el->data_len += 1 + txt_len;

	el->data = realloc(el->data, el->data_len);

	el->data[st++] = SHP_OP_TEXT;

	memcpy(&el->data[st], txt, txt_len);

//D();
	add_op_text_to_shape(f->this, E_GraphOp_TEXT, txt, txt_len);

	reset_rendered_flag(&el);
	*f->ret = 0;
}
#endif


void Shape_textString(void* par)
{
	F_Shape_textString *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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
	if ((el->type & 0xff) != GR_EL_TEXT) {
		*f->ret = -1;
		return;
	}

	char* txt     = string2c(f->txt);
	//int   txt_len = utf8size(txt);

	free(el->data);
	el->data = strdup(txt);

	text_line_t* tl = el->text_lines;
	while (tl) {
		tl = text_del_line(tl);
	}
	el->text_lines = tl;

	reset_rendered_flag(&el);
	*f->ret = 0;
}


void Shape_textFont(void* par)
{
	F_Shape_textFont *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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
	if ((el->type & 0xff) != GR_EL_TEXT) {
		*f->ret = -1;
		return;
	}

	char* font_path = string2c(f->path);
	//int   txt_len = utf8size(txt);

	if (el->text_font_path)
		free(el->text_font_path);
	el->text_font_path = strdup(font_path);

	text_line_t* tl = el->text_lines;
	while (tl) {
		tl = text_del_line(tl);
	}
	el->text_lines = tl;

	reset_rendered_flag(&el);
	*f->ret = 0;
}


void Shape_textFontSize(void* par)
{
	F_Shape_textFontSize *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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
	if ((el->type & 0xff) != GR_EL_TEXT) {
		*f->ret = -1;
		return;
	}

	uint size = f->size;

	el->text_font_size = size;

	text_line_t* tl = el->text_lines;
	while (tl) {
		tl = text_del_line(tl);
	}
	el->text_lines = tl;

	reset_rendered_flag(&el);
	*f->ret = 0;
}


void Shape_textAlign(void* par)
{
	F_Shape_textAlign *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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
	if ((el->type & 0xff) != GR_EL_TEXT) {
		*f->ret = -1;
		return;
	}

	char* al     = string2c(f->align);
	//int   txt_len = utf8size(txt);

	if (strcmp(al, "left") == 0) {
		el->text_block_flags &= ~(TEXT_BLOCK_ALIGN_LEFT | TEXT_BLOCK_ALIGN_CENTER | TEXT_BLOCK_ALIGN_RIGHT);
		el->text_block_flags |=   TEXT_BLOCK_ALIGN_LEFT;
	} else
	if (strcmp(al, "center") == 0) {
		el->text_block_flags &= ~(TEXT_BLOCK_ALIGN_LEFT | TEXT_BLOCK_ALIGN_CENTER | TEXT_BLOCK_ALIGN_RIGHT);
		el->text_block_flags |=   TEXT_BLOCK_ALIGN_CENTER;
	} else
	if (strcmp(al, "right") == 0) {
		el->text_block_flags &= ~(TEXT_BLOCK_ALIGN_LEFT | TEXT_BLOCK_ALIGN_CENTER | TEXT_BLOCK_ALIGN_RIGHT);
		el->text_block_flags |=   TEXT_BLOCK_ALIGN_RIGHT;
	}

	reset_rendered_flag(&el);
	*f->ret = 0;
}


void Shape_textViewPort(void* par)
{
	F_Shape_textViewPort *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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
	if ((el->type & 0xff) != GR_EL_TEXT) {
		*f->ret = -1;
		return;
	}

	int w = f->w;
	int h = f->h;

	el->text_vp_w = w;
	el->text_vp_h = h;

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_setAsMask(void* par)
{
	F_Shape_setAsMask *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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

	if (f->this == H || f->this == nil)
		return;

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

//D();
	add_op_to_shape(f->this, E_GraphOp_CLOSE_PATH, 0., 0., 0., 0., 0., 0.);

	reset_rendered_flag(&el);
	*f->ret = 0;
}



void Shape_updGraph(void* par)
{
	int i, j, k;

	F_Shape_updGraph *f = par;

	E_Shape* shp = f->this;
	
	if (f->this == H || f->this == nil)
		return;

	graphic_el **el_nest = find_graph_el_by_id(shp->id, NULL);
	if (!el_nest) {
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		return;
	}

	Array* ar_ops = shp->graph_ops;

	if (ar_ops->len <= 0) {
		return;
	}

	uint    ops_cnt = ar_ops->len;
	E_GraphOp** ops = (E_GraphOp**)ar_ops->data;

	uint sz = 0;

	for (i = 0; i < ops_cnt; i++) {
		E_GraphOp* op = ops[i];
		switch (op->pick) {
			case E_GraphOp_MOVE_TO:
				sz += 1 + 2 * sizeof(floatToChars_t);
				break;

			case E_GraphOp_LINE_TO:
				sz += 1 + 2 * sizeof(floatToChars_t);
				break;

			case E_GraphOp_CURVE_TO:
				sz += 1 + 3 * 2 * sizeof(floatToChars_t);
				break;

			case E_GraphOp_ARC_CW:
				sz += 1 + (2 + 1 + 2) * sizeof(floatToChars_t);
				break;

			case E_GraphOp_ARC_CCW:
				sz += 1 + (2 + 1 + 2) * sizeof(floatToChars_t);
				break;

			case E_GraphOp_CLOSE_PATH:
				sz += 1;
				break;
		};
	}

	if (el->type & GR_EL_MASK) {
		sz++;
	}

	char* buf = realloc(el->data, sz);

	el->data = buf;
	el->data_len = sz;

	floatToChars_t tmp;

	j = 0;

	for (k = 0; k < ops_cnt; k++) {
		E_GraphOp* op = ops[k];
		switch (op->pick) {
			case E_GraphOp_MOVE_TO:
				buf[j++] = SHP_OP_MOVE_TO;

				tmp.f = (float)op->u.MOVE_TO.coord.x;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.MOVE_TO.coord.y;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}
				break;

			case E_GraphOp_LINE_TO:
				buf[j++] = SHP_OP_LINE_TO;

				tmp.f = (float)op->u.LINE_TO.coord.x;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.LINE_TO.coord.y;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}
				break;

			case E_GraphOp_CURVE_TO:
				buf[j++] = SHP_OP_CURVE_TO;

				tmp.f = (float)op->u.CURVE_TO.coord.t0.x;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.CURVE_TO.coord.t0.y;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.CURVE_TO.coord.t1.x;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.CURVE_TO.coord.t1.y;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.CURVE_TO.coord.t2.x;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.CURVE_TO.coord.t2.y;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}
				break;

			case E_GraphOp_ARC_CW:
				buf[j++] = SHP_OP_ARC_CW;

				tmp.f = (float)op->u.ARC_CW.coord.center.x;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.ARC_CW.coord.center.y;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.ARC_CW.coord.radius;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.ARC_CW.coord.ang_from;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.ARC_CW.coord.ang_to;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}
				break;

			case E_GraphOp_ARC_CCW:
				buf[j++] = SHP_OP_ARC_CCW;

				tmp.f = (float)op->u.ARC_CW.coord.center.x;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.ARC_CW.coord.center.y;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.ARC_CW.coord.radius;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.ARC_CW.coord.ang_from;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}

				tmp.f = (float)op->u.ARC_CW.coord.ang_to;
				for (i = 0; i < sizeof(floatToChars_t); i++) {
					buf[j++] = tmp.c[i];
				}
				break;

			case E_GraphOp_CLOSE_PATH:
				buf[j++] = SHP_OP_CLOSE_PATH;
				break;
		};
	}

	if (el->type & GR_EL_MASK) {
		buf[j++] = SHP_OP_toMASK;
	}

	reset_rendered_flag(&el);
}



void Shape_undoGraph(void* par)
{
	int i, j, k;

	F_Shape_undoGraph *f = par;

	E_Shape* shp = f->this;
	
	if (f->this == H || f->this == nil)
		return;

	graphic_el **el_nest = find_graph_el_by_id(shp->id, NULL);
	if (!el_nest) {
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		return;
	}
	if ((el->type & 0xff) != GR_EL_SHAPE) {
		return;
	}

	fill_shp_graph_ops_from_graphic_el(shp, el);

//	Array* ar_ops = shp->graph_ops;
//	if (ar_ops->len <= 0) {
//		return;
//	}
}


void Shape_LineWidth(void* par)
{
	F_Shape_LineWidth *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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
	
	if (f->this == H || f->this == nil)
		return;

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
	
	if (f->this == H || f->this == nil)
		return;

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

	if (f->this == H || f->this == nil)
		return;

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

		reset_rendered_flag(&el); //->root);
	}
	*f->ret = 0;
}



void Shape_setScale(void* par)
{
	F_Shape_setScale *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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

		reset_rendered_flag(&el);
	}
	*f->ret = 0;
}



void Shape_setOffset(void* par)
{
	F_Shape_setOffset *f = par;
	
	if (f->this == H || f->this == nil)
		return;

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

	if (f->this == H || f->this == nil)
		return;

	if (f->shape == H || f->shape == nil)
		return;

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

//D();
		graphic_el* old_shape = set_graph_el_to_pos(parent_el_nest, el, n, 0);
		if (old_shape) {
			f->ret->t0 = old_shape->id; //el2->id;
		} else {
			f->ret->t0 = 0;
		}
	}

	E_Shape* shp = f->shape;
	E_Shape* parent = f->this;

	f->ret->t1 = 0;
}


void Shape_insDrawZ(void* par)
{
	F_Shape_insDrawZ *f = par;
	
	if (f->this == H || f->this == nil)
		return;

	if (f->shape == H || f->shape == nil)
		return;

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

//D();
	if (el && (parent_id == 0 
				|| (*parent_el_nest && (*parent_el_nest)->type & GR_EL_MASK)
				|| (*parent_el_nest && (*parent_el_nest)->type & 0xff == GR_EL_CANVAS)
	)) {
		*el_nest = el->next;

//D();
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



void Shape_getMaskedCanvasLen(void* par)
{
	int i, j, k;

	F_Shape_getMaskedCanvasLen *f = par;

	E_Shape* shp = f->this;

	*f->ret = 0;
	
	if (f->this == H || f->this == nil)
		return;

	graphic_el **el_nest = find_graph_el_by_id(shp->id, NULL);
	if (!el_nest) {
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		return;
//	}

	if (el->masked) {
		graphic_el* itm = el->masked;

		for (i = 0; itm != NULL; i++, itm = itm->next);

		*f->ret = i;
	}
}



void Shape_getSubCanvasLen(void* par)
{
	int i, j, k;

	F_Shape_getSubCanvasLen *f = par;

	E_Shape* shp = f->this;

	*f->ret = 0;
	
	if (f->this == H || f->this == nil)
		return;

	graphic_el **el_nest = find_graph_el_by_id(shp->id, NULL);
	if (!el_nest) {
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		return;
//	}

	if (el->canvas) {
		graphic_el* itm = el->canvas;

		for (i = 0; itm != NULL; i++, itm = itm->next);

		*f->ret = i;
	}
}


void Shape_getMaskedCanvasItm(void* par)
{
	int j, cnt;

	F_Shape_getMaskedCanvasItm *f = par;

	E_Shape* shp = f->this;

	if (f->this == H || f->this == nil)
		return;

	graphic_el **el_nest = find_graph_el_by_id(shp->id, NULL);
	if (!el_nest) {
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		return;
//	}

	if (!el->masked || f->i < 0)
		return;

	graphic_el* itm = el->masked;

	for (cnt = 0; itm != NULL; cnt++, itm = itm->next);

	if (f->i >= cnt)
		return;

	for (
		j = 0, itm = el->masked; 
		j < f->i && itm != NULL; 
		j++, itm = itm->next);

	*f->ret = shape_by_graphic_el(itm);
}



void Shape_getSubCanvasItm(void* par)
{
	int j, cnt;

	F_Shape_getSubCanvasItm *f = par;

	E_Shape* shp = f->this;

	if (f->this == H || f->this == nil)
		return;

	graphic_el **el_nest = find_graph_el_by_id(shp->id, NULL);
	if (!el_nest) {
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		return;
//	}

	if (!el->canvas || f->i < 0)
		return;

	graphic_el* itm = el->canvas;

	for (cnt = 0; itm != NULL; cnt++, itm = itm->next);

	if (f->i >= cnt)
		return;

	for (
		j = 0, itm = el->canvas; 
		j < f->i && itm != NULL; 
		j++, itm = itm->next);

	*f->ret = shape_by_graphic_el(itm);
}



void Shape_getParent(void* par)
{
	F_Shape_getParent *f = par;

	E_Shape* shp = f->this;

	if (f->this == H || f->this == nil)
		return;

	graphic_el **el_nest = find_graph_el_by_id(shp->id, NULL);
	if (!el_nest) {
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		return;
//	}

	if (el->root == NULL || *(el->root) == NULL || (*el->root)->id == 0) {
		crRefShapeObj(0, f->ret);
	} else {
		*f->ret = shape_by_graphic_el(*el->root);
	}
}


void Shape_setExecCbPath(void* par)
{
	F_Shape_setExecCbPath *f = par;

	E_Shape* shp = f->this;

	f->ret->t1 = -1;

	if (f->this == H || f->this == nil)
		return;

	graphic_el **el_nest = find_graph_el_by_id(shp->id, NULL);
	if (!el_nest) {
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		return;
//	}

	char* exec_mod_old = el->exec_mod;
	char* exec_mod_new;

	destroy(f->ret->t0);
	f->ret->t0 = c2string(exec_mod_old, strlen(exec_mod_old));

	String* new_path = f->new_path;

	if (new_path == H || new_path == nil) {
		f->ret->t1 = 0;
		return;
	}

	free(exec_mod_old);

	exec_mod_new = string2c(new_path);

	int fd = kopen(exec_mod_new, OREAD);
	if (fd < 0) {
		el->exec_mod = strdup("");
		return;
	}
	kclose(fd);

	el->exec_mod = strdup(exec_mod_new);

//	load_shape_exec_mod(el);

	f->ret->t1 = 0;
}



#if 1
void Shape_setExecCbChan(void* par)
{
	F_Shape_setExecCbChan *f = par;

	E_Shape* shp = f->this;

	if (f->this == H || f->this == nil)
		return;

	if (f->c == H)
		return;

	graphic_el **el_nest = find_graph_el_by_id(shp->id, NULL);
	if (!el_nest) {
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		return;
//	}

	destroy(el->exec_mod_chan);

	Channel* c = f->c;

	heap_ref_inc(D2H(c));

	el->exec_mod_chan = c;


	Proc* p = getup(); //{}!!
	groot = p->groot;  //{}!!
	
}
#endif


void Shape_getExecCbChan(void* par)
{
	F_Shape_getExecCbChan *f = par;

	E_Shape* shp = f->this;

	if (f->this == H || f->this == nil)
		return;

	graphic_el **el_nest = find_graph_el_by_id(shp->id, NULL);
	if (!el_nest) {
		return;
	}
	graphic_el *el = *el_nest;
	if (!el) {
		return;
	}
//	if ((el->type & 0xff) != GR_EL_SHAPE) {
//		return;
//	}


//	Channel* c = cnewc(&Tptr, movp, 8);
//	destroy(el->exec_mod_chan);
//	el->exec_mod_chan = c;

	Channel* c = el->exec_mod_chan;

	destroy(*f->ret);
	*f->ret = c;


	Proc* p = getup(); //{}!!
	groot = p->groot;  //{}!!
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
//D();
	if (main_screen_flags & GR_EL_WAIT_TO_RENDER) {
//D();
		render_gr_stack_to_main_screen();

		main_screen_flags &= ~GR_EL_WAIT_TO_RENDER;
		main_screen_flags |=  GR_EL_RENDERED;
	}

//D();
	flushscreen();
//D();
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


