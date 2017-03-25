# makefile for LGPL-licensed LAStools
#
#COPTS    = -g -Wall -Wno-deprecated -DDEBUG 
COPTS     = -O3 -Wall -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-unused-result -DNDEBUG 
#COMPILER  = CC
COMPILER  = g++
LINKER  = g++
#BITS     = -64

#LIBS     = -L/usr/lib64
#LIBS     = -L/usr/lib32
#INCLUDE  = -I/usr/include

LASLIBS     = -L../LASlib/lib
LASINCLUDE  = -I../LASzip/src -I../LASlib/inc 

all: laszip lasinfo lasprecision las2txt txt2las las2las lasdiff lasmerge lasindex

laszip: laszip.o geoprojectionconverter.o 
	${LINKER} ${BITS} ${COPTS} laszip.o geoprojectionconverter.o -llas -o $@ ${LIBS} ${LASLIBS} $(INCLUDE) $(LASINCLUDE)
	cp $@ ../bin

lasinfo: lasinfo.o geoprojectionconverter.o
	${LINKER} ${BITS} ${COPTS} lasinfo.o geoprojectionconverter.o -llas -o $@ ${LIBS} ${LASLIBS} $(INCLUDE) $(LASINCLUDE)
	cp $@ ../bin

lasprecision: lasprecision.o geoprojectionconverter.o
	${LINKER} ${BITS} ${COPTS} lasprecision.o geoprojectionconverter.o -llas -o $@ ${LIBS} ${LASLIBS} $(INCLUDE) $(LASINCLUDE)
	cp $@ ../bin

las2txt: las2txt.o
	${LINKER} ${BITS} ${COPTS} las2txt.o -llas -o $@ ${LIBS} ${LASLIBS} $(INCLUDE) $(LASINCLUDE)
	cp $@ ../bin

txt2las: txt2las.o geoprojectionconverter.o
	${LINKER} ${BITS} ${COPTS} txt2las.o geoprojectionconverter.o -llas -o $@ ${LIBS} ${LASLIBS} $(INCLUDE) $(LASINCLUDE)
	cp $@ ../bin

las2las: las2las.o geoprojectionconverter.o
	${LINKER} ${BITS} ${COPTS} las2las.o geoprojectionconverter.o -llas -o $@ ${LIBS} ${LASLIBS} $(INCLUDE) $(LASINCLUDE)
	cp $@ ../bin

lasdiff: lasdiff.o
	${LINKER} ${BITS} ${COPTS} lasdiff.o -llas -o $@ ${LIBS} ${LASLIBS} $(INCLUDE) $(LASINCLUDE)
	cp $@ ../bin

lasmerge: lasmerge.o geoprojectionconverter.o 
	${LINKER} ${BITS} ${COPTS} lasmerge.o geoprojectionconverter.o -llas -o $@ ${LIBS} ${LASLIBS} $(INCLUDE) $(LASINCLUDE)
	cp $@ ../bin

lasindex: lasindex.o
	${LINKER} ${BITS} ${COPTS} lasindex.o -llas -o $@ ${LIBS} ${LASLIBS} $(INCLUDE) $(LASINCLUDE)
	cp $@ ../bin

.cpp.o: 
	${COMPILER} ${BITS} -c ${COPTS} ${INCLUDE} $(LASINCLUDE) $< -o $@

.c.o: 
	${COMPILER} ${BITS} -c ${COPTS} ${INCLUDE} $(LASINCLUDE) $< -o $@

clean:
	rm -rf *.o
	rm -rf laszip lasinfo lasprecision las2txt txt2las las2las lasdiff lasmerge lasindex

clobber:
	rm -rf *.o
	rm -rf laszip lasinfo lasprecision las2txt txt2las las2las lasdiff lasmerge lasindex
	rm -rf *~
