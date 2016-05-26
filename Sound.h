#pragma once

#ifdef USE_FMOD
#include "fmod/api/inc/fmod.h"
#endif 
#ifdef USE_UNTZ
#include "untz/include/UntzSound.h"
#endif
#ifdef USE_OPENAL
class ALSound;
#endif

class SoundSystem;

class Sound {
public:
    int id;
	SoundSystem *parent;
#ifdef USE_FMOD    
	FMOD_SOUND *sound;
	FMOD_CHANNEL *ch;
#endif
#ifdef USE_UNTZ
    UNTZ::Sound *sound;
    void *ch;
#endif    
#ifdef USE_OPENAL
    ALSound *sound;
    void *ch;
#endif    
	float default_volume;
    int external_id; // for app use

    char last_load_file_path[256]; // for headless
    float *last_samples;  // for headless
    int last_samples_num; // for headless, number of floats
    float last_play_volume; // for headless
    
	Sound( SoundSystem *s);
	void setLoop( bool flag );
	void play();
	void play(float vol);
	void playDistance(float mindist, float maxdist, float dist, float relvol );
	void stop();
	bool isPlaying();
    void setVolume(float v);
    float getVolume();
    float getTimePositionSec();
    void setTimePositionSec(float sec);
    void updateLastSamples( float *samples, int samples_num );
};
