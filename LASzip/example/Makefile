# makefile for laszippertest
#
#COPTS		= -g -Wall
COPTS		= -O3 -Wall
COMPILER	= g++
LINKER		= g++
#BITS		= -64

#LIBS		= -L/usr/lib32
#LIBS		= -L/usr/lib64
INCLUDE		= -I/usr/include

LAZLIBS		=
LAZINCLUDE	= -I../src

LAZOBJS		= ../src/laszip.o ../src/laszipper.o ../src/lasunzipper.o ../src/lasreadpoint.o ../src/lasreaditemcompressed_v1.o ../src/lasreaditemcompressed_v2.o ../src/laswritepoint.o  ../src/laswriteitemcompressed_v1.o ../src/laswriteitemcompressed_v2.o ../src/integercompressor.o ../src/arithmeticdecoder.o ../src/arithmeticencoder.o ../src/arithmeticmodel.o

all: laszippertest

laszippertest: laszippertest.o ${LAZOBJS}
	${LINKER} ${BITS} ${COPTS} laszippertest.o ${LAZOBJS} -o $@ ${LIBS} ${LAZLIBS} $(INCLUDE) $(LAZINCLUDE)

.cpp.o: 
	${COMPILER} ${BITS} -c ${COPTS} ${INCLUDE} $(LAZINCLUDE) $< -o $@

.c.o: 
	${COMPILER} ${BITS} -c ${COPTS} ${INCLUDE} $(LAZINCLUDE) $< -o $@

clean:
	rm -rf *.o
	rm -rf laszippertest

clobber:
	rm -rf *.o
	rm -rf laszippertest
	rm -rf *~
