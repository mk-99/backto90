# $Id: makefile.lnx,v 1.32 2002/11/12 13:47:21 stas_degteff Exp $
# Legacy makefile for building on Linux with gcc
# No support for the Husky build environment

CC=	gcc
CFLAGS=	-Wall -funsigned-char
CDEFS=	-DUNIX -D__LINUX__
COPT=	-g -O0
OBJ=	.o

# on i386, you may add this
# -fomit-frame-pointer -fstrength-reduce -fPIC -m486

LIB=	ar
RANLIB= ranlib

TARGET=	libsmapilnx.a

all: $(TARGET)

include makefile.inc


# Thunderbirds are go!

.c.o:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -c $<

$(TARGET): $(OBJS)
	$(LIB) r $@ $?
	$(RANLIB) $(TARGET)

clean:
	-rm *.o

distclean: clean
	-rm $(TARGET)
