#pragma once

#include "cumino.h"

#ifdef USE_FMOD
#include "fmod/api/inc/fmod.h"
#include "fmod/api/inc/fmod_errors.h"
inline void FMOD_ERRCHECK(FMOD_RESULT result){
	if (result != FMOD_OK){
		assertmsg( false, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result) );
	}
}
#endif


class Sound;

class RemoteHead;
class SoundSystem {
public:
    int id_gen;
    Sound *sounds[1024];
    RemoteHead *remote_head;
#ifdef USE_FMOD    
	FMOD_SYSTEM *sys;
#endif
#if defined(USE_UNTZ) || defined(USE_OPENAL) || defined(__linux__)
    void *sys; // not used    
#endif    
	SoundSystem();

	Sound * newSE( const char *path ) { return newSE(path,1.0f); }
	Sound * newSE( const char *path, float vol ) { return newSound(path,vol,false); };
	Sound * newBGM( const char *path ) { return newBGM(path,1.0f); }
	Sound * newBGM( const char *path, float vol ) { return newSound(path,vol,true); };

	Sound *newSound( const char *path, float vol, bool use_stream_currently_ignored );
	Sound *newSound( const char *path );
    Sound *newSoundFromMemory( float *sample, int sample_num );

    void append( Sound*s );
    Sound *getById( int id );

    void setRemoteHead(RemoteHead*rh);
    void setVolume(float v);
};
