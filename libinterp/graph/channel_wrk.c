

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


#define SEND_STRING_LEN_MAX  256


static void movtmp_(REG* R)		/* Used by send & receive */
{
	Type *t;

	t = (Type*)*((WORD*)(R->m));

	incmem(R->s, t);
	if (t->np)
		freeptrs(R->d, t);
	memmove(R->d, R->s, t->size);
}


static void movp_(REG* R)
{
	Heap *h;
	WORD *dv, *sv;

	sv = *((WORD**)(R->s));
	if(sv != H) {
		h = D2H(sv);
		h->ref++;
		Setmark(h);
	}
	dv = *((WORD**)(R->d));
	*((WORD**)(R->d)) = sv;
	destroy(dv);
}


static int
cputb_(REG* R, Channel *c, void *v, void	(*mover)(REG*))
{
	Array *a;
	WORD len, r;

	if((a = c->buf) == H)
		return 0;
	len = c->buf->len;
	if(c->size < len){
		r = c->front + c->size;
		if(r >= len)
			r -= len;
		c->size++;
		R->s = v;
		R->m = &c->mid;
		R->d = a->data + r * a->t->size;
//		c->mover();
//		movp_(R);
		mover(R);
		return 1;
	}
	return 0;
}

static void
isend_(REG* R, void	(*mover)(REG*))
{
	Channel *c;
 	Prog *p;
//D();

	c = *((Channel**)(R->d));
	if(c == H)
		error(/*exNilref*/"dereference of nil");

//D();
	if((p = c->recv->prog) == nil) {
//D();
		if(c->buf != H && cputb_(R, c, R->s, mover))
			return;

//D();
		p = delrun(Psend);
//		if (p == nil)
//			return;

//D();
		p->ptr = R->s;
		p->chan = c;	/* for killprog */
		R->IC = 1;	
		R->t = 1;
		cqadd(&c->send, p);
//D();
		return;
	}

//D();
	if(c->buf != H && c->size > 0)
		print("non-empty buffer in isend\n");

//D();
	cqdel(&c->recv);
	if(p->state == Palt)
		altdone(p->R.s, p, c, 1);

//D();
	R->m = &c->mid;
	R->d = p->ptr;
	p->ptr = nil;
//	c->mover();
//	movp_(R);
	mover(R);
//DD("p: %p", p);
	addrun(p);
	R->t = 0;
//D();
}

#if 0
OP(irecv)
{
	Channel *c;
	Prog *p;

	c = C(s);
	if(c == H)
		error(exNilref);

	if((p = c->send->prog) == nil) {
		if(c->buf != H && cgetb(c, R.d))
			return;
		p = delrun(Precv);
		p->ptr = R.d;
		p->chan = c;	/* for killprog */
		R.IC = 1;
		R.t = 1;
		cqadd(&c->recv, p);
		return;
	}

	if(c->buf != H && c->size != c->buf->len)
		print("non-full buffer in irecv\n");

	cqdel(&c->send);
	if(p->state == Palt)
		altdone(p->R.s, p, c, 0);

	if(c->buf != H){
		cgetb(c, R.d);
		cputb(c, p->ptr);
		p->ptr = nil;
	}
	else{
		R.m = &c->mid;
		R.s = p->ptr;
		p->ptr = nil;
		c->mover();
	}
	addrun(p);
	R.t = 0;
}
#endif


int
csend_mouse_evt_(graphic_el* el, char* evt, double lx, double ly, double gx, double gy) //void *ip, Type *t, int len)
{
	REG R; //rsav;

	if (el == NULL || el->exec_mod_chan == H)
		return 0;

	Channel *c = el->exec_mod_chan;

	if(c == H)
		error(/*exNilref*/"dereference of nil");
	if(c->recv->prog == nil || c->buf != H)
		return 0;

#if 0
	struct {
		E_Shape* shp[1];
		String*  evt[1];
		double   x;
		double   y;
	} snd_dat;

	snd_dat.shp[0] = shape_by_graphic_el(el);

//	(D2H(snd_dat.shp[0]))->ref--;

	snd_dat.evt[0] = H;
	retstr(evt, &snd_dat.evt[0]);
	heap_ref_inc(D2H(snd_dat.evt[0]));

//	heap_ref_inc(D2H(snd_dat.shp[0]));

	snd_dat.x = lx;
	snd_dat.y = ly;

//	heap_ref_inc(D2H(snd_dat.shp));
//	heap_ref_inc(D2H(snd_dat.evt));
#if 0
	if(c->recv->prog == nil && (c->buf == H || c->size == c->buf->len)){
		if(c->buf != H){
			print("csendalt failed\n");
			freeptrs(ip, t);
			return 0;
		}
		c->buf = H2D(Array*, heaparray(t, len));
	}
#endif

//DD("> Before > c->recv->prog(%p) == nil && (c->buf(%p) == H || c->size(%d) == c->buf->len(%d)", c->recv->prog, c->buf, c->size, c->buf->len);
//DD("c->recv->prog: %p", c->recv->prog)
	R.s = &snd_dat; //ip;
	R.d = &c;
	isend_(&R, movtmp_);

#else

	E_Shape* shp;
	String*  strevt;

	E_ShapeEvent* ev;

gclock();

	shp = shape_by_graphic_el(el);

	strevt = H;
	retstr(evt, &strevt);
	heap_ref_inc(D2H(strevt));

	ev = H2D(E_ShapeEvent*, heap(TShapeEvent_MOUSE));

	ev->pick = E_ShapeEvent_MOUSE;

	ev->shape = shp;
	ev->u.MOUSE.data.t0 = strevt;

	ev->u.MOUSE.data.t1.x = lx;
	ev->u.MOUSE.data.t1.y = ly;
	ev->u.MOUSE.data.t2.x = gx;
	ev->u.MOUSE.data.t2.y = gy;


//DD("> Before > c->recv->prog(%p) == nil && (c->buf(%p) == H || c->size(%d) == c->buf->len(%d)", c->recv->prog, c->buf, c->size, c->buf->len);
//DD("c->recv->prog: %p", c->recv->prog)
	R.s = &ev;
	R.d = &c;
	isend_(&R, movp_);
gcunlock();

#endif

	return 1;
}



int
csend_touch_evt_(graphic_el* el, E_XY_coord* arevt)
{
	REG R; //rsav;

	if (el == NULL || el->exec_mod_chan == H || arevt == NULL)
		return 0;

	Channel *c = el->exec_mod_chan;

	if(c == H)
		error(/*exNilref*/"dereference of nil");
	if(c->recv->prog == nil || c->buf != H)
		return 0;

	gclock();

	Array* ar = H2D(Array*, heaparray(TXY_coord, MAXPOINTS));
	E_XY_coord* tp = (E_XY_coord*)ar->data;

	memcpy(tp, arevt, sizeof(E_XY_coord) * MAXPOINTS);

	E_Shape* shp;

	E_ShapeEvent* ev;

	shp = shape_by_graphic_el(el);

	heap_ref_inc(D2H(arevt));

	ev = H2D(E_ShapeEvent*, heap(TShapeEvent_TOUCH));

	ev->pick = E_ShapeEvent_TOUCH;

	ev->shape = shp;
	ev->u.TOUCH.data = ar;

//DD("> Before > c->recv->prog(%p) == nil && (c->buf(%p) == H || c->size(%d) == c->buf->len(%d)", c->recv->prog, c->buf, c->size, c->buf->len);
//DD("c->recv->prog: %p", c->recv->prog)
	R.s = &ev;
	R.d = &c;
	isend_(&R, movp_);

	gcunlock();

	return 1;
}




static int
csendalt_(Channel *c, void *ip, Type *t, int len)
{
	REG R; //rsav;

	if(c == H)
		error(/*exNilref*/"dereference of nil");

	if(c->recv->prog == nil && (c->buf == H || c->size == c->buf->len)){
		if(c->buf != H){
			print("csendalt failed\n");
			freeptrs(ip, t);
			return 0;
		}
		c->buf = H2D(Array*, heaparray(t, len));
	}

//	rsav = R;
	R.s = ip;
	R.d = &c;
	isend_(&R, movp_);
//	R = rsav;
	freeptrs(ip, t);
	return 1;
}


static int sendStringToGraphEl(graphic_el* el, char *msg)
{
	void *ptrs[1];
	int r;

	if (el == NULL || el->exec_mod_chan == H)
		return 0;

	ptrs[0] = H;
	retstr(msg, (String**) &ptrs[0]);

/*
	acquire();
	if(waserror()){
		release();
		nexterror();
	}
*/
	r = csendalt_(el->exec_mod_chan, ptrs, &Tptr, SEND_STRING_LEN_MAX);
/*
	poperror();
	release();
*/
	return r;
}

static int printfToGraphEl(graphic_el *el, char *fmt, ...)
{
	char *buf;
	int r;
	va_list arg;

	if (el == NULL || el->exec_mod_chan == H)
		return 0;

	va_start(arg, fmt);
	buf = vsmprint(fmt, arg);
	va_end(arg);
	r = sendStringToGraphEl(el, buf);
	free(buf);

	return r;
}


