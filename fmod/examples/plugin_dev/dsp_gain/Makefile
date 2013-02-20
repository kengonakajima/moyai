ARCH = -arch i386 -arch ppc -arch x86_64

release: main.c
	gcc -O3 ${ARCH} -dynamiclib -o example.dylib main.c

debug: main.c
	gcc -O0 ${ARCH} -g -dynamiclib -o example.dylib main.c

clean:
	rm -f example.dylib
