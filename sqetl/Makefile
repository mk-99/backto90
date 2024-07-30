#

CC = gcc
CFLAGS = -g -O0 -Wall -funsigned-char
CDEFS = -DUNIX -D__LINUX__
COPT = -Lsmapi -Lcjson -lsmapilnx -lcjson -largp -liconv
OBJ = .o

TARGET = makesqbase readsqbase

all: $(TARGET)

makesqbase: makesqbase.c
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -o makesqbase makesqbase.c fconv.c

readsqbase: readsqbase.c
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -o readsqbase readsqbase.c fconv.c

fconvtest: fconvtest.c fconv.c
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -o fconvtest fconvtest.c fconv.c

clean:
	-rm -f *.o

distclean: clean
	-rm -f $(TARGET)
