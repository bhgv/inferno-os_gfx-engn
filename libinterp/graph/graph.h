

#ifndef _GRAPH_GRAPH_H_
#define _GRAPH_GRAPH_H_


#if 0

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

#endif


//#define static 


#define D() print("%s> %s:%d\n", __FILE__, __func__, __LINE__)
#define DD(...) {print("%s> %s:%d ", __FILE__, __func__, __LINE__); print(__VA_ARGS__); print("\n");}


extern int	Xsize_;
extern int	Ysize_;


void graph_scrn_init();


extern graphic_el* groot;

//graphic_el* free_levels = NULL;


typedef union {
	float f;
	char  c[sizeof(float)];
} floatToChars_t;


extern Proc* getup();


E_Shape* shape_by_graphic_el(graphic_el *el);
void fill_shp_graph_ops_from_graphic_el(E_Shape* shp, graphic_el *el);


graphic_el** find_graph_el_by_id(int id, int *is_not_free);


cairo_surface_t* _draw_svg(char* svg_buf, double *w_out, double *h_out);

int sendStringToGraphEl(graphic_el* el, char *msg);
int printfToGraphEl(graphic_el *el, char *fmt, ...);

int csend_mouse_evt_(graphic_el* el, char* evt, double lx, double ly, double gx, double gy);
int csend_touch_evt_(graphic_el* el, E_XY_coord* arevt);


graphic_el* findGraphElByXY(double x, double y);



uchar* encToChunk_el(graphic_el *el, uchar* buf, uint len, uint* buf_size, uchar *type);
uchar* encToChunk_el_list(uchar prefix, graphic_el *el, uchar* buf, uint len, uint* buf_size);


graphic_el* chunk_new_shape_el(uchar typ, uchar* p, uint len, uint* cnt_used);


E_Shape* do_new_svg(char* svg_str, E_Shape* o_old);
E_Shape* do_new_shape(E_Shape* o_old);


void add_op_to_shape(E_Shape *shp, int op_type,
							double p0, double p1, double p2, double p3, double p4, double p5);


graphic_el* set_graph_el_to_pos(graphic_el** parent_el_nest, graphic_el *el, int n, int is_ins);


void render_gr_stack_to_main_screen();


void reset_rendered_flag(graphic_el **el);


cairo_surface_t* _draw_svg(char* svg_buf, double *w_out, double *h_out);
cairo_surface_t* _draw_png(char* png_path, double *w_out, double *h_out);



extern cairo_surface_t *sfc_main_screen;
extern uint             main_screen_flags;

extern ulong	*graph_scrn_data;



static Type*	Tlevel;


extern Type*	TGraphOp_MOVE_TO;
extern Type*	TGraphOp_LINE_TO;
extern Type*	TGraphOp_CURVE_TO;
extern Type*	TGraphOp_ARC_CW;
extern Type*	TGraphOp_ARC_CCW;
extern Type*	TGraphOp_CLOSE_PATH;

extern Type*	TShapeEvent_MOUSE;
extern Type*	TShapeEvent_TOUCH;

extern Type*	TShape;

extern Type*	TXY_coord;



//static	ulong	*data = NULL;

enum {
//	M_NONE = 0,
	M_MOVE = 0,

	M_WHEEL_FWD,
	M_WHEEL_BkWD,

	M_LB_DBLCLK,
	M_MB_DBLCLK,
	M_RB_DBLCLK,

	M_LB_DWN,
	M_MB_DWN,
	M_RB_DWN,
	
	M_LB_UP,
	M_MB_UP,
	M_RB_UP,

	M_MK_LB,
	M_MK_MB,
	M_MK_RB,
};

#define MAXPOINTS 10


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



extern String* s_moveTo;
extern String* s_lineTo;
extern String* s_curveTo;
extern String* s_arcCW;
extern String* s_arcCCW;
extern String* s_closePath;


graphic_el* new_graphic_el(int type, int isAddToFreeList);

void crRefShapeObj(int id, E_Shape** p);

void heap_ref_inc(Heap* ho);



void flushscreen();


#endif // _GRAPH_GRAPH_H_


