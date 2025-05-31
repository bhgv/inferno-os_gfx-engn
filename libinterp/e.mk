
OFILES=\
	$OFILES\
	graph.$O\
	render.$O\
	channel_wrk.$O\
	aux_svg.$O\
	aux_png.$O\
	win_dep.$O\
	e.$O\

HFILES=\
	graph/graph.h\

#	$MODULES\
#	../module/e.m\

EMOD=\
	../module/e.m\


default:V:	all

all install:V:	$OFILES


e.h:D: $EMOD
	rm -f $target && limbo -a -I../module ../module/e.m > $target

emod.h:D: $EMOD
	rm -f $target && limbo -t E -I../module ../module/e.m > $target

e.$O:		e.h emod.h

graph.$O:	graph/graph.c graph/graph.h
	$CC $CFLAGS -o $target graph/graph.c

render.$O:	graph/render.c graph/graph.h
	$CC $CFLAGS -o $target graph/render.c

channel_wrk.$O:	graph/channel_wrk.c
	$CC $CFLAGS -o $target graph/channel_wrk.c

aux_svg.$O:	graph/aux_svg.c
	$CC $CFLAGS -o $target graph/aux_svg.c

aux_png.$O:	graph/aux_png.c
	$CC $CFLAGS -o $target graph/aux_png.c

win_dep.$O:	graph/win_dep.c graph/graph.h
	$CC $CFLAGS -o $target graph/win_dep.c


#nuke:EV:
#	rm -f e.h emod.h

