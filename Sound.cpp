#include "common.h"
#include "client.h"
#include "Sound.h"
#include "SoundSystem.h"
#include "Remote.h"

#ifdef USE_OPENAL
#include "ALSound.h" // Thin wrapper of OpenAL
#endif

Sound::Sound( SoundSystem *s) : parent(s), sound(0), ch(0), default_volume(1), external_id(0), last_samples(0), last_samples_num(0), last_play_volume(0) {
    last_load_file_path[0] = '\0';
}
void Sound::play(){
	play(default_volume);
}

void Sound::play(float vol){
	if(vol==0)return;
#ifdef USE_FMOD    
	FMOD_RESULT r;
	if( !this->ch ){
		r = FMOD_System_PlaySound( parent->sys, FMOD_CHANNEL_FREE, sound, 0, & this->ch );
	} else {
		r = FMOD_System_PlaySound( parent->sys, FMOD_CHANNEL_REUSE, sound, 0, & this->ch );            
	}
	FMOD_ERRCHECK(r);
	FMOD_Channel_SetVolume(ch, default_volume * vol );
#endif
#ifdef USE_UNTZ
    sound->play();
    sound->setVolume(vol);
#endif
#ifdef USE_OPENAL
    sound->play();
    sound->setVolume(vol);
#endif    
    last_play_volume = default_volume * vol;    
    if(parent->remote_head) parent->remote_head->notifySoundPlay( this, last_play_volume );
}

void Sound::playDistance(float mindist, float maxdist, float dist, float relvol) {
	if(dist<mindist) {
		play(1 * relvol);
	} else if( dist > maxdist ) {
		return;
	} else {
		float width = maxdist - mindist;
		float r = 1 - (dist - mindist) / width;
		play(r * relvol);
	}
}


void Sound::stop() {
    //    print("Sound::stop! %p debugid:%d ch:%p",this, this->debug_id ,this->ch);
#ifdef USE_FMOD    
	FMOD_Channel_Stop( this->ch );
    this->ch = NULL;    
#endif
#ifdef USE_UNTZ
    sound->stop();
#endif
#ifdef USE_OPENAL
    sound->stop();
#endif        
    if(this->parent->remote_head) this->parent->remote_head->notifySoundStop(this);
}

bool Sound::isPlaying() {
#ifdef USE_FMOD    
	if(!this->ch)return false;
	FMOD_BOOL val;
	FMOD_RESULT r;
	r = FMOD_Channel_IsPlaying( this->ch, &val );
	if( r != FMOD_OK ) return false;
	return val;
#elif defined(USE_UNTZ)
    if(!sound)return false;
    return sound->isPlaying();
#elif defined(USE_OPENAL)
    if(!sound)return false;    
    return sound->isPlaying();
#else
    return false; // TODO: implement linux virtual sound
#endif

}
void Sound::setVolume( float v ) {
#ifdef USE_FMOD    
	FMOD_Channel_SetVolume(this->ch, v );
#endif
#ifdef USE_UNTZ
    sound->setVolume(v);
#endif
#ifdef USE_OPENAL
    sound->setVolume(v);
#endif    
}
float Sound::getVolume() {
#ifdef USE_FMOD    
    float v;
    FMOD_Channel_GetVolume(this->ch,&v);
    return v;
#elif defined(USE_UNTZ)
    return sound->getVolume();
#elif defined(USE_OPENAL)
    return sound->getVolume();
#else
    return 1; // TODO: implement linux virtual sound
#endif
    
}
void Sound::setLoop( bool flag ) {
#ifdef USE_FMOD    
	if( flag ) {
		FMOD_Sound_SetMode( sound, FMOD_LOOP_NORMAL );            
	} else {
		FMOD_Sound_SetMode( sound, FMOD_LOOP_OFF );    
	}
#endif
#ifdef USE_UNTZ
    sound->setLooping(flag);
#endif
#ifdef USE_OPENAL
    sound->setLooping(flag);
#endif    
}
void Sound::updateLastSamples( float *samples, int samples_num ) {
    if( last_samples ) FREE(last_samples);

    size_t sz = samples_num*sizeof(float);
    last_samples = (float*)MALLOC(sz);
    memcpy( last_samples, samples, sz );
    last_samples_num = samples_num;
}

// returns -1 if not playing
float Sound::getTimePositionSec() {
#ifdef USE_FMOD    
    if(!this->ch) return -1;
    unsigned int pos_ms;
    FMOD_RESULT r = FMOD_Channel_GetPosition( this->ch, &pos_ms, FMOD_TIMEUNIT_MS );
    if( r != FMOD_OK ) {
        print("FMOD_Channel_GetPosition: failed. ret:%d", r );
        return -1;
    }
    return (float)(pos_ms) / 1000.0f;
#elif defined(USE_UNTZ)
    return (float) sound->getPosition();
#elif defined(USE_OPENAL)
    return (float) sound->getPosition();
#else
    return 0; // TODO: implement linux virtual sound
#endif    
}
void Sound::setTimePositionSec( float sec ) {
#ifdef USE_FMOD    
    if(!this->ch) return;
    unsigned int pos_ms = (unsigned int)(sec * 1000);
    FMOD_RESULT r = FMOD_Channel_SetPosition( this->ch, pos_ms, FMOD_TIMEUNIT_MS );
    if( r != FMOD_OK ) {
        print("FMOD_Channel_SetPosition: failed. ret:%d", r );
    }
#endif
#ifdef USE_UNTZ
    sound->setPosition(sec);
#endif
#ifdef USE_OPENAL
    sound->setPosition(sec);
#endif    
}

