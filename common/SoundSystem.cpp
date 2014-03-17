#include "SoundSystem.h"
#include "Sound.h"

SoundSystem::SoundSystem()  : sys(0) {
	FMOD_RESULT r;
	r = FMOD_System_Create(&sys);
	FMOD_ERRCHECK(r);

	unsigned int version;
	r = FMOD_System_GetVersion(sys, &version);
	FMOD_ERRCHECK(r);
	if(version < FMOD_VERSION ){
		printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		return;
	}
	r = FMOD_System_Init( sys, 32, FMOD_INIT_NORMAL, NULL );
	FMOD_ERRCHECK(r);

    for(int i=0;i<elementof(sounds);i++) sounds[i] = NULL;
}

Sound *SoundSystem::newSound( const char *path, float vol, bool use_stream_currently_ignored ) {
	FMOD_RESULT r;
	Sound *out = new Sound(this);
	FMOD_SOUND *s;
	r = FMOD_System_CreateSound(sys, path, FMOD_SOFTWARE, 0, &s );
	FMOD_ERRCHECK(r);
	FMOD_Sound_SetMode( s, FMOD_LOOP_OFF );
	out->sound = s;
	out->default_volume = vol;
    out->id = id_gen;
    id_gen++;
    append(out);
	return out;
}

Sound *SoundSystem::newSound( const char *path ){
	return newSound( path, 1.0, false );
}

void SoundSystem::append( Sound *s ) {
    for(int i=0;i<elementof(sounds);i++) {
        if( sounds[i] == NULL ) {
            sounds[i] = s;
            return;
        }
    }
    assertmsg(false, "sound full");
}

Sound *SoundSystem::getById( int id ) {
    for(int i=0;i<elementof(sounds);i++) {
        if( sounds[i] && sounds[i]->id == id ) {
            return sounds[i];
        }
    }
    return NULL;
}

