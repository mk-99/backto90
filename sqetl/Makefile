# $Id: makefile.lnx,v 1.32 2002/11/12 13:47:21 stas_degteff Exp $
# Legacy makefile for building on Linux with gcc
# No support for the Husky build environment

CC=	gcc
CFLAGS=	-Wall -funsigned-char
CDEFS=	-DUNIX -D__LINUX__
# COPT=	-O3 -Lsmapi -lsmapilnx
COPT= -g -O0 -Lsmapi -lsmapilnx
OBJ=	.o

# on i386, you may add this
# -fomit-frame-pointer -fstrength-reduce -fPIC -m486

TARGET=	makesqbase

all: $(TARGET)

# Thunderbirds are go!

$(TARGET):
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -o $(TARGET) makesqbase.c

clean:
	-rm *.o

distclean: clean
	-rm $(TARGET)
