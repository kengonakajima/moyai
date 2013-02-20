/*===============================================================================================
 Record example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2011.

 This example shows how to record a sound, then write it to a wav file.
 It then shows how to play a sound while it is being recorded to.  Because it is recording, the
 sound playback has to be delayed a little bit so that the playback doesn't play part of the
 buffer that is still being written to.
===============================================================================================*/
#include "../../api/inc/fmod.h"
#include "../../api/inc/fmod_errors.h"
#include "../common/wincompat.h"

#include <string.h>


#define SWAPENDIAN_WORD(_x)  ((((unsigned short)(_x) & 0xFF00) >>  8) | \
                              (((unsigned short)(_x) & 0x00FF) <<  8))

#define SWAPENDIAN_DWORD(_x) ((((unsigned int)(_x) & 0x000000FF) << 24) | \
                              (((unsigned int)(_x) & 0x0000FF00) <<  8) | \
                              (((unsigned int)(_x) & 0x00FF0000) >>  8) | \
                              (((unsigned int)(_x) & 0xFF000000) >> 24))

#define SWAPENDIAN_FLOAT(_x) { unsigned int *__tmp = (unsigned int *)&(_x); *__tmp = SWAPENDIAN_DWORD(*__tmp); }

#define __PACKED __attribute__((packed)) /* gcc packed */


void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}


/*
[
	[DESCRIPTION]
	Writes out the contents of a record buffer to a file.

	[PARAMETERS]
 
	[RETURN_VALUE]
	void

	[REMARKS]
]
*/
void SaveToWav(FMOD_SOUND *sound)
{
    FILE             *fp;
    int               channels, bits;
    float             rate;
    void             *ptr1, *ptr2;
    unsigned int      lenbytes, len1, len2;
    FMOD_SOUND_FORMAT format;
    int               count = 0;

    if (!sound)
    {
        return;
    }

    FMOD_Sound_GetFormat  (sound, 0, &format, &channels, &bits);
    FMOD_Sound_GetDefaults(sound, &rate, 0, 0, 0);
    FMOD_Sound_GetLength  (sound, &lenbytes, FMOD_TIMEUNIT_PCMBYTES);

    {
        /*
            WAV Structures
        */
        typedef struct
        {
	        signed char id[4];
	        int 		size;
        } RiffChunk;
    
        struct
        {
            RiffChunk       chunk           __PACKED;
            unsigned short	wFormatTag      __PACKED;    /* format type  */
            unsigned short	nChannels       __PACKED;    /* number of channels (i.e. mono, stereo...)  */
            unsigned int	nSamplesPerSec  __PACKED;    /* sample rate  */
            unsigned int	nAvgBytesPerSec __PACKED;    /* for buffer estimation  */
            unsigned short	nBlockAlign     __PACKED;    /* block size of data  */
            unsigned short	wBitsPerSample  __PACKED;    /* number of bits per sample of mono data */
        } __PACKED FmtChunk  = { {{'f','m','t',' '}, sizeof(FmtChunk) - sizeof(RiffChunk) }, 1, channels, (int)rate, (int)rate * channels * bits / 8, 1 * channels * bits / 8, bits };

        if (format == FMOD_SOUND_FORMAT_PCMFLOAT)
        {
            FmtChunk.wFormatTag = 3;
        }

        struct
        {
            RiffChunk   chunk;
        } DataChunk = { {{'d','a','t','a'}, lenbytes } };

        struct
        {
            RiffChunk   chunk;
	        signed char rifftype[4];
        } WavHeader = { {{'R','I','F','F'}, sizeof(FmtChunk) + sizeof(RiffChunk) + lenbytes }, {'W','A','V','E'} };

        #ifdef __BIG_ENDIAN__
        /*
            Do some endian swapping
        */
        FmtChunk.chunk.size      = SWAPENDIAN_DWORD(FmtChunk.chunk.size);
        FmtChunk.wFormatTag      = SWAPENDIAN_WORD(FmtChunk.wFormatTag);
        FmtChunk.nChannels       = SWAPENDIAN_WORD(FmtChunk.nChannels);
        FmtChunk.nSamplesPerSec  = SWAPENDIAN_DWORD(FmtChunk.nSamplesPerSec);
        FmtChunk.nAvgBytesPerSec = SWAPENDIAN_DWORD(FmtChunk.nAvgBytesPerSec);
        FmtChunk.nBlockAlign     = SWAPENDIAN_WORD(FmtChunk.nBlockAlign);
        FmtChunk.wBitsPerSample  = SWAPENDIAN_WORD(FmtChunk.wBitsPerSample);
        DataChunk.chunk.size     = SWAPENDIAN_DWORD(DataChunk.chunk.size);
        WavHeader.chunk.size     = SWAPENDIAN_DWORD(WavHeader.chunk.size);
        #endif

        fp = fopen("record.wav", "wb");
       
        /*
            Write out the WAV header.
        */
        fwrite(&WavHeader, sizeof(WavHeader), 1, fp);
        fwrite(&FmtChunk, sizeof(FmtChunk), 1, fp);
        fwrite(&DataChunk, sizeof(DataChunk), 1, fp);

        /*
            Lock the sound to get access to the raw data.
        */
        FMOD_Sound_Lock(sound, 0, lenbytes, &ptr1, &ptr2, &len1, &len2);

        #ifdef __BIG_ENDIAN__
        /*
            Write it to disk.
        */
        if (format == FMOD_SOUND_FORMAT_PCM16)
        {
            signed short *wptr = (signed short *)ptr1;

            for (count = 0; count < len1 >> 1; count++)
            {
                wptr[count] = SWAPENDIAN_WORD(wptr[count]);
            }
        }
        else if (format == FMOD_SOUND_FORMAT_PCMFLOAT)
        {
            float *fptr = (float *)ptr1;
    
            for (count = 0; count < len1 >> 2; count++)
            {
                SWAPENDIAN_FLOAT(fptr[count]);
            }
        }
        #endif

        fwrite(ptr1, len1, 1, fp);

        /*
            Unlock the sound to allow FMOD to use it again.
        */
        FMOD_Sound_Unlock(sound, ptr1, ptr2, len1, len2);

        fclose(fp);
    }
}


int main(int argc, char *argv[])
{
    FMOD_SYSTEM           *system  = 0;
    FMOD_SOUND            *sound   = 0;
    FMOD_CHANNEL          *channel = 0;
    FMOD_RESULT            result;
    FMOD_CREATESOUNDEXINFO exinfo;
    int                    key, driver, recorddriver, numdrivers, count;
    unsigned int           version;    
    int                    iscoreaudio = 0;

    /*
        Global Settings
    */
    result = FMOD_System_Create(&system);
    ERRCHECK(result);

    result = FMOD_System_GetVersion(system, &version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return 0;
    }

    /*
        Enumerate playback devices
    */

    result = FMOD_System_GetNumDrivers(system, &numdrivers);
    ERRCHECK(result);

    printf("---------------------------------------------------------\n");    
    printf("Choose a PLAYBACK driver\n");
    printf("---------------------------------------------------------\n");    
    for (count=0; count < numdrivers; count++)
    {
        char name[256];

        result = FMOD_System_GetDriverInfo(system, count, name, 256, 0);
        ERRCHECK(result);

        printf("%d : %s\n", count + 1, name);
    }
    printf("---------------------------------------------------------\n");
    printf("Press a corresponding number or ESC to quit\n");

    do
    {
        key = getch();
        if (key == 27)
        {
            return 0;
        }
        driver = key - '1';
    } while (driver < 0 || driver >= numdrivers);

    result = FMOD_System_SetDriver(system, driver);
    ERRCHECK(result);

    /*
        Enumerate record devices
    */

    result = FMOD_System_GetRecordNumDrivers(system, &numdrivers);
    ERRCHECK(result);

    printf("---------------------------------------------------------\n");    
    printf("Choose a RECORD driver\n");
    printf("---------------------------------------------------------\n");    
    for (count=0; count < numdrivers; count++)
    {
        char name[256];

        result = FMOD_System_GetRecordDriverInfo(system, count, name, 256, 0);
        ERRCHECK(result);

        printf("%d : %s\n", count + 1, name);
    }
    printf("---------------------------------------------------------\n");
    printf("Press a corresponding number or ESC to quit\n");

    recorddriver = 0;
    do
    {
        key = getch();
        if (key == 27)
        {
            return 0;
        }
        recorddriver = key - '1';
    } while (recorddriver < 0 || recorddriver >= numdrivers);

    printf("\n");
  
    result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, NULL);
    ERRCHECK(result);

    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));

    exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.numchannels      = 2;
    exinfo.defaultfrequency = 44100;
    if (iscoreaudio)
    {
        exinfo.format   = FMOD_SOUND_FORMAT_PCMFLOAT;
        exinfo.length   = exinfo.defaultfrequency * sizeof(float) * exinfo.numchannels * 5;    
    } 
    else
    {
        exinfo.format   = FMOD_SOUND_FORMAT_PCM16;
        exinfo.length   = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * 5;
    }

    result = FMOD_System_CreateSound(system, 0, FMOD_2D | FMOD_SOFTWARE | FMOD_OPENUSER, &exinfo, &sound);
    ERRCHECK(result);

    printf("===================================================================\n");
    printf("Recording example.  Copyright (c) Firelight Technologies 2004-2011.\n");
    printf("===================================================================\n");
    printf("\n");
    printf("Press 'r' to record a 5 second segment of audio and write it to a wav file.\n");
    printf("Press 'p' to play the 5 second segment of audio.\n");
    printf("Press 'l' to turn looping on/off.\n");
    printf("Press 's' to stop recording and playback.\n");
    printf("Press 'w' to save the 5 second segment to a wav file.\n");
    printf("Press 'Esc' to quit\n");
    printf("\n");

    /*
        Main loop.
    */
    do
    {
        static FMOD_CHANNEL *channel = 0;
        static int   looping   = 0;
        int          recording = 0;
        int          playing   = 0;
        unsigned int recordpos = 0;
        unsigned int playpos   = 0;
        unsigned int length;

        if (kbhit())
        {
            key = getch();

            switch (key)
            {
                case 'r' :
                case 'R' :
                {
                    result = FMOD_System_RecordStart(system, recorddriver, sound, looping);
                    ERRCHECK(result);
                    break;
                }
                case 'p' :
                case 'P' :
                {
                    if (looping)
                    {
                        FMOD_Sound_SetMode(sound, FMOD_LOOP_NORMAL);
                    }
                    else
                    {
                        FMOD_Sound_SetMode(sound, FMOD_LOOP_OFF);
                    }
                    ERRCHECK(result);

                    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_REUSE, sound, 0, &channel);
                    ERRCHECK(result);
                    break;
                }
                case 'l' :
                case 'L' :
                {
                    looping = !looping;
                    break;
                }
                case 's' :
                case 'S' :
                {
                    result = FMOD_System_RecordStop(system, recorddriver);
                    if (channel)
                    {
                        FMOD_Channel_Stop(channel);
                        channel = 0;
                    }
                    break;
                }
                case 'w' :
                case 'W' :
                {
                    printf("Writing to record.wav ...                                                     \r");

                    SaveToWav(sound);
                    Sleep(500);
                    
                    break;
                }
            }
        }

        FMOD_Sound_GetLength(sound, &length, FMOD_TIMEUNIT_PCM);
        ERRCHECK(result);

        FMOD_System_IsRecording(system, recorddriver, &recording);
        ERRCHECK(result);

        FMOD_System_GetRecordPosition(system, recorddriver, &recordpos);
        ERRCHECK(result);

        if (channel)
        {
            FMOD_Channel_IsPlaying(channel, &playing);
            ERRCHECK(result);

            FMOD_Channel_GetPosition(channel, &playpos, FMOD_TIMEUNIT_PCM);
            ERRCHECK(result);
        }

        printf("\rState: %-19s. Record pos = %6d : Play pos = %6d : Loop %-3s", recording ? playing ? "Recording / playing" : "Recording" : playing ? "Playing" : "Idle", recordpos, playpos, looping ? "On" : "Off");
	fflush(stdout);

        FMOD_System_Update(system);

        Sleep(10);

    } while (key != 27);

    printf("\n");

    /*
        Shut down
    */
    result = FMOD_Sound_Release(sound);
    ERRCHECK(result);

    result = FMOD_System_Release(system);
    ERRCHECK(result);

    return 0;
}


