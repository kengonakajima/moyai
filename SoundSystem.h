#pragma once

#include "cumino.h"
#include "fmod/api/inc/fmod.h"
#include "fmod/api/inc/fmod_errors.h"

class Sound;

inline void FMOD_ERRCHECK(FMOD_RESULT result){
	if (result != FMOD_OK){
		assertmsg( false, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result) );
	}
}

class RemoteHead;
class SoundSystem {
public:
    int id_gen;
	FMOD_SYSTEM *sys;
    Sound *sounds[1024];
    RemoteHead *remote_head;
    
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

    void setRemoteHead(RemoteHead*rh) { remote_head = rh; };
    void notifySoundPlay( int id, float vol );
    void notifySoundStop( int id );
};
