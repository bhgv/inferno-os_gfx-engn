typedef struct E_XY_coord E_XY_coord;
typedef struct E_Arc_coord E_Arc_coord;
typedef struct E_GraphOp E_GraphOp;
typedef struct E_ShapeEvent E_ShapeEvent;
typedef struct E_Level E_Level;
typedef struct E_Shape E_Shape;
struct E_XY_coord
{
	REAL	x;
	REAL	y;
};
#define E_XY_coord_size 16
#define E_XY_coord_map {0}
struct E_Arc_coord
{
	E_XY_coord	center;
	REAL	radius;
	REAL	ang_from;
	REAL	ang_to;
};
#define E_Arc_coord_size 40
#define E_Arc_coord_map {0}
#define E_GraphOp_MOVE_TO 0
#define E_GraphOp_LINE_TO 1
#define E_GraphOp_CURVE_TO 2
#define E_GraphOp_ARC_CW 3
#define E_GraphOp_ARC_CCW 4
#define E_GraphOp_CLOSE_PATH 5
#define E_GraphOp_TEXT 6
struct E_GraphOp
{
	int	pick;
	String*	op;
	union{
		struct{
			E_XY_coord	coord;
		} MOVE_TO;
		struct{
			E_XY_coord	coord;
		} LINE_TO;
		struct{
			struct{ E_XY_coord t0; E_XY_coord t1; E_XY_coord t2; }	coord;
		} CURVE_TO;
		struct{
			E_Arc_coord	coord;
		} ARC_CW;
		struct{
			E_Arc_coord	coord;
		} ARC_CCW;
		struct{
			WORD	coord;
		} CLOSE_PATH;
		struct{
			String*	text;
		} TEXT;
	} u;
};
#define E_GraphOp_MOVE_TO_size 24
#define E_GraphOp_MOVE_TO_map {0x40,}
#define E_GraphOp_LINE_TO_size 24
#define E_GraphOp_LINE_TO_map {0x40,}
#define E_GraphOp_CURVE_TO_size 56
#define E_GraphOp_CURVE_TO_map {0x40,}
#define E_GraphOp_ARC_CW_size 48
#define E_GraphOp_ARC_CW_map {0x40,}
#define E_GraphOp_ARC_CCW_size 48
#define E_GraphOp_ARC_CCW_map {0x40,}
#define E_GraphOp_CLOSE_PATH_size 12
#define E_GraphOp_CLOSE_PATH_map {0x40,}
#define E_GraphOp_TEXT_size 12
#define E_GraphOp_TEXT_map {0x60,}
#define E_ShapeEvent_MOUSE 0
#define E_ShapeEvent_TOUCH 1
struct E_ShapeEvent
{
	int	pick;
	E_Shape*	shape;
	union{
		struct{
			struct{ String* t0; uchar	_pad4[4]; E_XY_coord t1; E_XY_coord t2; }	data;
		} MOUSE;
		struct{
			Array*	data;
		} TOUCH;
	} u;
};
#define E_ShapeEvent_MOUSE_size 48
#define E_ShapeEvent_MOUSE_map {0x60,}
#define E_ShapeEvent_TOUCH_size 12
#define E_ShapeEvent_TOUCH_map {0x60,}
struct E_Level
{
	String*	note;
};
#define E_Level_size 4
#define E_Level_map {0x80,}
struct E_Shape
{
	WORD	id;
	Array*	graph_ops;
	String*	exec_cb_path;
	Array*	sub_canvas;
	Array*	mask_canvas;
	Array*	parent;
};
#define E_Shape_size 24
#define E_Shape_map {0x7c,}
void Shape_ArcCCW(void*);
typedef struct F_Shape_ArcCCW F_Shape_ArcCCW;
struct F_Shape_ArcCCW
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	x;
	REAL	y;
	REAL	radius;
	REAL	angle_from;
	REAL	angle_to;
};
void Shape_ArcCW(void*);
typedef struct F_Shape_ArcCW F_Shape_ArcCW;
struct F_Shape_ArcCW
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	x;
	REAL	y;
	REAL	radius;
	REAL	angle_from;
	REAL	angle_to;
};
void Shape_ClosePath(void*);
typedef struct F_Shape_ClosePath F_Shape_ClosePath;
struct F_Shape_ClosePath
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
};
void Shape_CurveTo(void*);
typedef struct F_Shape_CurveTo F_Shape_CurveTo;
struct F_Shape_CurveTo
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	x1;
	REAL	y1;
	REAL	x2;
	REAL	y2;
	REAL	x3;
	REAL	y3;
};
void Shape_FillColor(void*);
typedef struct F_Shape_FillColor F_Shape_FillColor;
struct F_Shape_FillColor
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	r;
	REAL	g;
	REAL	b;
	REAL	a;
};
void Shape_LineColor(void*);
typedef struct F_Shape_LineColor F_Shape_LineColor;
struct F_Shape_LineColor
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	r;
	REAL	g;
	REAL	b;
	REAL	a;
};
void Shape_LineTo(void*);
typedef struct F_Shape_LineTo F_Shape_LineTo;
struct F_Shape_LineTo
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	x;
	REAL	y;
};
void Shape_LineWidth(void*);
typedef struct F_Shape_LineWidth F_Shape_LineWidth;
struct F_Shape_LineWidth
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	w;
};
void Shape_MoveTo(void*);
typedef struct F_Shape_MoveTo F_Shape_MoveTo;
struct F_Shape_MoveTo
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	x;
	REAL	y;
};
void E_destroyObj(void*);
typedef struct F_E_destroyObj F_E_destroyObj;
struct F_E_destroyObj
{
	WORD	regs[NREG-1];
	WORD	noret;
	uchar	temps[12];
	WORD	id;
};
void E_doRender(void*);
typedef struct F_E_doRender F_E_doRender;
struct F_E_doRender
{
	WORD	regs[NREG-1];
	WORD	noret;
	uchar	temps[12];
};
void Shape_encToChunk(void*);
typedef struct F_Shape_encToChunk F_Shape_encToChunk;
struct F_Shape_encToChunk
{
	WORD	regs[NREG-1];
	Array**	ret;
	uchar	temps[12];
	E_Shape*	this;
};
void Shape_getDrawZ(void*);
typedef struct F_Shape_getDrawZ F_Shape_getDrawZ;
struct F_Shape_getDrawZ
{
	WORD	regs[NREG-1];
	struct{ E_Shape* t0; WORD t1; }*	ret;
	uchar	temps[12];
	E_Shape*	this;
	WORD	n;
};
void Shape_getExecCbChan(void*);
typedef struct F_Shape_getExecCbChan F_Shape_getExecCbChan;
struct F_Shape_getExecCbChan
{
	WORD	regs[NREG-1];
	Channel**	ret;
	uchar	temps[12];
	E_Shape*	this;
};
void E_getLevelsList(void*);
typedef struct F_E_getLevelsList F_E_getLevelsList;
struct F_E_getLevelsList
{
	WORD	regs[NREG-1];
	Array**	ret;
	uchar	temps[12];
};
void Shape_getMaskedCanvasItm(void*);
typedef struct F_Shape_getMaskedCanvasItm F_Shape_getMaskedCanvasItm;
struct F_Shape_getMaskedCanvasItm
{
	WORD	regs[NREG-1];
	E_Shape**	ret;
	uchar	temps[12];
	E_Shape*	this;
	WORD	i;
};
void Shape_getMaskedCanvasLen(void*);
typedef struct F_Shape_getMaskedCanvasLen F_Shape_getMaskedCanvasLen;
struct F_Shape_getMaskedCanvasLen
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
};
void Shape_getParent(void*);
typedef struct F_Shape_getParent F_Shape_getParent;
struct F_Shape_getParent
{
	WORD	regs[NREG-1];
	E_Shape**	ret;
	uchar	temps[12];
	E_Shape*	this;
};
void Shape_getSubCanvasItm(void*);
typedef struct F_Shape_getSubCanvasItm F_Shape_getSubCanvasItm;
struct F_Shape_getSubCanvasItm
{
	WORD	regs[NREG-1];
	E_Shape**	ret;
	uchar	temps[12];
	E_Shape*	this;
	WORD	i;
};
void Shape_getSubCanvasLen(void*);
typedef struct F_Shape_getSubCanvasLen F_Shape_getSubCanvasLen;
struct F_Shape_getSubCanvasLen
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
};
void E_increment(void*);
typedef struct F_E_increment F_E_increment;
struct F_E_increment
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	WORD	i;
};
void Shape_insDrawZ(void*);
typedef struct F_Shape_insDrawZ F_Shape_insDrawZ;
struct F_Shape_insDrawZ
{
	WORD	regs[NREG-1];
	struct{ WORD t0; WORD t1; }*	ret;
	uchar	temps[12];
	E_Shape*	this;
	E_Shape*	shape;
	WORD	n;
};
void E_moveTo(void*);
typedef struct F_E_moveTo F_E_moveTo;
struct F_E_moveTo
{
	WORD	regs[NREG-1];
	WORD	noret;
	uchar	temps[12];
	WORD	id;
	WORD	x;
	WORD	y;
};
void Level_new(void*);
typedef struct F_Level_new F_Level_new;
struct F_Level_new
{
	WORD	regs[NREG-1];
	E_Level**	ret;
	uchar	temps[12];
};
void E_newCanvas(void*);
typedef struct F_E_newCanvas F_E_newCanvas;
struct F_E_newCanvas
{
	WORD	regs[NREG-1];
	E_Shape**	ret;
	uchar	temps[12];
};
void E_newFromChunk(void*);
typedef struct F_E_newFromChunk F_E_newFromChunk;
struct F_E_newFromChunk
{
	WORD	regs[NREG-1];
	E_Shape**	ret;
	uchar	temps[12];
	Array*	chunk;
};
void E_newPNG(void*);
typedef struct F_E_newPNG F_E_newPNG;
struct F_E_newPNG
{
	WORD	regs[NREG-1];
	E_Shape**	ret;
	uchar	temps[12];
	String*	png_file_name;
};
void E_newSVG(void*);
typedef struct F_E_newSVG F_E_newSVG;
struct F_E_newSVG
{
	WORD	regs[NREG-1];
	E_Shape**	ret;
	uchar	temps[12];
	String*	svg;
};
void E_newShape(void*);
typedef struct F_E_newShape F_E_newShape;
struct F_E_newShape
{
	WORD	regs[NREG-1];
	E_Shape**	ret;
	uchar	temps[12];
};
void E_newText(void*);
typedef struct F_E_newText F_E_newText;
struct F_E_newText
{
	WORD	regs[NREG-1];
	E_Shape**	ret;
	uchar	temps[12];
	String*	txt;
};
void Shape_rmDrawZ(void*);
typedef struct F_Shape_rmDrawZ F_Shape_rmDrawZ;
struct F_Shape_rmDrawZ
{
	WORD	regs[NREG-1];
	struct{ E_Shape* t0; WORD t1; }*	ret;
	uchar	temps[12];
	E_Shape*	this;
	WORD	n;
};
void E_rootCanvas(void*);
typedef struct F_E_rootCanvas F_E_rootCanvas;
struct F_E_rootCanvas
{
	WORD	regs[NREG-1];
	E_Shape**	ret;
	uchar	temps[12];
};
void E_say(void*);
typedef struct F_E_say F_E_say;
struct F_E_say
{
	WORD	regs[NREG-1];
	WORD	noret;
	uchar	temps[12];
	String*	s;
};
void Shape_setAngle(void*);
typedef struct F_Shape_setAngle F_Shape_setAngle;
struct F_Shape_setAngle
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	angle;
};
void Shape_setAsMask(void*);
typedef struct F_Shape_setAsMask F_Shape_setAsMask;
struct F_Shape_setAsMask
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	WORD	is_mask;
};
void Shape_setDrawZ(void*);
typedef struct F_Shape_setDrawZ F_Shape_setDrawZ;
struct F_Shape_setDrawZ
{
	WORD	regs[NREG-1];
	struct{ WORD t0; WORD t1; }*	ret;
	uchar	temps[12];
	E_Shape*	this;
	E_Shape*	shape;
	WORD	n;
};
void Shape_setExecCbChan(void*);
typedef struct F_Shape_setExecCbChan F_Shape_setExecCbChan;
struct F_Shape_setExecCbChan
{
	WORD	regs[NREG-1];
	WORD	noret;
	uchar	temps[12];
	E_Shape*	this;
	Channel*	c;
};
void Shape_setExecCbPath(void*);
typedef struct F_Shape_setExecCbPath F_Shape_setExecCbPath;
struct F_Shape_setExecCbPath
{
	WORD	regs[NREG-1];
	struct{ String* t0; WORD t1; }*	ret;
	uchar	temps[12];
	E_Shape*	this;
	String*	new_path;
};
void Shape_setOffset(void*);
typedef struct F_Shape_setOffset F_Shape_setOffset;
struct F_Shape_setOffset
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	x;
	REAL	y;
};
void Shape_setScale(void*);
typedef struct F_Shape_setScale F_Shape_setScale;
struct F_Shape_setScale
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	uchar	_pad36[4];
	REAL	sx;
	REAL	sy;
};
void Shape_textAlign(void*);
typedef struct F_Shape_textAlign F_Shape_textAlign;
struct F_Shape_textAlign
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	String*	align;
};
void Shape_textFont(void*);
typedef struct F_Shape_textFont F_Shape_textFont;
struct F_Shape_textFont
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	String*	path;
};
void Shape_textFontSize(void*);
typedef struct F_Shape_textFontSize F_Shape_textFontSize;
struct F_Shape_textFontSize
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	WORD	size;
};
void Shape_textString(void*);
typedef struct F_Shape_textString F_Shape_textString;
struct F_Shape_textString
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	String*	txt;
};
void Shape_textViewPort(void*);
typedef struct F_Shape_textViewPort F_Shape_textViewPort;
struct F_Shape_textViewPort
{
	WORD	regs[NREG-1];
	WORD*	ret;
	uchar	temps[12];
	E_Shape*	this;
	WORD	w;
	WORD	h;
};
void E_tst(void*);
typedef struct F_E_tst F_E_tst;
struct F_E_tst
{
	WORD	regs[NREG-1];
	WORD	noret;
	uchar	temps[12];
	String*	s;
};
void Shape_undoGraph(void*);
typedef struct F_Shape_undoGraph F_Shape_undoGraph;
struct F_Shape_undoGraph
{
	WORD	regs[NREG-1];
	WORD	noret;
	uchar	temps[12];
	E_Shape*	this;
};
void Shape_updGraph(void*);
typedef struct F_Shape_updGraph F_Shape_updGraph;
struct F_Shape_updGraph
{
	WORD	regs[NREG-1];
	WORD	noret;
	uchar	temps[12];
	E_Shape*	this;
};
#define E_PATH "$E"
