
OFILES=\
	$OFILES\
	e.$O\

#HFILES=\

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

#nuke:EV:
#	rm -f e.h emod.h

