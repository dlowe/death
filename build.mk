X11_LIBDIR := /usr/X11R6/lib
X11_INCDIR := /usr/X11R6/include

prog: prog.c
	${CC} -I${X11_INCDIR} -L${X11_LIBDIR} -D_BSD_SOURCE $< -o $@ -lX11
