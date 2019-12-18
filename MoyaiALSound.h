#ifndef _ALSOUND_H_
#define _ALSOUND_H_

#ifdef WIN32
#include "al.h"
#include "alc.h"
#else
#include "OpenAL/al.h"
#include "OpenAL/alc.h"
#endif

#include "AL/alut.h"


class MoyaiALSound {
public:
    typedef enum {
                  STOPPED=0,
                  PLAYING=1,
    } STATE;
    float *samples;
    int posFrame; // 1 for samples[2] when numchannel==2
    int numFrames; // total frame num
    int numChannels;
    int sampleRate;
    float volume;
    STATE state;
    bool loop;
    MoyaiALSound() : samples(nullptr), posFrame(0), numFrames(0), numChannels(0), sampleRate(0), volume(1), state(STOPPED), loop(false) {}

    bool isPlaying() {
        return state == PLAYING;
    }
    float getVolume() {
        return volume;
    }
    void play() {
        posFrame=0;
        state=PLAYING;
    }
    void stop() {
        state=STOPPED;
    }
    void setVolume( float v) {
        volume = v;
    }
    void setPosition(float sec) {
        posFrame = (sec * sampleRate);
    }
    float getPosition() {
        float sec = (float)(posFrame) / (float)sampleRate;
        return sec;
    }
    void setLooping(bool flag) {
        loop=flag;
    }
    static MoyaiALSound *create( const char *cpath ) ;
    static MoyaiALSound *create( int sampleRate, int numChannels, int numFrames, bool loop, float *samples );    
};

void startMoyaiAL();

#endif
