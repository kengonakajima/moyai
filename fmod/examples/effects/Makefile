LIB_PATH 			= ../../api/lib
LIB_NAME_RELEASE	= libfmodex.dylib
LIB_NAME_LOGGING	= libfmodexL.dylib
ARCH				= -arch i386 -arch x86_64

release: main.cpp
	g++ -O3 ${ARCH} -o example main.cpp ${LIB_PATH}/${LIB_NAME_RELEASE}
	install_name_tool -change ./${LIB_NAME_RELEASE} ${LIB_PATH}/${LIB_NAME_RELEASE} example

debug: main.cpp
	g++ -O0 ${ARCH} -g -o example main.cpp ${LIB_PATH}/${LIB_NAME_LOGGING}
	install_name_tool -change ./${LIB_NAME_LOGGING} ${LIB_PATH}/${LIB_NAME_LOGGING} example

release_c: main.c
	gcc -O3 ${ARCH} -o example main.c ${LIB_PATH}/${LIB_NAME_RELEASE}
	install_name_tool -change ./${LIB_NAME_RELEASE} ${LIB_PATH}/${LIB_NAME_RELEASE} example

debug_c: main.c
	gcc -O0 ${ARCH} -g -o example main.c ${LIB_PATH}/${LIB_NAME_LOGGING}
	install_name_tool -change ./${LIB_NAME_LOGGING} ${LIB_PATH}/${LIB_NAME_LOGGING} example

clean:
	rm -f example
