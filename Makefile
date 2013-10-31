
MOYAICLISRCS=common.cpp client.cpp cumino.cpp
MOYAICLIOBJS=$(MOYAICLISRCS:.cpp=.o)
MOYAISVSRCS=common.cpp cumino.cpp
MOYAISVOBJS=$(MOYAISVSRCS:.cpp=.o)
DEMO2DSRCS=demo2d.cpp 
DEMO2DOBJS=$(DEMO2DSRCS:.cpp=.o)
DEMO3DSRCS=demo3d.cpp
DEMO3DOBJS=$(DEMO3DSRCS:.cpp=.o)
DEMOSVSRCS=demosv.cpp
DEMOSVOBJS=$(DEMOSVSRCS:.cpp=.o)



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
LZ4OBJS=lz4.o lz4hc.o

SOILLIB=libsoil.a
LZ4LIB=liblz4.a
FTGLLIB=libftgl.a
OUTCLILIB=libmoyaicl.a
OUTSVLIB=libmoyaisv.a

EXTCOMMONLIBS= $(ZLIBLIB) $(BZ2LIB) $(LIBPNGLIB)  $(LZ4LIB)
EXTCLILIBS = $(EXTCOMMONLIBS) $(FREETYPELIB) $(FTGLLIB) $(SOILLIB) $(GLFWLIB) 
CLILIBFLAGS=-framework Cocoa -framework IOKit -framework OpenGL -framework CoreFoundation  -m64  fmod/api/lib/libfmodex.dylib 
CFLAGS=-O0 -I$(FREETYPE)/include -g  -I./freetype-gl -Wall -m64  -I$(LIBPNG) -I$(GLFW)/include


DEMO2D=demo2d
DEMO3D=demo3d
DEMOSV=demosv

all : $(DEMO2D) $(DEMO3D) $(DEMOSV)



$(DEMO2D) : $(EXTCLILIBS) $(OUTCLILIB) $(DEMO2DOBJS) 
	g++ $(CFLAGS) $(CLILIBFLAGS) $(DEMO2DOBJS) -o $(DEMO2D) $(OUTCLILIB) $(EXTCLILIBS)

$(DEMO3D) : $(EXTCLILIBS) $(OUTCLILIB) $(DEMO3DOBJS)
	g++ $(CFLAGS) $(CLILIBFLAGS) $(DEMO3DOBJS) -o $(DEMO3D) $(OUTCLILIB) $(EXTCLILIBS)

$(DEMOSV) : $(EXTCOMMONLIBS) $(OUTSVLIB) $(DEMOSVOBJS)
	g++ $(CFLAGS) -m64 $(DEMOSVOBJS) -o $(DEMOSV) $(OUTSVLIB) $(EXTCOMMONLIBS)


demosv.o : demosv.cpp
	g++ -c demosv.cpp $(CFLAGS)
demo2d.o : demo2d.cpp 
	g++ -c demo2d.cpp $(CFLAGS)
demo3d.o : demo3d.cpp 
	g++ -c demo3d.cpp $(CFLAGS)

$(OUTCLILIB) : $(MOYAICLIOBJS)
	ar cr $(OUTCLILIB) $(MOYAICLIOBJS)
	ranlib $(OUTCLILIB)

$(OUTSVLIB) : $(MOYAISVOBJS)
	ar cr $(OUTSVLIB) $(MOYAISVOBJS)
	ranlib $(OUTSVLIB)

$(FTGLLIB) : $(FTGLOBJS)
	ar cr $(FTGLLIB) $(FTGLOBJS)
	ranlib $(FTGLLIB)

$(SOILLIB) : $(SOILOBJS)
	ar cr $(SOILLIB) $(SOILOBJS)
	ranlib $(SOILLIB)

$(LZ4LIB) : $(LZ4OBJS)
	ar cr $(LZ4LIB) $(LZ4OBJS)
	ranlib $(LZ4LIB)


common.o : common.cpp
	g++ -c common.cpp $(CFLAGS)
client.o : client.cpp
	g++ -c client.cpp $(CFLAGS)
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

lz4.o:
	g++ -c lz4/lz4.c $(CFLAGS)

lz4hc.o:
	g++ -c lz4/lz4hc.c $(CFLAGS)

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



$(GLFW) :
	rm -rf $(GLFW)
	unzip $(GLFW).zip

$(GLFWLIB): $(GLFW)
	cd $(GLFW); make cocoa


clean:
	rm -rf $(FREETYPE) $(BZ2) $(ZLIB) $(LIBPNG) $(GLFW) 
	rm -f *.o deps.make $(DEMO2D) $(DEMO3D) $(DEMOSV) $(OUTCLILIB) $(OUTSVLIB) *.o *.a

depend: $(GLFWLIB)
	$(CC) $(CFLAGS) -MM $(TESTSRCS) $(MOYAICLISRCS) $(DEMO2DSRCS) $(DEMO3DSRCS)  > deps.make


-include deps.make
