LIBFLAGS=-framework OpenGL -framework GLUT  -framework CoreFoundation  -m64  fmod/api/lib/libfmodex.dylib 
CFLAGS=-Ifreetype-2.4.10/include -g  -I./freetype-gl -Wall -m64  -Ilibpng-1.5.13/ -DUSE_FMOD



MOYAISRCS=moyai.cpp cumino.cpp
TESTSRCS=test.cpp 
TESTOBJS=$(TESTSRCS:.cpp=.o)
MOYAIOBJS=$(MOYAISRCS:.cpp=.o)
FTLIB=freetype-2.4.10/objs/.libs/libfreetype.a  # ビルドしたらできるよ
BZ2LIB=bzip2-1.0.6/libbz2.a # ビルドしたらできるよ
ZLIB=zlib-1.2.7/libz.a
FTGLOBJS=vertex-attribute.o vertex-buffer.o vector.o texture-atlas.o texture-font.o
SOILOBJS=SOIL.o stb_image_aug.o image_DXT.o image_helper.o 
SOILLIB=libsoil.a
FTGLLIB=libftgl.a
LIBPNGLIB=libpng-1.5.13/.libs/libpng15.a
OUTLIB=libmoyai.a


all : test

test : $(TESTOBJS) $(OUTLIB) $(SOILLIB) $(FTGLLIB) $(FTLIB) $(ZLIB) $(BZ2LIB)
	g++ $(CFLAGS) $(LIBFLAGS) $(TESTOBJS) -o test $(OUTLIB) $(SOILLIB) $(FTGLLIB) $(FTLIB) $(ZLIB) $(BZ2LIB) $(LIBPNGLIB)

test.o : test.cpp
	g++ -c test.cpp $(CFLAGS)


$(OUTLIB) : $(MOYAIOBJS)
	ar cr $(OUTLIB) $(MOYAIOBJS)
	ranlib $(OUTLIB)
$(FTGLLIB) : $(FTGLOBJS)
	ar cr $(FTGLLIB) $(FTGLOBJS)
	ranlib $(FTGLLIB)
$(SOILLIB) : $(SOILOBJS)
	ar cr $(SOILLIB) $(SOILOBJS)
	ranlib $(SOILLIB) 

moyai.o : moyai.cpp
	g++ -c moyai.cpp $(CFLAGS)
cumino.o : cumino.cpp
	g++ -c cumino.cpp $(CFLAGS)

# freetype-gl
texture-atlas.o :
	g++ -c freetype-gl/texture-atlas.c $(CFLAGS)
texture-font.o :
	g++ -c freetype-gl/texture-font.c $(CFLAGS)
vector.o :
	g++ -c freetype-gl/vector.c $(CFLAGS)
vertex-buffer.o :
	g++ -c freetype-gl/vertex-buffer.c $(CFLAGS)
vertex-attribute.o :
	g++ -c freetype-gl/vertex-attribute.c $(CFLAGS)

# SOIL
SOIL.o:
	g++ -c soil/src/SOIL.c $(CFLAGS) 

stb_image_aug.o:
	g++ -c soil/src/stb_image_aug.c $(CFLAGS)

image_DXT.o:
	g++ -c soil/src/image_DXT.c $(CFLAGS)

image_helper.o:
	g++ -c soil/src/image_helper.c $(CFLAGS)


clean:
	rm -f *.o deps.make test $(OUTLIB) *.o *.a
	make depend

depend: 
	$(CC) $(CFLAGS) -MM $(TESTSRCS) $(MOYAISRCS) > deps.make




auto:
	ruby auto.rb

-include deps.make
