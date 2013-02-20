/*===============================================================================================
 Record to disk example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2011.

 This example shows how to do a streaming record to disk.
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
void WriteWavHeader(FILE *fp, FMOD_SOUND *sound, int length)
{
    int               channels, bits;
    float             rate;
    FMOD_SOUND_FORMAT format;

    if (!sound)
    {
        return;
    }

    fseek(fp, 0, SEEK_SET);

    FMOD_Sound_GetFormat  (sound, 0, &format, &channels, &bits);
    FMOD_Sound_GetDefaults(sound, &rate, 0, 0, 0);

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
        } DataChunk = { {{'d','a','t','a'}, length } };

        struct
        {
            RiffChunk   chunk;
	        signed char rifftype[4];
        } WavHeader = { {{'R','I','F','F'}, sizeof(FmtChunk) + sizeof(RiffChunk) + length }, {'W','A','V','E'} };

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
 
        /*
            Write out the WAV header.
        */
        fwrite(&WavHeader, sizeof(WavHeader), 1, fp);
        fwrite(&FmtChunk, sizeof(FmtChunk), 1, fp);
        fwrite(&DataChunk, sizeof(DataChunk), 1, fp);
    }
}


int main(int argc, char *argv[])
{
    FMOD_SYSTEM          *system  = 0;
    FMOD_SOUND           *sound   = 0;
    FMOD_RESULT            result;
    FMOD_CREATESOUNDEXINFO exinfo;
    int                    key, recorddriver, numdrivers, count;
    unsigned int           version;    
    FILE                  *fp;
    unsigned int           datalength = 0, soundlength;
    int                   iscoreaudio = 0;

    /*
        Create a System object and initialize.
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

    result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, 0);
    ERRCHECK(result);

    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));

    exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.numchannels      = 2;
    exinfo.defaultfrequency = 44100;
   
    if (iscoreaudio)
    {
        exinfo.format = FMOD_SOUND_FORMAT_PCMFLOAT;
        exinfo.length = exinfo.defaultfrequency * sizeof(float) * exinfo.numchannels * 2;
    }
    else
    {
        exinfo.format = FMOD_SOUND_FORMAT_PCM16;
        exinfo.length = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * 2;
    }
 
    result = FMOD_System_CreateSound(system, 0, FMOD_2D | FMOD_SOFTWARE | FMOD_OPENUSER, &exinfo, &sound);
    ERRCHECK(result);

    printf("========================================================================\n");
    printf("Record to disk example.  Copyright (c) Firelight Technologies 2004-2011.\n");
    printf("========================================================================\n");
    printf("\n");
    printf("Press a key to start recording to record.wav\n");
    printf("\n");

    getch();

    result = FMOD_System_RecordStart(system, recorddriver, sound, TRUE);
    ERRCHECK(result);

    printf("Press 'Esc' to quit\n");
    printf("\n");

    fp = fopen("record.wav", "wb");
    if (!fp)
    {
        printf("ERROR : could not open record.wav for writing.\n");
        return 1;
    }

    /*
        Write out the wav header.  As we don't know the length yet it will be 0.
    */
    WriteWavHeader(fp, sound, datalength);

    result = FMOD_Sound_GetLength(sound, &soundlength, FMOD_TIMEUNIT_PCM);
    ERRCHECK(result);

    /*
        Main loop.
    */
    do
    {
        static unsigned int lastrecordpos = 0;
        unsigned int recordpos = 0;

        if (kbhit())
        {
            key = getch();
        }

        FMOD_System_GetRecordPosition(system, recorddriver, &recordpos);
        ERRCHECK(result);

        if (recordpos != lastrecordpos)        
        {
            void *ptr1, *ptr2;
            int blocklength;
            unsigned int len1, len2;
            
            blocklength = (int)recordpos - (int)lastrecordpos;
            if (blocklength < 0)
            {
                blocklength += soundlength;
            }

            /*
                Lock the sound to get access to the raw data.
            */
            FMOD_Sound_Lock(sound, lastrecordpos * 4, blocklength * 4, &ptr1, &ptr2, &len1, &len2);   /* *4 = stereo 16bit.  1 sample = 4 bytes. */

            /*
                Write it to disk.
            */
            if (ptr1 && len1)
            {
                #ifdef __BIG_ENDIAN__
                if (exinfo.format == FMOD_SOUND_FORMAT_PCM16)
                {
                    signed short *wptr = (signed short *)ptr1;

                    for (count = 0; count < len1 >> 1; count++)
                    {
                        wptr[count] = SWAPENDIAN_WORD(wptr[count]);
                    }
                }
                else if (exinfo.format == FMOD_SOUND_FORMAT_PCMFLOAT)
                {
                    float *fptr = (float *)ptr1;
    
                    for (count = 0; count < len1 >> 2; count++)
                    {
                        SWAPENDIAN_FLOAT(fptr[count]);
                    }
                }
                #endif

                datalength += fwrite(ptr1, 1, len1, fp);
            }
            if (ptr2 && len2)
            {
                #ifdef __BIG_ENDIAN__
                if (exinfo.format == FMOD_SOUND_FORMAT_PCM16)
                {
                    signed short *wptr = (signed short *)ptr2;

                    for (count = 0; count < len2 >> 1; count++)
                    {
                        wptr[count] = SWAPENDIAN_WORD(wptr[count]);
                    }
                }
                else if (exinfo.format == FMOD_SOUND_FORMAT_PCMFLOAT)
                {
                    float *fptr = (float *)ptr2;
    
                    for (count = 0; count < len2 >> 2; count++)
                    {
                        SWAPENDIAN_FLOAT(fptr[count]);
                    }
                }
                #endif

                datalength += fwrite(ptr2, 1, len2, fp);
            }

            /*
                Unlock the sound to allow FMOD to use it again.
            */
            FMOD_Sound_Unlock(sound, ptr1, ptr2, len1, len2);
        }

        lastrecordpos = recordpos;

        printf("\rRecord buffer pos = %6d : Record time = %02d:%02d", recordpos, datalength / exinfo.defaultfrequency / 4 / 60, (datalength / exinfo.defaultfrequency / 4) % 60);
        fflush(stdout);

        FMOD_System_Update(system);

        Sleep(10);

    } while (key != 27);

    printf("\n");

    /*
        Write back the wav header now that we know its length.
    */
    WriteWavHeader(fp, sound, datalength);

    fclose(fp);

    /*
        Shut down
    */
    result = FMOD_Sound_Release(sound);
    ERRCHECK(result);

    result = FMOD_System_Release(system);
    ERRCHECK(result);

    return 0;
}


