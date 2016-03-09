#include "client.h"


#include "SoundSystem.h"
#include "Sound.h"

SoundSystem::SoundSystem()  : id_gen(1), sys(0), remote_head(0) {
	FMOD_RESULT r;
	r = FMOD_System_Create(&sys);
	FMOD_ERRCHECK(r);

	unsigned int version;
	r = FMOD_System_GetVersion(sys, &version);
	FMOD_ERRCHECK(r);
	if(version < FMOD_VERSION ){
		print("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		return;
	}
	r = FMOD_System_Init( sys, 32, FMOD_INIT_NORMAL, NULL );
	FMOD_ERRCHECK(r);

    for(int i=0;i<elementof(sounds);i++) sounds[i] = NULL;
}

Sound *SoundSystem::newSound( const char *path, float vol, bool use_stream_currently_ignored ) {
    const char *cpath = platformCStringPath(path);
    FMOD_RESULT r;
	Sound *out = new Sound(this);
	FMOD_SOUND *s;
	r = FMOD_System_CreateSound(sys, cpath, FMOD_SOFTWARE, 0, &s );
    if( r != FMOD_OK ) {
        print("newSound: can't create sound:'%s'", cpath );
    }
	FMOD_ERRCHECK(r);
	FMOD_Sound_SetMode( s, FMOD_LOOP_OFF );
	out->sound = s;
	out->default_volume = vol;
    out->id = id_gen;
    strncpy( out->last_load_file_path, path, sizeof(out->last_load_file_path) );
    id_gen++;
    append(out);
	return out;
}

Sound *SoundSystem::newSound( const char *path ){
	return newSound( path, 1.0, false );
}
Sound *SoundSystem::newSoundFromMemory( float *samples, int samples_num ) {
    FMOD_RESULT r;
    Sound *out = new Sound(this);
    FMOD_SOUND *s;
    FMOD_CREATESOUNDEXINFO exinfo;

    memset( &exinfo, 0, sizeof(exinfo) );
    exinfo.cbsize = sizeof(exinfo);
    exinfo.length = sizeof(float) * samples_num;
    exinfo.defaultfrequency = 44100;
    exinfo.numchannels = 1;
    exinfo.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    
    r = FMOD_System_CreateSound( sys, (const char*) samples, FMOD_SOFTWARE | FMOD_OPENMEMORY | FMOD_OPENRAW, &exinfo, &s );
    FMOD_ERRCHECK(r);
    FMOD_Sound_SetMode( s, FMOD_LOOP_OFF );
    out->sound = s;
    out->default_volume = 1;
    out->id = id_gen;
    out->updateLastSamples(samples, samples_num);
    id_gen++;
    append(out);
    return out;
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

