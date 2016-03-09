#pragma once

#include "fmod/api/inc/fmod.h"

class SoundSystem;

class Sound {
public:
    int id;
	FMOD_SOUND *sound;
	SoundSystem *parent;
	FMOD_CHANNEL *ch;

	float default_volume;
    int external_id; // for app use

    char last_load_file_path[256]; // for headless
    float *last_samples;  // for headless
    int last_samples_num; // for headless, number of floats
    
	Sound( SoundSystem *s);
	void setLoop( bool flag );
	void play();
	void play(float vol);
	void playDistance(float mindist, float maxdist, float dist, float relvol );
	void stop();
    void pause( bool to_pause );
	bool isPlaying();
    void setVolume(float v);
    float getVolume();
    float getTimePositionSec();
    void setTimePositionSec(float sec);
    void updateLastSamples( float *samples, int samples_num );
};
