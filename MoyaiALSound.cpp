#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "client.h"
#include "MoyaiALSound.h"

#ifdef USE_MOYAIAL
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
//#include "libswscale/swscale.h"
};
#endif

//static inline int16_t limit_float_conv_int16(float inValue) {
//    return (int16_t)((1-2*signbit(inValue)) * atanf(fabs(inValue) * 2.0f) * ((2.0f / 3.14f) * 32767));
//}

static const int FREQ = 48000;

static MoyaiALSound *g_moyaial_sounds[1024];
static int g_moyaial_sounds_used=0;

static void appendSound(MoyaiALSound *snd) {
    assert(g_moyaial_sounds_used<=elementof(g_moyaial_sounds));
    g_moyaial_sounds[g_moyaial_sounds_used]=snd;
    g_moyaial_sounds_used++;
    fprintf(stderr,"appendSound: used:%d\n",g_moyaial_sounds_used);
}


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
    out->resample();    
    appendSound(out);
    return out;
}
void MoyaiALSound::resample() {
    if(sampleRate==FREQ) return;
    
#if USE_MOYAIAL
    int layout;
    if(numChannels==1) {
        layout = AV_CH_LAYOUT_MONO;
    } else {
        layout = AV_CH_LAYOUT_STEREO;
    }
    // https://github.com/illuusio/ffmpeg-example/blob/master/example2.c
    SwrContext *swr = swr_alloc_set_opts( NULL,
                                          // out
                                          layout,
                                          AV_SAMPLE_FMT_FLT,
                                          FREQ,
                                          // in
                                          layout,
                                          AV_SAMPLE_FMT_FLT,
                                          sampleRate,
                                          0,
                                         NULL);
    int ret = swr_init(swr);
    assert(ret>=0);
    const uint8_t *input = (uint8_t*)samples;
    float sec = (float)numFrames / (float)sampleRate;
    int needNumFrames = (int)( (float)sec * FREQ);
    fprintf(stderr," sec:%f needNumFrames:%d\n", sec,needNumFrames);
    uint8_t *output = (uint8_t*)MALLOC( needNumFrames * numChannels * sizeof(float));
    ret = swr_convert(swr, &output, needNumFrames, &input, numFrames );
    fprintf(stderr, "swr_convert: ret:%d\n",ret);
    if(ret<0) {
        fprintf(stderr, "swr_convert failed: %d\n",ret);
        FREE(output);
        return;
    }
    float *to_free = samples;
    samples = (float*)output;
    FREE(to_free);
    numFrames = ret;
    sampleRate = FREQ;
#endif    
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
    out->resample();
    appendSound(out);    
    return out;
}

static const int ABUFNUM = 4, ABUFLEN = 512; // 256 not working
static int16_t g_pcmdata[ABUFNUM][ABUFLEN*2]; // stereo
static ALuint g_alsource;
static ALuint g_albuffer[ABUFNUM];
static double t=0,dt=0;

static void (*g_on_before_mix)(int16_t *samples, int numFrames, int numChannels, int freq ) = nullptr;
static void (*g_on_mix_done)(int16_t *samples, int numFrames, int numChannels, int freq ) = nullptr;
void setMoyaiALOnMixDone( void (*cb)( int16_t *samples, int numFrames, int numChannels, int freq ) ) {
    g_on_mix_done = cb;
}
void setMoyaiALOnBeforeMix( void (*cb)( int16_t *samples, int numFrames, int numChannels, int freq ) ) {
    g_on_before_mix = cb;
}
static void mixFill(int bufind) {
#if 0 // debug sound filler
    for(int i=0;i<ABUFLEN;i++) {
        t+=0.01+dt;
        dt+=0.0000002;
        g_pcmdata[bufind][i*2+0] = sin(t)*1000;
        g_pcmdata[bufind][i*2+1] = random()%200;
    }
#else
    for(int i=0;i<ABUFLEN;i++) {
        g_pcmdata[bufind][i*2+0] = 0;
        g_pcmdata[bufind][i*2+1] = 0;
    }
#endif
    if(g_on_before_mix) g_on_before_mix( g_pcmdata[bufind], ABUFLEN, 2, FREQ );
    
    for(int i=0;i<g_moyaial_sounds_used;i++) {
        MoyaiALSound *snd = g_moyaial_sounds[i];
        assert(snd->sampleRate == FREQ);
        
        if(snd->state == MoyaiALSound::PLAYING) {
#if 0
            float s = snd->samples[snd->posFrame*snd->numChannels];
            fprintf(stderr, "Playing:[%d] ch:%d nf:%d posF:%d(%f) sr:%d v:%f st:%d loop:%d\n",
                    i, snd->numChannels, snd->numFrames, snd->posFrame, s,
                    snd->sampleRate, snd->volume, snd->state, snd->loop );
#endif            
        } else {
            continue;
        }
        for(int i=0;i<ABUFLEN;i++) {
            float left_sample = snd->samples[snd->posFrame*snd->numChannels+0] * snd->volume;
            if(left_sample<-1)left_sample=-1;
            else if(left_sample>1)left_sample=1;
            g_pcmdata[bufind][i*2+0] += (short)(left_sample*32767);
            float right_sample = left_sample;             
            if(snd->numChannels==2) {
                right_sample = snd->samples[snd->posFrame*snd->numChannels+1] * snd->volume;
                if(right_sample<-1)right_sample=-1;
                else if(right_sample>1)right_sample=1;
            }
            g_pcmdata[bufind][i*2+1] = (short)(right_sample*32767);
            
            snd->posFrame++;
            if(snd->posFrame > snd->numFrames) {
                snd->posFrame = 0;
                if(!snd->loop) {
                    snd->state = MoyaiALSound::STOPPED;
                    break;
                }
            }
        }
    }
    if(g_on_mix_done) {
        g_on_mix_done(g_pcmdata[bufind], ABUFLEN, 2, FREQ );
    }
}

static void *moyaiALThreadFunc(void *arg) {
    alGenBuffers(ABUFNUM,g_albuffer);
    alGenSources(1,&g_alsource);
    fprintf(stderr,"moyaiALsrc:%d\n",g_alsource);
    for(int j=0;j<ABUFNUM;j++) {
        for(int i=0;i<ABUFLEN;i++) {
            t+=0.01+dt;
            dt+=0.0000002;
            g_pcmdata[j][i*2+0] = sin(t)*1000;
            g_pcmdata[j][i*2+1] = random()%200;
        }
    }
    for(int i=0;i<ABUFNUM;i++) {
        alBufferData(g_albuffer[i], AL_FORMAT_STEREO16, g_pcmdata[i], ABUFLEN*sizeof(int16_t)*2, FREQ);
        alSourceQueueBuffers(g_alsource,1,&g_albuffer[i]);
        fprintf(stderr,"alsourcequeuebuffers: %d\n",alGetError());
    }
    alSourcePlay(g_alsource);
	fprintf(stderr,"alsourceplay done:%d\n",alGetError());

    static int play_head=0;
    while(true) {
        ALint proced;
        alGetSourcei(g_alsource,AL_BUFFERS_PROCESSED,&proced);
        //        fprintf(stderr,"proced:%d\n",proced);
        if(proced>0) {
            for(int proci=0;proci<proced;proci++) {
                int bufind =  play_head % ABUFNUM;
                alSourceUnqueueBuffers(g_alsource,1,&g_albuffer[bufind]);
                mixFill(bufind);
                alBufferData(g_albuffer[bufind], AL_FORMAT_STEREO16, g_pcmdata[bufind], ABUFLEN*sizeof(int16_t)*2,FREQ);
                alSourceQueueBuffers(g_alsource,1,&g_albuffer[bufind]);
                play_head++;
                
            }
        }
        usleep(1*1000);
    }
}
void startMoyaiAL() {
    static bool init=false;
    if(init) return;
    init=true;
    
#ifdef WIN32    
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)moyaiALThreadFunc, url, 0, NULL);
#else
    pthread_t tid;
    int err = pthread_create(&tid,NULL,moyaiALThreadFunc,NULL);
    if(err) {
        print("moyaiALThreadFunc: pthread_create failed:%d",err);
        return;
    }
#endif    
}
