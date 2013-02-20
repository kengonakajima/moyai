ARCH = -arch i386 -arch x86_64

release: example.cpp
	g++ -O2 ${ARCH} -o example example.cpp -framework CoreFoundation libfsbankex.a ../../api/lib/libfmodex.dylib
	install_name_tool -change ./libfmodex.dylib ../../api/lib/libfmodex.dylib example

clean:
	rm -f example
