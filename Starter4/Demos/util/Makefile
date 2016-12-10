
# Makefile for OpenGL teaching/examples 



## Linux
CC	= gcc
CFLAGS	=-DGL_GLEXT_PROTOTYPES -Wall
LDFLAGS	= -L/usr/X11R6/lib
GLIBS	= -lglut -lGLU -lGL -lX11 -lXi -lXmu -lm

SRCS = \
	utility.c	\
	glUtils.c	\
	textfile.c

TARGET = make

$(TARGET) : $(patsubst %.c, %.o, $(SRCS))

%.o : %.c
	/bin/rm -f $@
	$(CC) $(CFLAGS) -c -o $@ $<  $(LDFLAGS) $(GLIBS)

clean:
	/bin/rm -f *.o


