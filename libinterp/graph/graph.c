

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



void heap_ref_inc(Heap* ho)
{
	ho->ref++;
	Setmark(ho);
}


static Array* array_resize(Array* ar, uint size)
{
	if (ar == NULL)
		return H;

	int osize = hmsize(D2H(ar));
	int sz = sizeof(Heap) + sizeof(Array) + (ar->t->size * (size));

	if (osize >= sz) {
		ar->len = size;
//		el_list = (void**)ar->data;
		return ar;

	} else {
		Heap* ho = heaparray(ar->t, size);
		heap_ref_inc(ho);

		Array* ar_new = H2D(Array*, ho);
//DD("!!:.: ar_new: %p, len: %d (%d)", ar_new, ar_new->len, sizeof(void*));

		char* el_old_list = ar->data;
		char* el_list	  = ar_new->data;

		memcpy(el_list, el_old_list, ar->t->size * ar->len);

		destroy(ar);

		return ar_new;
	}
}



static void proc_user_cleaner_iterator(graphic_el *el);

static graphic_el* del_graphic_el(graphic_el *el)
{
	if (!el)
		return NULL;

	graphic_el *el_nxt = el->next;

//	if (el->shp) {
//		poolmutable(D2H(el->shp));
//		el->shp = NULL;
//	}

	if (el->exec_mod) {
		free(el->exec_mod);
	}
	destroy(el->exec_mod_chan);

	if (el->sfc) {
		cairo_surface_destroy(el->sfc);
	}

	if (el->data) {
		free(el->data);
	}

	void* tl = el->text_lines;
	while (tl) {
		tl = text_del_line(tl);
	}
	if (el->text_style_name)
		free(el->text_style_name);

	destroy(el->exec_mod_chan);

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

graphic_el** find_graph_el_by_id(int id, int *is_not_free)
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


graphic_el* set_graph_el_to_pos(graphic_el** parent_el_nest, graphic_el *el, int n, int is_ins)
{
//D();
	if (el == NULL || n < 0)
		return NULL;


DD("parent_el_nest: %p", parent_el_nest);
if (parent_el_nest)
	DD("*parent_el_nest: %p", *parent_el_nest);


	graphic_el **el_nest;
	graphic_el **parent;

	if (!parent_el_nest || !*parent_el_nest) {
		Proc *p = getup();
//D();
		el_nest = &p->groot;
		parent  = NULL;
	} else if (((*parent_el_nest)->type & 0xff) == GR_EL_CANVAS) {
//D();
		el_nest = &((*parent_el_nest)->canvas);
		parent  = parent_el_nest;
	} else {
//D();
		el_nest = &((*parent_el_nest)->masked);
		parent  = parent_el_nest;
	}

	graphic_el *el2 = *el_nest;
	
//D();
	// go to level
	for (; n >= 0; n--) {
//D();
		if (el2 == NULL) {
//D();
			el2 = malloc(sizeof(graphic_el));
			memset(el2, 0, sizeof(graphic_el));

			int id = ((int)((ulong)el2 >> 2) ^ (int)((ulong)el2 >> 32)) & 0xfffffff;
			
			el2->id   = id;
			el2->type = GR_EL_SHAPE;

			*el_nest = el2;
//D();
		}

		if (n == 0)
			break;
//D();
		el_nest = &el2->next;
		el2 = *el_nest;
//D();
	}
	
//D();
	if (el->id != el2->id) {
//D();
		el->root = parent;

		if (!is_ins) {
//D();
			el->next = el2->next;
//D();
			*el_nest = el;
		
//D();
			if (el2->id <= 0) {
//D();
				free(el2);
				return NULL;
			} else {
//D();
				Proc *p = getup();
				el2->next = p->free_levels;
				p->free_levels = el2;

				return el2;
			}
		} else {
//D();
			*el_nest = el;
			el->next = el2;
//D();
		}

		reset_rendered_flag(&el);
	}
//D();
	return el;
}



void crRefShapeObj(int id, E_Shape** p)
{
	E_Shape* shp;

	Heap* ho;
	Array *ar;

	if (!p)
		return;

//	 ref = H2D(E_Shape**, heap(Tptr));
	shp = H2D(E_Shape*,  heap(TShape));

//	poolimmutable(D2H(shp));

	shp->id = id;

	//ho = heaparray(&Tptr, 0);
	//heap_ref_inc(ho);
	//ar = H2D(Array*, ho);
	shp->sub_canvas = H; //ar;

	//ho = heaparray(&Tptr, 0);
	//heap_ref_inc(ho);
	//ar = H2D(Array*, ho);
	shp->mask_canvas = H; //ar;

	ho = heaparray(&Tptr, 0);
	heap_ref_inc(ho);
	ar = H2D(Array*, ho);
	shp->graph_ops = ar;

	shp->parent = H;
	
	destroy(*p);
	*p = shp;

//	return shp;
}


graphic_el* new_graphic_el(int type, int isAddToFreeList)
{
	graphic_el *el = malloc(sizeof(graphic_el));
	memset(el, 0, sizeof(graphic_el));

	int id = ((int)((ulong)el >> 2) ^ (int)((ulong)el >> 32)) & 0xfffffff;

	el->id = id;
	el->type = type;

	el->exec_mod      = strdup("");
//	el->m             = H;
	el->exec_mod_chan = H;

	el->angle = 0.;
	el->x = 0.;
	el->y = 0.;
	el->sx = 1.;
	el->sy = 1.;

	el->line_b = 1.;

	el->line_a = 1.;
	el->fill_a = 1.;

	el->line_width = 1.;

	el->masked = NULL;
	el->canvas = NULL;

//	el->flags &= ~GR_EL_WAIT_TO_RENDER;
//	el->flags |=  GR_EL_RENDERED;	
	el->flags = GR_EL_WAIT_TO_RENDER;

	el->shp = NULL;

	el->text_lines = NULL;

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


static graphic_el* do_new_svg_el(char* svg_str, int isAddToFreeList)
{
	graphic_el* el = new_graphic_el(GR_EL_SVG, isAddToFreeList);

	el->data = strdup(svg_str);

/*
	el->sfc = _draw_svg(svg_str, &el->w, &el->h);

	el->x_min = 0.;
	el->x_max = el->w;
	el->y_min = 0.;
	el->y_max = el->h;

	el->flags &= ~GR_EL_WAIT_TO_RENDER;
	el->flags |=  GR_EL_RENDERED;
*/

	return el;
}


E_Shape* do_new_svg(char* svg_str, E_Shape* o_old)
{
	E_Shape* o_out = o_old;

	graphic_el* el = do_new_svg_el(svg_str, 1);	

	crRefShapeObj(el->id, &o_out);

	el->shp = o_out;

	return o_out;
}


E_Shape* do_new_shape(E_Shape* o_old)
{
	E_Shape* o_out = o_old;

	graphic_el* el = new_graphic_el(GR_EL_SHAPE, 1);

	crRefShapeObj(el->id, &o_out);

	el->shp = o_out;

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


graphic_el* chunk_new_shape_el(uchar typ, uchar* p, uint len, uint* cnt_used);


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


graphic_el* chunk_new_shape_el(uchar typ, uchar* p, uint len, uint* cnt_used)
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




uchar* encToChunk_el(graphic_el *el, uchar* buf, uint len, uint* buf_size, uchar *type);

uchar* encToChunk_el_list(uchar prefix, graphic_el *el, uchar* buf, uint len, uint* buf_size)
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


uchar* encToChunk_el(graphic_el *el, uchar* buf, uint len, uint* buf_size, uchar *type)
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


extern Pool* heapmem;

void add_op_to_shape(E_Shape *shp, int op_type,
							double p0, double p1, double p2, double p3, double p4, double p5)
{
	Heap *ho, *hs;

	if (!(
		   op_type == E_GraphOp_MOVE_TO
		|| op_type == E_GraphOp_LINE_TO
		|| op_type == E_GraphOp_CURVE_TO
		|| op_type == E_GraphOp_ARC_CW
		|| op_type == E_GraphOp_ARC_CCW
		|| op_type == E_GraphOp_CLOSE_PATH
//		|| op_type == E_GraphOp_TEXT
	))
		return;

	Array* ar_ops = shp->graph_ops;
	
//	{if (ar_ops->len > 4) return;}

#if 1
	void** el_list;

#if 1
	ar_ops = array_resize(ar_ops, ar_ops->len + 1);
	shp->graph_ops = ar_ops;

	el_list = ar_ops->data;
#else
	int osize = hmsize(D2H(ar_ops));
	int sz = sizeof(Heap) + sizeof(Array) + (Tptr.size * (ar_ops->len + 1));

	if (osize >= sz) {
		ar_ops->len++;
		el_list = (void**)ar_ops->data;
	} else {
		ho = heaparray(&Tptr, ar_ops->len + 1);
		ho->ref++;
		Setmark(ho);

		Array* ar_new = H2D(Array*, ho);
DD("!!:.: ar_new: %p, len: %d (%d)", ar_new, ar_new->len, sizeof(void*));

		void** el_old_list = (void**)ar_ops->data;
		       el_list     = (void**)ar_new->data;

		for (int i = 0; i < ar_ops->len; i++) {
			el_list[i] = el_old_list[i];
		}

		shp->graph_ops = ar_new;
//	el_list = (void**)ar_new->data;

		destroy(ar_ops);

		ar_ops = ar_new;
	}
#endif

#else
	Heap* h = D2H(ar_ops);

	ar_ops->len++;

	ulong sz = 
//		(hmsize(h) + Tptr.size + 7) & ~7L;
		sizeof(Heap) + sizeof(Array) + (Tptr.size * (ar_ops->len));

DD("Heap pre: %p (%p)", h, D2H(shp));
	h = (Heap*)poolrealloc(heapmem, h, sz);
DD("Heap after: %p (%p)", h, D2H(shp));

//#undef DD(...)
//#define DD(...) 

	if(heapmonitor != nil)
		heapmonitor(0, h, sz);

	ar_ops = H2D(Array*, h);
	shp->graph_ops = ar_ops;

	void** el_list = (void**)ar_ops->data;

	void* p = (void*)el_list[ar_ops->len - 1];
//	for (int i = 0; i < ar_ops->t->size; i++, p++) {
//		initmem(ar_ops->t, p);
//	}
#endif

	String* s = NULL;

	E_GraphOp* gop = (E_GraphOp*)0xDEADF15;
	switch (op_type) {
		case E_GraphOp_MOVE_TO:
			ho = heap(TGraphOp_MOVE_TO);
			gop = H2D(E_GraphOp*, ho);
			gop->pick = E_GraphOp_MOVE_TO;

			s = s_moveTo; //c2string("moveTo", strlen("moveTo"));

			gop->u.MOVE_TO.coord.x = p0;
			gop->u.MOVE_TO.coord.y = p1;
			break;

		case E_GraphOp_LINE_TO:
			ho = heap(TGraphOp_LINE_TO);
			gop = H2D(E_GraphOp*, ho);
			//gop = H2D(E_GraphOp*, heap(TGraphOp_LINE_TO));
			gop->pick = E_GraphOp_LINE_TO;

			s = s_lineTo; //c2string("lineTo", strlen("lineTo"));

			gop->u.LINE_TO.coord.x = p0;
			gop->u.LINE_TO.coord.y = p1;
			break;

		case E_GraphOp_CURVE_TO:
			ho = heap(TGraphOp_CURVE_TO);
			gop = H2D(E_GraphOp*, ho);
//			gop = H2D(E_GraphOp*, heap(TGraphOp_CURVE_TO));
			gop->pick = E_GraphOp_CURVE_TO;

			s = s_curveTo; //c2string("curveTo", strlen("curveTo"));

			gop->u.CURVE_TO.coord.t0.x = p0;
			gop->u.CURVE_TO.coord.t0.y = p1;

			gop->u.CURVE_TO.coord.t1.x = p2;
			gop->u.CURVE_TO.coord.t1.y = p3;

			gop->u.CURVE_TO.coord.t2.x = p4;
			gop->u.CURVE_TO.coord.t2.y = p5;
			break;

		case E_GraphOp_ARC_CW:
			ho = heap(TGraphOp_ARC_CW);
			gop = H2D(E_GraphOp*, ho);
//			gop = H2D(E_GraphOp*, heap(TGraphOp_ARC_CW));
			gop->pick = E_GraphOp_ARC_CW;

			s = s_arcCW; //c2string("arcCW", strlen("arcCW"));

			gop->u.ARC_CW.coord.center.x = p0;
			gop->u.ARC_CW.coord.center.y = p1;

			gop->u.ARC_CW.coord.radius   = p2;

			gop->u.ARC_CW.coord.ang_from = p3;
			gop->u.ARC_CW.coord.ang_to   = p4;
			break;

		case E_GraphOp_ARC_CCW:
			ho = heap(TGraphOp_ARC_CCW);
			gop = H2D(E_GraphOp*, ho);
//			gop = H2D(E_GraphOp*, heap(TGraphOp_ARC_CCW));
			gop->pick = E_GraphOp_ARC_CCW;

			s = s_arcCCW; //c2string("arcCCW", strlen("arcCCW"));

			gop->u.ARC_CW.coord.center.x = p0;
			gop->u.ARC_CW.coord.center.y = p1;

			gop->u.ARC_CW.coord.radius   = p2;

			gop->u.ARC_CW.coord.ang_from = p3;
			gop->u.ARC_CW.coord.ang_to   = p4;
			break;

		case E_GraphOp_CLOSE_PATH:
			ho = heap(TGraphOp_CLOSE_PATH);
			gop = H2D(E_GraphOp*, ho);
//			gop = H2D(E_GraphOp*, heap(TGraphOp_CLOSE_PATH));
			gop->pick = E_GraphOp_CLOSE_PATH;

			s = s_closePath; //c2string("closePath", strlen("closePath"));
			break;
/*
		case E_GraphOp_TEXT:
			ho = heap(TGraphOp_TEXT);
			gop = H2D(E_GraphOp*, ho);
//			gop = H2D(E_GraphOp*, heap(TGraphOp_CLOSE_PATH));
			gop->pick = E_GraphOp_TEXT;

			gop->u.TEXT.text = c2string((char*)p0, (int)p1);

			s = s_text; //c2string("closePath", strlen("closePath"));
			break;
*/
	};

	ho->ref++;
	Setmark(ho);

//	hs = D2H(s);
//	hs->ref++;
//	Setmark(hs);

	gop->op = s;

//	lvl = H2D(E_Level**, heap(&Tptr));
//	lvl = H2D(E_Level*, heap(Tlevel));

//	lvl->graphics.fill = H;

	el_list[ar_ops->len - 1] = (void*)gop;

//	ar_ops->len++;

//	D();
//	destroy(*f->ret);
//	*f->ret = lvl;


}



void add_op_text_to_shape(E_Shape *shp, int op_type, char* text, int text_len)
{
	Heap *ho, *hs;

	if (!(
		op_type == E_GraphOp_TEXT
	))
		return;

	Array* ar_ops = shp->graph_ops;

#if 1
	void** el_list;

#if 1
	ar_ops = array_resize(ar_ops, ar_ops->len + 1);
	shp->graph_ops = ar_ops;

	el_list = ar_ops->data;
#else
								int osize = hmsize(D2H(ar_ops));
								int sz = sizeof(Heap) + sizeof(Array) + (Tptr.size * (ar_ops->len + 1));
							
								if (osize >= sz) {
									ar_ops->len++;
									el_list = (void**)ar_ops->data;
								} else {
									ho = heaparray(&Tptr, ar_ops->len + 1);
									ho->ref++;
									Setmark(ho);
							
									Array* ar_new = H2D(Array*, ho);
							DD("!!:.: ar_new: %p, len: %d (%d)", ar_new, ar_new->len, sizeof(void*));
							
									void** el_old_list = (void**)ar_ops->data;
										   el_list	   = (void**)ar_new->data;
							
									for (int i = 0; i < ar_ops->len; i++) {
										el_list[i] = el_old_list[i];
									}
							
									shp->graph_ops = ar_new;
							//	el_list = (void**)ar_new->data;
							
									destroy(ar_ops);
							
									ar_ops = ar_new;
								}
#endif

#else
								Heap* h = D2H(ar_ops);
							
								ar_ops->len++;
							
								ulong sz = 
							//		(hmsize(h) + Tptr.size + 7) & ~7L;
									sizeof(Heap) + sizeof(Array) + (Tptr.size * (ar_ops->len));
							
							DD("Heap pre: %p (%p)", h, D2H(shp));
								h = (Heap*)poolrealloc(heapmem, h, sz);
							DD("Heap after: %p (%p)", h, D2H(shp));
							
							//#undef DD(...)
							//#define DD(...) 
							
								if(heapmonitor != nil)
									heapmonitor(0, h, sz);
							
								ar_ops = H2D(Array*, h);
								shp->graph_ops = ar_ops;
							
								void** el_list = (void**)ar_ops->data;
							
								void* p = (void*)el_list[ar_ops->len - 1];
							//	for (int i = 0; i < ar_ops->t->size; i++, p++) {
							//		initmem(ar_ops->t, p);
							//	}
#endif

	String* s = NULL;

	E_GraphOp* gop = (E_GraphOp*)0xDEADF15;
	switch (op_type) {
		case E_GraphOp_TEXT:
			ho = heap(TGraphOp_TEXT);
			gop = H2D(E_GraphOp*, ho);
//			gop = H2D(E_GraphOp*, heap(TGraphOp_CLOSE_PATH));
			gop->pick = E_GraphOp_TEXT;

			gop->u.TEXT.text = c2string(text, text_len);

			s = s_text; //c2string("closePath", strlen("closePath"));
			break;
	};

	ho->ref++;
	Setmark(ho);

	//	hs = D2H(s);
	//	hs->ref++;
	//	Setmark(hs);

	gop->op = s;
							
	el_list[ar_ops->len - 1] = (void*)gop;
}




void fill_shp_graph_ops_from_graphic_el(E_Shape* shp, graphic_el *el)
{
	if (!shp || !el)
		return;

	int i, j;

	uint cnt = 0;
	uint do_cnt = 1;

	for (i = 0; do_cnt && i < el->data_len; i++) {
		uchar op = el->data[i];
		switch (op) {
			case SHP_OP_MOVE_TO:
				i += 2*sizeof(float);
				cnt++;
				break;

			case SHP_OP_LINE_TO:
				i += 2*sizeof(float);
				cnt++;
				break;

			case SHP_OP_CURVE_TO:
				i += 3*2*sizeof(float);
				cnt++;
				break;

			case SHP_OP_ARC_CW:
				i += (2 + 1 + 2)*sizeof(float);
				cnt++;
				break;

			case SHP_OP_ARC_CCW:
				i += (2 + 1 + 2)*sizeof(float);
				cnt++;
				break;

			case SHP_OP_toMASK:
				break;

			case SHP_OP_unMASK:
				break;

			case SHP_OP_CLOSE_PATH:
				cnt++;
				break;

			default:
				do_cnt = 0;
				break;
		}
	}


	E_GraphOp** shp_lst = (E_GraphOp**)shp->graph_ops->data;
	for (i = 0; i < shp->graph_ops->len; i++) {
		destroy(shp_lst[i]);
	}

	shp->graph_ops = array_resize(shp->graph_ops, cnt);
	shp_lst = (E_GraphOp**)shp->graph_ops->data;


	j = 0;

	for (i = 0; do_cnt && i < el->data_len;) {
		uchar op = el->data[i];
		i++;

		String* s = NULL;
	
		floatToChars_t* tmp;
	
		Heap*			ho = NULL;
		E_GraphOp*      gop = NULL;
		switch (op) {
			case SHP_OP_MOVE_TO:
				//i += 2*sizeof(float);
				ho = heap(TGraphOp_MOVE_TO);
				gop = H2D(E_GraphOp*, ho);
				gop->pick = E_GraphOp_MOVE_TO;
	
				s = s_moveTo; //c2string("moveTo", strlen("moveTo"));

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.MOVE_TO.coord.x = tmp->f;
				i += sizeof(floatToChars_t);

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.MOVE_TO.coord.y = tmp->f;
				i += sizeof(floatToChars_t);
				break;
	
			case SHP_OP_LINE_TO:
				//i += 2*sizeof(float);
				ho = heap(TGraphOp_LINE_TO);
				gop = H2D(E_GraphOp*, ho);
				//gop = H2D(E_GraphOp*, heap(TGraphOp_LINE_TO));
				gop->pick = E_GraphOp_LINE_TO;
	
				s = s_lineTo; //c2string("lineTo", strlen("lineTo"));
	
				tmp = (floatToChars_t*)&el->data[i];
				gop->u.LINE_TO.coord.x = tmp->f;
				i += sizeof(floatToChars_t);

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.LINE_TO.coord.y = tmp->f;
				i += sizeof(floatToChars_t);
				break;
	
			case SHP_OP_CURVE_TO:
				ho = heap(TGraphOp_CURVE_TO);
				gop = H2D(E_GraphOp*, ho);
	//			gop = H2D(E_GraphOp*, heap(TGraphOp_CURVE_TO));
				gop->pick = E_GraphOp_CURVE_TO;
	
				s = s_curveTo; //c2string("curveTo", strlen("curveTo"));
	
				tmp = (floatToChars_t*)&el->data[i];
				gop->u.CURVE_TO.coord.t0.x = tmp->f;
				i += sizeof(floatToChars_t);

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.CURVE_TO.coord.t0.y = tmp->f;
				i += sizeof(floatToChars_t);
	

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.CURVE_TO.coord.t1.x = tmp->f;
				i += sizeof(floatToChars_t);

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.CURVE_TO.coord.t1.y = tmp->f;
				i += sizeof(floatToChars_t);
	

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.CURVE_TO.coord.t2.x = tmp->f;
				i += sizeof(floatToChars_t);

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.CURVE_TO.coord.t2.y = tmp->f;
				i += sizeof(floatToChars_t);
				break;
	
			case SHP_OP_ARC_CW:
				ho = heap(TGraphOp_ARC_CW);
				gop = H2D(E_GraphOp*, ho);
	//			gop = H2D(E_GraphOp*, heap(TGraphOp_ARC_CW));
				gop->pick = E_GraphOp_ARC_CW;
	
				s = s_arcCW; //c2string("arcCW", strlen("arcCW"));
	
				tmp = (floatToChars_t*)&el->data[i];
				gop->u.ARC_CW.coord.center.x = tmp->f;
				i += sizeof(floatToChars_t);

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.ARC_CW.coord.center.y = tmp->f;
				i += sizeof(floatToChars_t);

	
				tmp = (floatToChars_t*)&el->data[i];
				gop->u.ARC_CW.coord.radius	 = tmp->f;
				i += sizeof(floatToChars_t);

	
				tmp = (floatToChars_t*)&el->data[i];
				gop->u.ARC_CW.coord.ang_from = tmp->f;
				i += sizeof(floatToChars_t);

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.ARC_CW.coord.ang_to	 = tmp->f;
				i += sizeof(floatToChars_t);
				break;
	
			case SHP_OP_ARC_CCW:
				ho = heap(TGraphOp_ARC_CCW);
				gop = H2D(E_GraphOp*, ho);
	//			gop = H2D(E_GraphOp*, heap(TGraphOp_ARC_CCW));
				gop->pick = E_GraphOp_ARC_CCW;
	
				s = s_arcCCW; //c2string("arcCCW", strlen("arcCCW"));
	
				tmp = (floatToChars_t*)&el->data[i];
				gop->u.ARC_CCW.coord.center.x = tmp->f;
				i += sizeof(floatToChars_t);

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.ARC_CCW.coord.center.y = tmp->f;
				i += sizeof(floatToChars_t);

	
				tmp = (floatToChars_t*)&el->data[i];
				gop->u.ARC_CCW.coord.radius	 = tmp->f;
				i += sizeof(floatToChars_t);

	
				tmp = (floatToChars_t*)&el->data[i];
				gop->u.ARC_CCW.coord.ang_from = tmp->f;
				i += sizeof(floatToChars_t);

				tmp = (floatToChars_t*)&el->data[i];
				gop->u.ARC_CCW.coord.ang_to	 = tmp->f;
				i += sizeof(floatToChars_t);
				break;

			case SHP_OP_toMASK:
				break;

			case SHP_OP_unMASK:
				break;

			case SHP_OP_CLOSE_PATH:
				ho = heap(TGraphOp_CLOSE_PATH);
				gop = H2D(E_GraphOp*, ho);
	//			gop = H2D(E_GraphOp*, heap(TGraphOp_CLOSE_PATH));
				gop->pick = E_GraphOp_CLOSE_PATH;
	
				s = s_closePath; //c2string("closePath", strlen("closePath"));
				break;
		};

		if (ho && gop) {
			ho->ref++;
			Setmark(ho);

			gop->op = s;

			shp_lst[j] = gop;
			j++;
		}
	}
}


E_Shape* shape_by_graphic_el(graphic_el *el)
{
	E_Shape* shp = H;

	crRefShapeObj(el->id, &shp);

	switch (el->type & 0xff) {
		case GR_EL_SHAPE:
			if (el->data && el->data_len > 0) {
				fill_shp_graph_ops_from_graphic_el(shp, el);
			}
			break;
	}

	return shp;
}


static graphic_el* findGraphElsList(graphic_el* el, double x, double y)
{
	graphic_el* el_found = NULL;

	for (; el != NULL && el != nil && el != H; el = el->next) {
		if (x < el->x || y < el->y)
			continue;

		if (x > (el->x + el->w) || y > (el->y + el->h))
			continue;

		if (el->exec_mod_chan != NULL && el->exec_mod_chan != H)
			el_found = el;

		if (el->canvas) {
			graphic_el* el2 = findGraphElsList(el->canvas, x - el->x, y - el->y);
			if (el2 != NULL)
				el_found = el2;
		}
	}
	return el_found;
}


graphic_el* findGraphElByXY(double x, double y)
{
//	Proc* p = getup();

//	if (p == NULL)
//		return NULL;

	graphic_el* el = groot;  //p->groot;

	el = findGraphElsList(el, x, y);

	return el;
}


