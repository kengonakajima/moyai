#include "common.h"
#include "Sound.h"
#include "SoundSystem.h"
#include "Remote.h"

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
#endif
#ifdef USE_UNTZ
    return sound->isPlaying();
#endif    
}
void Sound::setVolume( float v ) {
#if USE_FMOD    
	FMOD_Channel_SetVolume(this->ch, v );
#endif
#if USE_UNTZ
    sound->setVolume(v);
#endif
}
float Sound::getVolume() {
#ifdef USE_FMOD    
    float v;
    FMOD_Channel_GetVolume(this->ch,&v);
    return v;
#endif
#ifdef USE_UNTZ
    return sound->getVolume();
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
    if(!this->ch) return -1;
#ifdef USE_FMOD    
    unsigned int pos_ms;
    FMOD_RESULT r = FMOD_Channel_GetPosition( this->ch, &pos_ms, FMOD_TIMEUNIT_MS );
    if( r != FMOD_OK ) {
        print("FMOD_Channel_GetPosition: failed. ret:%d", r );
        return -1;
    }
    return (float)(pos_ms) / 1000.0f;
#endif
#ifdef USE_UNTZ
    return (float) sound->getPosition();
#endif    
}
void Sound::setTimePositionSec( float sec ) {
    if(!this->ch) return;
#ifdef USE_FMOD    
    unsigned int pos_ms = (unsigned int)(sec * 1000);
    FMOD_RESULT r = FMOD_Channel_SetPosition( this->ch, pos_ms, FMOD_TIMEUNIT_MS );
    if( r != FMOD_OK ) {
        print("FMOD_Channel_SetPosition: failed. ret:%d", r );
    }
#endif
#ifdef USE_UNTZ
    sound->setPosition(sec);
#endif   
}

