INC_PATH_FMOD       	= ../../../api/inc
LIB_PATH_FMOD 			= ../../../api/lib
LIB_PATH_FMODEVENTNET	= ../../api/lib
LIB_NAME_FMOD			= libfmodex.dylib
LIB_NAME_FMODEVENTNET	= libfmodeventnet.dylib
ARCH					= -arch i386 -arch ppc -arch x86_64

release: main.cpp
	cp ${LIB_PATH_FMOD}/${LIB_NAME_FMOD} .
	cp ${LIB_PATH_FMODEVENTNET}/${LIB_NAME_FMODEVENTNET} .
	g++ -O3 ${ARCH} -o example main.cpp ./${LIB_NAME_FMOD} ./${LIB_NAME_FMODEVENTNET} -I${INC_PATH_FMOD}

debug: main.cpp
	cp ${LIB_PATH_FMOD}/${LIB_NAME_FMOD} .
	cp ${LIB_PATH_FMODEVENTNET}/${LIB_NAME_FMODEVENTNET} .
	g++ -O0 ${ARCH} -g -o example main.cpp ./${LIB_NAME_FMOD} ./${LIB_NAME_FMODEVENTNET} -I${INC_PATH_FMOD}

clean:
	rm -f example
	rm -f ${LIB_NAME_FMOD}
	rm -f ${LIB_NAME_FMODEVENTNET}

