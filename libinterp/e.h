typedef struct E_GrOp E_GrOp;
typedef struct E_Level E_Level;
typedef struct E_Shape E_Shape;
struct E_GrOp
{
	Array*	fill;
	String*	svg;
};
#define E_GrOp_size 8
#define E_GrOp_map {0xc0,}
struct E_Level
{
	E_GrOp	graphics;
};
#define E_Level_size 8
#define E_Level_map {0xc0,}
struct E_Shape
{
	WORD	id;
};
#define E_Shape_size 4
#define E_Shape_map {0}
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
void E_getLevelsList(void*);
typedef struct F_E_getLevelsList F_E_getLevelsList;
struct F_E_getLevelsList
{
	WORD	regs[NREG-1];
	Array**	ret;
	uchar	temps[12];
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
void E_tst(void*);
typedef struct F_E_tst F_E_tst;
struct F_E_tst
{
	WORD	regs[NREG-1];
	WORD	noret;
	uchar	temps[12];
	String*	s;
};
#define E_PATH "$E"
