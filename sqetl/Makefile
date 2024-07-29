#

CC = gcc
CFLAGS = -g -O0 -Wall -funsigned-char
CDEFS = -DUNIX -D__LINUX__
COPT = -Lsmapi -Lcjson -lsmapilnx -lcjson -largp
OBJ = .o

TARGET = makesqbase readsqbase

all: $(TARGET)

makesqbase: makesqbase.c
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -o makesqbase makesqbase.c

readsqbase: readsqbase.c
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -o readsqbase readsqbase.c

clean:
	-rm -f *.o

distclean: clean
	-rm -f $(TARGET)
