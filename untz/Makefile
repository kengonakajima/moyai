# Minimum build UNTZ on osx only

# Requires:
# brew install libogg
# brew install libvorbis

LIBOBJS=AudioMixer.o BufferedAudioSource.o BufferedAudioSourceThread.o UntzSound.o OggAudioSource.o UserAudioSource.o MemoryAudioSource.o ExtAudioFileAudioSource.o UntzSystem.o 
OBJS=$(LIBOBJS) main.o
CFLAGS=-I./include -I./src -I/usr/local/include -Isrc/native/ios -DMACOSX -g

all: libuntz.a synth 

AudioMixer.o : src/AudioMixer.cpp
	g++ -c $(CFLAGS) src/AudioMixer.cpp 
BufferedAudioSource.o : src/BufferedAudioSource.cpp
	g++ -c $(CFLAGS) src/BufferedAudioSource.cpp
BufferedAudioSourceThread.o : src/BufferedAudioSourceThread.cpp
	g++ -c $(CFLAGS) src/BufferedAudioSourceThread.cpp
UntzSound.o : src/UntzSound.cpp
	g++ -c $(CFLAGS) src/UntzSound.cpp
OggAudioSource.o : src/OggAudioSource.cpp
	g++ -c $(CFLAGS) src/OggAudioSource.cpp
UserAudioSource.o : src/UserAudioSource.cpp
	g++ -c $(CFLAGS) src/UserAudioSource.cpp
MemoryAudioSource.o : src/MemoryAudioSource.cpp
	g++ -c $(CFLAGS) src/MemoryAudioSource.cpp
ExtAudioFileAudioSource.o : src/native/ios/ExtAudioFileAudioSource.cpp
	g++ -c $(CFLAGS) src/native/ios/ExtAudioFileAudioSource.cpp
UntzSystem.o : src/native/ios/UntzSystem.cpp
	g++ -c $(CFLAGS) src/native/ios/UntzSystem.cpp
main.o : main.cpp
	g++ -c $(CFLAGS) main.cpp

libuntz.a : $(LIBOBJS)
	ar cr libuntz.a $(LIBOBJS)
	ranlib libuntz.a

synth: main.o $(LIBOBJS)
	g++ -o synth main.o libuntz.a -framework Cocoa -framework AudioToolbox /usr/local/lib/libogg.a /usr/local/lib/libvorbis.a /usr/local/lib/libvorbisfile.a

clean:
	rm -f *.o synth libuntz.a

