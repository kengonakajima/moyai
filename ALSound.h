#ifndef _ALSOUND_H_
#define _ALSOUND_H_

#include "OpenAL/al.h"
#include "OpenAL/alc.h"
#include "AL/alut.h"

class ALSoundInfo {
public:
    unsigned int mSampleRate; // ex. 44100
    unsigned int mChannels; // ex. 1 or 2
    unsigned int mTotalFrames; // num of float values. 2 samples for 1 frame when channel=2
    ALSoundInfo() : mSampleRate(0), mChannels(0), mTotalFrames(0) {}
};

static inline int16_t limit_float_conv_int16(float inValue)
{
    return (int16_t)((1-2*signbit(inValue)) * atanf(fabs(inValue) * 2.0f) * ((2.0f / 3.14f) * 32767));
}

class ALSound {
public:
    ALuint source;
    ALuint buffer;
    float volume;
    ALSound() : source(0), buffer(0), volume(1) {}

    bool isPlaying() {
        ALenum state;
        alGetSourcei( source, AL_SOURCE_STATE, &state );
        return (state == AL_PLAYING);
    }
    float getVolume() {
        return volume;
    }
    void play() {
        assert(source!=0);
        alSourcePlay(source);
    }
    void stop() {
        print("stop not implemented");
    }
    void setVolume( float v) {
        volume = v;
        alSourcef(source,AL_GAIN,v);
    }
    void setPosition(float pos) {
        alSourcef(source,AL_SEC_OFFSET,pos);
    }
    float getPosition() {
        float ofs;
        alGetSourcef(source,AL_SEC_OFFSET,&ofs);
        return ofs;
    }
    void setLooping(bool flag_to_loop) {
        alSourcei(source,AL_LOOPING,flag_to_loop);
    }
    static ALSound *create( const char *cpath ) {
        ALSound *out = new ALSound();
        out->buffer = alutCreateBufferFromFile(cpath);
        if(out->buffer==0) {
            fprintf(stderr, "alutCreateBufferFromFile error: %s file:%s\n", alutGetErrorString(alutGetError()), cpath );
            return 0;
        }
        alGenSources(1,&out->source);
        alSourcei(out->source,AL_BUFFER,out->buffer);
        return out;
    }
    static ALSound *create( ALSoundInfo info, float *samples ) {
        ALSound *out = new ALSound();
        alGenBuffers( 1, &out->buffer );
        if(out->buffer==0) {
            fprintf(stderr, "alGenBuffers failed\n");
            return 0;
        }
        assertmsg( info.mChannels == 1, "ALSound::create(): creating a sound from memory supports only monoral data\n");

        ALsizei bytesize = sizeof(int16_t) * info.mTotalFrames * info.mChannels;
        
        int16_t *i16outbuf = (int16_t*)MALLOC(info.mTotalFrames * sizeof(int16_t) );
        for(int i=0;i<info.mTotalFrames;i++) {
            i16outbuf[i] = limit_float_conv_int16( samples[i] ); // from UNTZ code
        }

        print("calling alBufferData. mChannels:%d mTotalFrames:%d mSampleRate:%d", info.mChannels, info.mTotalFrames, info.mSampleRate );
        alBufferData( out->buffer, AL_FORMAT_MONO16, i16outbuf, bytesize, info.mSampleRate );
        alGenSources(1, &out->source);
        alSourcei(out->source,AL_BUFFER,out->buffer);
        return out;
    }
};
#endif
