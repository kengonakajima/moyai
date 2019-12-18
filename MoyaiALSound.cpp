#include <assert.h>
#include <stdio.h>
#include "common.h"
#include "MoyaiALSound.h"


//static inline int16_t limit_float_conv_int16(float inValue) {
//    return (int16_t)((1-2*signbit(inValue)) * atanf(fabs(inValue) * 2.0f) * ((2.0f / 3.14f) * 32767));
//}


MoyaiALSound *MoyaiALSound::create( const char *cpath ) {
    ALenum fmt;
    void *data;
    ALsizei bytesz=0;
    ALsizei freq;
    int sampleSize;
    alutLoadWAVFile((ALbyte*)cpath, &fmt, &data, &bytesz, &freq );
    if(bytesz==0) return nullptr;
    
    MoyaiALSound *out = new MoyaiALSound();
    out->sampleRate = freq;
    
    switch(fmt) {
    case AL_FORMAT_MONO8:
        sampleSize=1;
        out->numChannels=1;
        out->numFrames=bytesz;
        break;
    case AL_FORMAT_MONO16:
        sampleSize=2;
        out->numChannels=1;
        out->numFrames=bytesz/2;
        break;
    case AL_FORMAT_STEREO8:
        sampleSize = 1;
        out->numChannels=2;
        out->numFrames=bytesz/2;        
        break;
    case AL_FORMAT_STEREO16:
        sampleSize = 2;
        out->numChannels=2;
        out->numFrames=bytesz/4;
        break;
    }
    
    fprintf(stderr, "loadwavfile: bytesz:%d fmt:%d numch:%d numframe:%d freq:%d path:%s\n",
            bytesz, fmt, out->numChannels, out->numFrames, out->sampleRate, cpath );



    out->samples = (float*)MALLOC(out->numFrames*out->numChannels*sizeof(float));
    
    assert(out->samples);
    for(int i=0;i<out->numFrames;i++) {
        if(sampleSize==1) {
            char *ptr=(char*)data;
            out->samples[i*out->numChannels+0]=((float)ptr[i*out->numChannels+0])/127.0f;
            if(out->numChannels==2) {
                out->samples[i*out->numChannels+1]=((float)ptr[i*out->numChannels+1])/127.0f;
            }
        } else if( sampleSize==2 ) {
            short *ptr=(short*)data;
            out->samples[i*out->numChannels+0]=((float)ptr[i*out->numChannels+0])/32767.0f;
            if(out->numChannels==2) {
                out->samples[i*out->numChannels+1]=((float)ptr[i*out->numChannels+1])/32767.0f;
            }
        }
    }
    return out;
}

MoyaiALSound *MoyaiALSound::create( int sampleRate, int numChannels, int numFrames, bool loop,  float *samples ) {
    MoyaiALSound *out = new MoyaiALSound();
    out->sampleRate = sampleRate;
    out->numChannels = numChannels;
    out->numFrames = numFrames;
    out->loop = loop;

    int bytesize = sizeof(float) * numFrames * numChannels;
    out->samples = (float*)MALLOC(bytesize);
    assert(out);
    for(int i=0;i<numFrames * numChannels;i++) {
        out->samples[i]=samples[i];
    }
    return out;
}
