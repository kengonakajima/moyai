FTDIR=../../freetype-2.4.10
INCDIRS=-I $(FTDIR)/include/freetype -I $(FTDIR)/include
SRCS=main.c $(FTDIR)/src/base/ftobjs.c $(FTDIR)/src/base/ftapi.c $(FTDIR)/src/base/ftstream.c $(FTDIR)/src/base/ftsystem.c $(FTDIR)/src/base/ftutil.c $(FTDIR)/src/base/ftoutln.c $(FTDIR)/src/base/ftglyph.c $(FTDIR)/src/base/ftgloadr.c $(FTDIR)/src/base/ftcalc.c $(FTDIR)/src/base/ftrfork.c


all: freetype.js

freetype.js: Makefile main.c
	emcc  -DFT2_BUILD_LIBRARY $(INCDIRS) -O2 $(SRCS) -o freetype.js -s EXPORTED_FUNCTIONS="['_load_font','_close_font','_get_bitmap','_get_left','_get_top','_get_width','_get_height','_get_pitch','_get_advance']" --memory-init-file 0 

