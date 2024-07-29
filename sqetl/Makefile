#

CC = gcc
CFLAGS = -g -O0 -Wall -funsigned-char
CDEFS = -DUNIX -D__LINUX__
COPT = -Lsmapi -Lcjson -lsmapilnx -lcjson -largp
OBJ = .o

TARGET = makesqbase

all: $(TARGET)

makesqbase: makesqbase.c
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -o $(TARGET) makesqbase.c

sqcmdline: sqcmdline.c
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -o sqcmdline sqcmdline.c

clean:
	-rm -f *.o

distclean: clean
	-rm -f $(TARGET)
