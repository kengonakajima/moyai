
MOYAISRCS=moyai.cpp cumino.cpp
DEMO2DSRCS=demo2d.cpp 
DEMO2DOBJS=$(DEMO2DSRCS:.cpp=.o)
DEMO3DSRCS=demo3d.cpp
DEMO3DOBJS=$(DEMO3DSRCS:.cpp=.o)
MOYAIOBJS=$(MOYAISRCS:.cpp=.o)

FREETYPE=freetype-2.4.10
FREETYPELIB=$(FREETYPE)/objs/.libs/libfreetype.a  # build product of freetype source

BZ2=bzip2-1.0.6
BZ2LIB=$(BZ2)/libbz2.a # build product of bz2 source

ZLIB=zlib-1.2.7
ZLIBLIB=$(ZLIB)/libz.a

LIBPNG=libpng-1.5.13
LIBPNGLIB=$(LIBPNG)/.libs/libpng15.a

GLFW=glfw-2.7.7
GLFWLIB=$(GLFW)/lib/cocoa/libglfw.a

FTGLOBJS=vertex-attribute.o vertex-buffer.o vector.o texture-atlas.o texture-font.o
SOILOBJS=SOIL.o stb_image_aug.o image_DXT.o image_helper.o 
SOILLIB=libsoil.a
FTGLLIB=libftgl.a
OUTLIB=libmoyai.a

EXTLIBS= $(ZLIBLIB) $(BZ2LIB) $(LIBPNGLIB) $(FREETYPELIB) $(FTGLLIB) $(SOILLIB) $(GLFWLIB) 
LIBFLAGS=-framework Cocoa -framework IOKit -framework OpenGL -framework CoreFoundation  -m64  fmod/api/lib/libfmodex.dylib 
CFLAGS=-I$(FREETYPE)/include -g  -I./freetype-gl -Wall -m64  -I$(LIBPNG) -DUSE_FMOD -I$(GLFW)/include


DEMO2D=demo2d
DEMO3D=demo3d


all : $(DEMO2D) $(DEMO3D)


$(DEMO2D) : $(EXTLIBS) $(OUTLIB) $(DEMO2DOBJS)
	g++ $(CFLAGS) $(LIBFLAGS) $(DEMO2DOBJS) -o $(DEMO2D) $(OUTLIB) $(EXTLIBS)

$(DEMO3D) : $(EXTLIBS) $(OUTLIB) $(DEMO3DOBJS)
	g++ $(CFLAGS) $(LIBFLAGS) $(DEMO3DOBJS) -o $(DEMO3D) $(OUTLIB) $(EXTLIBS)

demo2d.o : demo2d.cpp moyai.h
	g++ -c demo2d.cpp $(CFLAGS)
demo3d.o : demo3d.cpp moyai.h
	g++ -c demo3d.cpp $(CFLAGS)

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

$(FREETYPELIB):
	rm -rf $(FREETYPE)
	tar zxf $(FREETYPE).tar.bz2
	cd $(FREETYPE); ./configure; make

$(BZ2LIB):
	rm -rf $(BZ2)
	tar zxf $(BZ2).tar.gz
	cd $(BZ2); make

$(ZLIBLIB):
	rm -rf $(ZLIB)
	tar zxf $(ZLIB).tar.gz
	cd $(ZLIB); ./configure; make

$(LIBPNGLIB):
	rm -rf $(LIBPNG)
	tar zxf $(LIBPNG).tar.gz
	cd $(LIBPNG); ./configure; make

$(GLFWLIB):
	cd $(GLFW); make cocoa


clean:
	rm -r $(FREETYPE) $(BZ2) $(ZLIB) $(LIBPNG)
	rm -f *.o deps.make $(DEMO2D) $(OUTLIB) *.o *.a
	make depend

depend: 
	$(CC) $(CFLAGS) -MM $(TESTSRCS) $(MOYAISRCS) > deps.make


-include deps.make
