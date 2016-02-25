
MOYAICLISRCS=common.cpp cumino.cpp  lodepng.cpp Prop2D.cpp Prop3D.cpp ColorReplacerShader.cpp Font.cpp FragmentShader.cpp IndexBuffer.cpp Layer.cpp MoyaiClient.cpp TextBox.cpp Prim.cpp Texture.cpp VertexBuffer.cpp Viewport.cpp DrawBatch.cpp Camera.cpp CharGrid.cpp Grid.cpp Mesh.cpp Pad.cpp PerformanceCounter.cpp PrimDrawer.cpp Sound.cpp SoundSystem.cpp VertexFormat.cpp TileDeck.cpp Remote.cpp net.cpp

MOYAICLIOBJS=$(MOYAICLISRCS:.cpp=.o)
MOYAISVSRCS=common.cpp cumino.cpp  lodepng.cpp Camera.cpp Grid.cpp
MOYAISVOBJS=$(MOYAISVSRCS:.cpp=.o)
DEMO2DSRCS=demo2d.cpp 
DEMO2DOBJS=$(DEMO2DSRCS:.cpp=.o)
DEMO3DSRCS=demo3d.cpp
DEMO3DOBJS=$(DEMO3DSRCS:.cpp=.o)
DEMOSVSRCS=demosv.cpp
DEMOSVOBJS=$(DEMOSVSRCS:.cpp=.o)
MIN2DSRCS=min2d.cpp
MIN2DOBJS=$(MIN2DSRCS:.cpp=.o)


FREETYPE=freetype-2.4.10
FREETYPELIB=$(FREETYPE)/objs/.libs/libfreetype.a  # build product of freetype source

BZ2=bzip2-1.0.6
BZ2LIB=$(BZ2)/libbz2.a # build product of bz2 source

ZLIB=zlib-1.2.7
ZLIBLIB=$(ZLIB)/libz.a

GLFW=glfw-3.1.2
GLFWLIB=$(GLFW)/src/libglfw3.a

FTGLOBJS=vertex-attribute.o vertex-buffer.o vector.o texture-atlas.o texture-font.o
SNAPPYOBJS=snappy/snappy-sinksource.o snappy/snappy-c.o snappy/snappy.o

FTGLLIB=libftgl.a
OUTCLILIB=libmoyaicl.a
OUTSVLIB=libmoyaisv.a
SNAPPYLIB=libsnappy.a

EXTCOMMONLIBS= $(ZLIBLIB) $(BZ2LIB) $(LIBPNGLIB) $(SNAPPYLIB)
EXTCLILIBS = $(EXTCOMMONLIBS) $(FREETYPELIB) $(FTGLLIB) $(GLFWLIB) 
CLILIBFLAGS=-framework Cocoa -framework IOKit -framework OpenGL -framework CoreFoundation -framework CoreVideo -m64  fmod/api/lib/libfmodex.dylib 
CFLAGS=-O0 -I/usr/local/include -I$(FREETYPE)/include -g  -I./freetype-gl -Wall -m64  -I./$(GLFW)/include


DEMO2D=demo2d
DEMO3D=demo3d
DEMOSV=demosv
MIN2D=min2d

all : $(DEMO2D) $(DEMO3D) $(DEMOSV) $(MIN2D)

server : $(OUTSVLIB) $(SNAPPYLIB)

$(MIN2D) : $(EXTCLILIBS) $(OUTCLILIB) $(MIN2DOBJS) $(BZ2LIB) $(ZLIBLIB)
	g++ $(CFLAGS) $(CLILIBFLAGS) $(MIN2DOBJS) -o $(MIN2D) $(OUTCLILIB) $(EXTCLILIBS)

$(DEMO2D) : $(EXTCLILIBS) $(OUTCLILIB) $(DEMO2DOBJS) $(BZ2LIB) $(ZLIBLIB) 
	g++ $(CFLAGS) $(CLILIBFLAGS) $(DEMO2DOBJS) -o $(DEMO2D) $(OUTCLILIB) $(EXTCLILIBS)

$(DEMO3D) : $(EXTCLILIBS) $(OUTCLILIB) $(DEMO3DOBJS) $(BZ2LIB) $(ZLIBLIB) 
	g++ $(CFLAGS) $(CLILIBFLAGS) $(DEMO3DOBJS) -o $(DEMO3D) $(OUTCLILIB) $(EXTCLILIBS)

$(DEMOSV) : $(EXTCOMMONLIBS) $(OUTSVLIB) $(DEMOSVOBJS)
	g++ $(CFLAGS) -m64 $(DEMOSVOBJS) -o $(DEMOSV) $(OUTSVLIB) $(EXTCOMMONLIBS)


demosv.o : demosv.cpp
	g++ -c demosv.cpp $(CFLAGS)
demo2d.o : demo2d.cpp 
	g++ -c demo2d.cpp $(CFLAGS)
demo3d.o : demo3d.cpp 
	g++ -c demo3d.cpp $(CFLAGS)
min2d.o : min2d.cpp
	g++ -c min2d.cpp $(CFLAGS)

$(OUTCLILIB) : $(MOYAICLIOBJS)
	ar cr $(OUTCLILIB) $(MOYAICLIOBJS)
	ranlib $(OUTCLILIB)

$(OUTSVLIB) : $(MOYAISVOBJS)
	ar cr $(OUTSVLIB) $(MOYAISVOBJS)
	ranlib $(OUTSVLIB)

$(FTGLLIB) : $(FTGLOBJS)
	ar cr $(FTGLLIB) $(FTGLOBJS)
	ranlib $(FTGLLIB)

$(SNAPPYLIB) : $(SNAPPYOBJS)
	ar cr $(SNAPPYLIB) $(SNAPPYOBJS)
	ranlib $(SNAPPYLIB)

common.o : common.cpp
	g++ -c common.cpp $(CFLAGS)

cumino.o : cumino.cpp
	g++ -c cumino.cpp $(CFLAGS)
Prop2D.o : Prop2D.cpp
	g++ -c $(CFLAGS) Prop2D.cpp  -o Prop2D.o
Prop3D.o : Prop3D.cpp
	g++ -c $(CFLAGS) Prop3D.cpp  -o Prop3D.o
ColorReplacerShader.o : ColorReplacerShader.cpp
	g++ -c $(CFLAGS) ColorReplacerShader.cpp -o ColorReplacerShader.o
Font.o : Font.cpp
	g++ -c $(CFLAGS) Font.cpp -o Font.o
FragmentShader.o : FragmentShader.cpp
	g++ -c $(CFLAGS) FragmentShader.cpp -o FragmentShader.o
IndexBuffer.o : IndexBuffer.cpp
	g++ -c $(CFLAGS) IndexBuffer.cpp -o IndexBuffer.o
Layer.o : Layer.cpp
	g++ -c $(CFLAGS) Layer.cpp -o Layer.o
MoyaiClient.o : MoyaiClient.cpp
	g++ -c $(CFLAGS) MoyaiClient.cpp -o MoyaiClient.o
TextBox.o : TextBox.cpp
	g++ -c $(CFLAGS) TextBox.cpp -o TextBox.o
Texture.o : Texture.cpp
	g++ -c $(CFLAGS) Texture.cpp -o Texture.o
VertexBuffer.o : VertexBuffer.cpp
	g++ -c $(CFLAGS) VertexBuffer.cpp -o VertexBuffer.o
Viewport.o : Viewport.cpp
	g++ -c $(CFLAGS) Viewport.cpp -o Viewport.o
Prim.o : Prim.cpp
	g++ -c $(CFLAGS) Prim.cpp -o Prim.o
DrawBatch.o : DrawBatch.cpp
	g++ -c $(CFLAGS) DrawBatch.cpp -o DrawBatch.o
Camera.o : Camera.cpp
	g++ -c $(CFLAGS) Camera.cpp -o Camera.o
CharGrid.o : CharGrid.cpp
	g++ -c $(CFLAGS) CharGrid.cpp -o CharGrid.o
Grid.o : Grid.cpp
	g++ -c $(CFLAGS) Grid.cpp -o Grid.o
Mesh.o : Mesh.cpp
	g++ -c $(CFLAGS) Mesh.cpp -o Mesh.o
Pad.o : Pad.cpp
	g++ -c $(CFLAGS) Pad.cpp -o Pad.o
PerformanceCounter.o : PerformanceCounter.cpp
	g++ -c $(CFLAGS) PerformanceCounter.cpp -o PerformanceCounter.o
PrimDrawer.o : PrimDrawer.cpp
	g++ -c $(CFLAGS) PrimDrawer.cpp -o PrimDrawer.o
Sound.o : Sound.cpp
	g++ -c $(CFLAGS) Sound.cpp -o Sound.o
SoundSystem.o : SoundSystem.cpp
	g++ -c $(CFLAGS) SoundSystem.cpp -o SoundSystem.o
VertexFormat.o : VertexFormat.cpp
	g++ -c $(CFLAGS) VertexFormat.cpp -o VertexFormat.o
TileDeck.o : TileDeck.cpp
	g++ -c $(CFLAGS) TileDeck.cpp -o TileDeck.o
Remote.o : Remote.cpp
	g++ -c $(CFLAGS) Remote.cpp -o Remote.o
net.o : net.cpp
	g++ -c $(CFLAGS) net.cpp -o net.o

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


snappy-sinksource.o:
	g++ -c snappy/snappy-sinksource.cc $(CFLAGS)
snappy-c.o:
	g++ -c snappy/snappy-c.cc $(CFLAGS)
snappy.o:
	g++ -c snappy/snappy.cc $(CFLAGS)

$(FREETYPELIB):
	rm -rf $(FREETYPE)
	tar jxf $(FREETYPE).tar.bz2
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
	cd $(GLFW); cmake .; make


clean:
	make -C $(GLFW) clean
	rm -rf $(FREETYPE) $(BZ2) $(ZLIB) $(LIBPNG) 
	rm -f deps.make $(DEMO2D) $(MIN2D) $(DEMO3D) $(DEMOSV) $(OUTCLILIB) $(OUTSVLIB) *.o *.a */*.o

depend: $(GLFWLIB)
	$(CC) $(CFLAGS) -MM $(TESTSRCS) $(MOYAICLISRCS) $(DEMO2DSRCS) $(DEMO3DSRCS)  > deps.make


-include deps.make
