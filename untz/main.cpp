#include <stdio.h>
#include <pthread.h>

#include "../cumino.h"
#include "UntzSound.h"

uint64_t getThreadId() {
    uint64_t tid;
    pthread_threadid_np(NULL,&tid);
    return tid;
}

// [ch1:length][ch2:length]
UInt32 stream_callback(float* buffers, UInt32 numChannels, UInt32 length, void* userdata) {
    static int totl=0;
    static double first_at=now();
    totl+=length;
    fprintf(stderr,"[scb:%lld] cb %f len:%d tot:%d per_sec:%f\n", getThreadId(), now(), length, totl, (double)totl/(now()-first_at) );
    static int counter = 0;
    for(int i=0;i<length;i++) {
        buffers[i] = cos( (float)(counter+i) / 20.0f ) * cos( (float)(counter+1000) / 40000.0f );
    }
    for(int i=0;i<length;i++) {
        buffers[length+i] = cos( (float)(counter+i) / 20.0f ) * cos( (float)(counter+1000) / 40000.0f );        
    }
    counter+=length;
    return length;
}
void showVol(float v) {
    v = fabs(v);
    int maxn = 30;
    int l = (int)(v * maxn);
    char s[30+1];
    for(int i=0;i<maxn;i++) {
        if(i<=l) {
            s[i]='*';
        } else {
            s[i]='-';
        }
    }
    s[maxn]='\0';
    fprintf(stderr, "%s",s);
}
void output_callback( UInt32 numChannels, float *interleavedSamples, UInt32 numSamples ) {
    fprintf(stderr,"[outcb:%lld] output_callback nc:%d ns:%d dat:%f\n",
            getThreadId(),
            numChannels, numSamples, interleavedSamples[0] );
    showVol(interleavedSamples[0]);
}

int main(int argc, char **argv ) {

    UNTZ::System::initialize( 44100, 8192, 0 );

    UNTZ::System::setOutputCallback(output_callback);
    
    // sound from file
    //    UNTZ::Sound *snd = UNTZ::Sound::create( "../assets/blobloblll.wav", true );
    UNTZ::Sound *snd = UNTZ::Sound::create( "../assets/gymno1short.wav", true );    
    printf("sound created:%p",snd);
    snd->play();

    // sound from callback
    UNTZ::Sound *cbsnd = UNTZ::Sound::create(44100,2,stream_callback,NULL);
    cbsnd->play();


    while(true) {
        usleep(10*1000);
        fprintf(stderr,"[main:%lld] ", getThreadId() );
    }
    return 0;
}
