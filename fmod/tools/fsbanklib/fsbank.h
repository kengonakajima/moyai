#ifndef _FSBANK_H
#define _FSBANK_H

#if defined(_WIN32)
    #define FB_API __declspec(dllexport) _stdcall
    #define FSBANK_CALLBACK _stdcall
#elif defined(__linux__)
    #define FB_API __attribute__ ((visibility("default")))
    #define FSBANK_CALLBACK
#else
    #define FB_API
    #define FSBANK_CALLBACK
#endif

typedef unsigned int FSBANK_INITFLAGS;

#define FSBANK_INIT_NORMAL                  0x00000000  /* Initialize normally. */
#define FSBANK_INIT_IGNOREERRORS            0x00000001  /* Ignore individual subsound build errors, continue building for as long as possible. */
#define FSBANK_INIT_WARNINGSASERRORS        0x00000002  /* Treat any warnings issued as errors. */
#define FSBANK_INIT_CREATEINCLUDEHEADER     0x00000004  /* Create C header files with #defines defining indices for each member of the FSB. */
#define FSBANK_INIT_DONTLOADCACHEFILES      0x00000008  /* Ignore existing cache files. */
#define FSBANK_INIT_GENERATEPROGRESSITEMS   0x00000010  /* Generate status items that can be queried by another thread to monitor the build progress and give detailed error messages. */
#define FSBANK_INIT_UNICODE                 0x00000020  /* All file paths are treated as wide chars (Windows only). */


typedef unsigned int FSBANK_BUILDFLAGS;

#define FSBANK_BUILD_DEFAULT                0x00000000  /* Build with default settings. */
#define FSBANK_BUILD_DISABLESYNCPOINTS      0x00000001  /* Disable the storing of syncpoints in the output */
#define FSBANK_BUILD_DONTLOOP               0x00000002  /* Disable perfect loop encoding and sound stretching. Removes chirps from the start of oneshot MP2, MP3 and IMAADPCM sounds. */
#define FSBANK_BUILD_FILTERHIGHFREQ         0x00000004  /* XMA only. Enable high frequency filtering. */
#define FSBANK_BUILD_DISABLESEEKING         0x00000008  /* XMA only. Disable seek tables to save memory. */
#define FSBANK_BUILD_OPTIMIZESAMPLERATE     0x00000010  /* Attempt to optimize the sample rate down. Ignored if format is MP2, MP3 or CELT or if FSB4 basic headers flag is used. */
#define FSBANK_BUILD_DONTINTERLEAVE         0x00000020  /* VAG and GCADPCM only. Don't interleave the data as LRLRLRLR... instead use LLLLRRRR type encoding. Speeds up loading for samples (not streams, streams must remained interleaved), allows FMOD_OPENMEMORY_POINT. */
#define FSBANK_BUILD_FSB4_USEBASICHEADERS   0x00000040  /* FSB4 format only. Generate FSBs with small sample header data. They only contain basic information such as sample length, and everything else has its attributes inherited from the first sample (for example the default frequency). */
#define FSBANK_BUILD_FSB5_DONTWRITENAMES    0x00000080  /* FSB5 format only. Do not write out a names chunk to the FSB to reduce file size. */
#define FSBANK_BUILD_NOGUID                 0x00000100  /* FSB5 format only. Write out a null GUID for the FSB header.  The runtime will not use header caching for these FSB files. */

/* Build flag mask that specifies which settings can be overridden per subsound. */
#define FSBANK_BUILD_OVERRIDE_MASK          (FSBANK_BUILD_DISABLESYNCPOINTS | FSBANK_BUILD_DONTLOOP | FSBANK_BUILD_FILTERHIGHFREQ | FSBANK_BUILD_DISABLESEEKING | FSBANK_BUILD_OPTIMIZESAMPLERATE | FSBANK_BUILD_DONTINTERLEAVE)
/* Build flag mask that specifies which settings (when changed) invalidate a cache file. */
#define FSBANK_BUILD_CACHE_VALIDATION_MASK  (FSBANK_BUILD_DONTLOOP | FSBANK_BUILD_FILTERHIGHFREQ | FSBANK_BUILD_OPTIMIZESAMPLERATE | FSBANK_BUILD_DONTINTERLEAVE)


typedef enum FSBANK_RESULT
{
    FSBANK_OK,                              /* No errors. */
    FSBANK_ERR_CACHE_CHUNKNOTFOUND,         /* An expected chunk is missing from the cache, perhaps try deleting cache files. */
    FSBANK_ERR_CANCELLED,                   /* The build process was cancelled during compilation by the user. */
    FSBANK_ERR_CANNOT_CONTINUE,             /* The build process cannot continue due to previously ignored errors. */
    FSBANK_ERR_ENCODER,                     /* Encoder for chosen format has encountered an unexpected error. */
    FSBANK_ERR_ENCODER_INIT,                /* Encoder initialization failed. */
    FSBANK_ERR_ENCODER_NOTSUPPORTED,        /* Encoder for chosen format is not supported on this platform. */
    FSBANK_ERR_FILE_OS,                     /* An operating system based file error was encountered. */
    FSBANK_ERR_FILE_NOTFOUND,               /* A specified file could not be found. */
    FSBANK_ERR_FMOD,                        /* Internal error from FMOD sub-system. */
    FSBANK_ERR_INITIALIZED,                 /* Already initialized. */
    FSBANK_ERR_INVALID_FORMAT,              /* The format of the source file is invalid, see output for details. */
    FSBANK_ERR_INVALID_PARAM,               /* An invalid parameter has been passed to this function. */
    FSBANK_ERR_MEMORY,                      /* Ran out of memory. */
    FSBANK_ERR_UNINITIALIZED,               /* Not initialized yet. */
    FSBANK_ERR_WRITER_FORMAT,               /* Chosen encode format is not supported by this FSB version. */
    FSBANK_WARN_CANNOTLOOP,                 /* Source file is too short for seamless looping. Looping disabled. */
    FSBANK_WARN_IGNORED_OPTIMIZESAMPLERATE, /* FSBANK_BUILD_OPTIMIZESAMPLERATE flag ignored: MP2, MP3 and CELT formats, and the USEBASICHEADERS flag make this option irrelevant. */
    FSBANK_WARN_IGNORED_FILTERHIGHFREQ,     /* FSBANK_BUILD_FILTERHIGHFREQ flag ignored: feature only supported by XMA format. */
    FSBANK_WARN_IGNORED_DISABLESEEKING,     /* FSBANK_BUILD_DISABLESEEKING flag ignored: feature only supported by XMA format. */
    FSBANK_WARN_IGNORED_DONTINTERLEAVE,     /* FSBANK_BUILD_DONTINTERLEAVE flag ignored: feature only supported by VAG and GCADPCM formats. */
} FSBANK_RESULT;


typedef enum FSBANK_FORMAT
{
    FSBANK_FORMAT_PCM,              /* PCM                                 (1:1)   All platforms. */
    FSBANK_FORMAT_PCM_BIGENDIAN,    /* PCM Big Endian                      (1:1)   Xbox360 / PS3 / WiiU only. */
    FSBANK_FORMAT_IMAADPCM,         /* IMA ADPCM                           (3.5:1) All platforms. */
    FSBANK_FORMAT_MP2,              /* MPEG Layer 2                        (CBR)   All platforms except PS3.            Depends on toolame. */
    FSBANK_FORMAT_MP3,              /* MPEG Layer 3                        (CBR)   All platforms.                       Depends on libmp3lame. */
    FSBANK_FORMAT_XMA,              /* XMA                                 (VBR)   Xbox360 / XboxOne only (hardware).   Depends on xmaencoder. */
    FSBANK_FORMAT_GCADPCM,          /* GCADPCM                             (3.5:1) Wii / WiiU / 3DS only (hardware).    Depends on dsptool. */
    FSBANK_FORMAT_VAG,              /* VAG                                 (3.5:1) PSP only (hardware).                 Depends on encvag. */
    FSBANK_FORMAT_HEVAG,            /* High Efficiency VAG                 (3.5:1) PSVita only (hardware).              Depends on vagconv2. */
    FSBANK_FORMAT_CELT,             /* Constrained Energy Lapped Transform (CBR)   All platforms.                       Depends on celt_encoder. */
    FSBANK_FORMAT_AT9_VITA,         /* ATRAC9                              (CBR)   PSVita (hardware).                   Depends on libatrac9. */
    FSBANK_FORMAT_AT9_PS4,          /* ATRAC9                              (CBR)   PS4 (hardware).                      Depends on libatrac9. */
    FSBANK_FORMAT_XWMA,             /* XWMA                                (VBR)   Xbox360 only.                        Depends on xwmaencoder. */
    FSBANK_FORMAT_VORBIS,           /* Vorbis                              (VBR)   All platforms.                       Depends on libvorbis. */

    FSBANK_FORMAT_MAX               /* Upper bound for this enumeration, for use with validation. */
} FSBANK_FORMAT;


typedef enum FSBANK_FSBVERSION
{
    FSBANK_FSBVERSION_FSB4, /* Produce FSB version 4 files. */
    FSBANK_FSBVERSION_FSB5, /* Produce FSB version 5 files. */

    FSBANK_FSBVERSION_MAX   /* Upper bound for this enumeration, for use with validation. */
} FSBANK_FSBVERSION;


typedef enum FSBANK_SPEAKERMAP
{
    FSBANK_SPEAKERMAP_DEFAULT,      /* Sample uses default FMOD speaker mapping. */
    FSBANK_SPEAKERMAP_ALLMONO,      /* Sample is a collection of mono channels. */
    FSBANK_SPEAKERMAP_ALLSTEREO,    /* Sample is a collection of stereo channel pairs. */
    FSBANK_SPEAKERMAP_PROTOOLS,     /* Sample is 6ch and uses L C R LS RS LFE standard. */   

    FSBANK_SPEAKERMAP_MAX           /* Upper bound for this enumeration, for use with validation. */
} FSBANK_SPEAKERMAP;


typedef enum FSBANK_STATE
{
    FSBANK_STATE_DECODING,      /* Decode a file to usable raw sample data. */
    FSBANK_STATE_ANALYSING,     /* Scan sound data for details (such as optimized sample rate). */
    FSBANK_STATE_PREPROCESSING, /* Prepares sound data for encoder. */ 
    FSBANK_STATE_ENCODING,      /* Pass the sample data to the chosen encoder. */
    FSBANK_STATE_WRITING,       /* Write encoded data into an FSB. */
    FSBANK_STATE_FINISHED,      /* Process complete. */
    FSBANK_STATE_FAILED,        /* An error has occurred, check data (as FSBANK_STATEDATA_FAILED) for details. */
    FSBANK_STATE_WARNING,       /* A warning has been issued, check data (as FSBANK_STATEDATA_WARNING) for details. */
} FSBANK_STATE;


typedef struct FSBANK_SUBSOUND
{
    const char* const  *fileNames;              /* List of file names used to produce an interleaved sound. */
    unsigned int        numFileNames;           /* Number of files in above file name list, up to 16. */
    FSBANK_SPEAKERMAP   speakerMap;             /* Setting to define the mapping and order of channels. */
    FSBANK_BUILDFLAGS   overrideFlags;          /* Flags that will reverse the equivalent flags passed to FSBank_Build. */
    unsigned int        overrideQuality;        /* Override the quality setting passed to FSBank_Build. */
    float               desiredSampleRate;      /* Resample to this sample rate (ignores optimize sample rate setting), up to 192000Hz. */
    float               percentOptimizedRate;   /* If using FSBANK_BUILD_OPTIMIZESAMPLERATE, this is the percentage of that rate to be used, up to 100.0%. */
} FSBANK_SUBSOUND;


typedef struct FSBANK_PROGRESSITEM
{
    int             subSoundIndex;  /* Index into the subsound list passed to FSBank_Build that this update relates to (-1 indicates no specific subsound). */
    int             threadIndex;    /* Which thread index is serving this update (-1 indicates FSBank_Build / main thread). */
    FSBANK_STATE    state;          /* Progress through the encoding process. */
    const void     *stateData;      /* Cast to state specific data structure for extra information. */
} FSBANK_PROGRESSITEM;


typedef struct FSBANK_STATEDATA_FAILED
{
    FSBANK_RESULT errorCode;    /* Error result code. */
    char errorString[256];      /* Description for error code. */
} FSBANK_STATEDATA_FAILED;


typedef struct FSBANK_STATEDATA_WARNING
{
    FSBANK_RESULT warnCode;     /* Warning result code. */
    char warningString[256];    /* Description for warning code. */
} FSBANK_STATEDATA_WARNING;


#ifdef __cplusplus
extern "C" {
#endif  

typedef void* (FSBANK_CALLBACK *FSBANK_MEMORY_ALLOCCALLBACK)(unsigned int size, unsigned int, const char *sourceStr);
typedef void* (FSBANK_CALLBACK *FSBANK_MEMORY_REALLOCCALLBACK)(void *ptr, unsigned int size, unsigned int, const char *sourceStr);
typedef void  (FSBANK_CALLBACK *FSBANK_MEMORY_FREECALLBACK)(void *ptr, unsigned int, const char *sourceStr);


/*
[API]
[
    [DESCRIPTION]
    Specifies a method for FSBank to allocate memory through callbacks.

    [PARAMETERS]
    'useralloc'     Overrides the internal calls to alloc. Compatible with ANSI malloc().
    'userrealloc'   Overrides the internal calls to realloc. Compatible with ANSI realloc().
    'userfree'      Overrides the internal calls to free. Compatible with ANSI free().
]
*/
FSBANK_RESULT FB_API FSBank_MemoryInit(FSBANK_MEMORY_ALLOCCALLBACK userAlloc, FSBANK_MEMORY_REALLOCCALLBACK userRealloc, FSBANK_MEMORY_FREECALLBACK userFree);


/*
[API]
[
    [DESCRIPTION]
    Initialize the FSBank system.

    [PARAMETERS]
    'version'               FSB version, currently only FSBANK_FSBVERSION_FSB4 is supported.
    'flags'                 Initialization flags which control how the system behaves.
    'numSimultaneousJobs'   The maximum number of threads to create for parallel encoding. Set this
                            to your number of CPU 'cores' for best performance.
    'cacheDirectory'        Optional location to store the temporary cache files, default is a
                            directory off the current working directory.
]
*/
FSBANK_RESULT FB_API FSBank_Init(FSBANK_FSBVERSION version, FSBANK_INITFLAGS flags, unsigned int numSimultaneousJobs, const char *cacheDirectory);


/*
[API]
[
    [DESCRIPTION]
    Release the FSBank system, clean up used resources.

    Note: All progress items retrieved with FSBank_FetchNextProgressItem will be released by this function.
]
*/
FSBANK_RESULT FB_API FSBank_Release();


/*
[API]
[
    [DESCRIPTION]
    Begin the building process for the provided subsound descriptions, function will block until
    complete.

    [PARAMETERS]
    'subSounds'             An array of subsound descriptions each defining one subsound for the
                            final FSB.
    'numSubSounds'          The number of elements in the 'subSounds' array.
    'encodeFormat'          The format to be used for encoding the FSB.
    'buildFlags'            Building flags which control how the sample data is encoded.
    'quality'               Controls the quality level after compression. From 1 (high compression
                            / low quality) to 100 (high quality / low compression), use 0 for
                            default quality.
                            AT9:    Bitrate (Kbps) {x2 for stereo} [36, 48, 60, 72, 84, 96] maps
                                    to FMOD quality [1 to 100]
                            CELT:   Bitrate (Kbps) = FMOD quality * 3.2
                            MPEG:   Bitrate (Kbps) = FMOD quality * 3.2
                            Vorbis: Vorbis quality [-0.1 to 1.0] maps to FMOD quality [1 to 100] 
                            XMA:    XMA quality = FMOD quality
                            xWMA:   Bitrate (Kbps) depends on sample rate and channel count, quality
                                    will choose bitrate if multiple options are available:
                                    1ch 22KHz = [20]
                                    1ch 32KHz = [20]
                                    1ch 44KHz = [32, 48]
                                    2ch 22Khz = [32]
                                    2ch 32Khz = [32, 48]
                                    2ch 44KHz = [32, 48, 96, 192]
                                    2ch 48KHz = [48, 64, 96, 160, 192]
                                    6ch 44KHz = [96, 192]
                                    6ch 48KHz = [48, 192]
    'encryptKey'            Optional string 'key' used to encrypt the FSB, same key is required at
                            runtime for decryption.
    'outputFileName'        Name (and path) of the FSB to produce.
]
*/
FSBANK_RESULT FB_API FSBank_Build(const FSBANK_SUBSOUND *subSounds, unsigned int numSubSounds, FSBANK_FORMAT encodeFormat, FSBANK_BUILDFLAGS buildFlags, unsigned int quality, const char *encryptKey, const char *outputFileName);


/*
[API]
[
    [DESCRIPTION]
    Halt the build in progress, must be called from a different thread to FSBank_Build.
]
*/
FSBANK_RESULT FB_API FSBank_BuildCancel();


/*
[API]
[
    [DESCRIPTION]
    Fetch build progress items that describe the current state of the build. Can be called while
    the build is in progress to get realtime updates or after the build for a report. Call
    FSBank_ReleaseProgressItem to free allocated memory.

    [PARAMETERS]
    'progressItem'          One status update about the progress of a particular subsound.
]
*/
FSBANK_RESULT FB_API FSBank_FetchNextProgressItem(const FSBANK_PROGRESSITEM **progressItem);


/*
[API]
[
    [DESCRIPTION]
    Release memory associated with a progress item.

    [PARAMETERS]
    'progressItem'          One status update about the progress of a particular subsound.
]
*/
FSBANK_RESULT FB_API FSBank_ReleaseProgressItem(const FSBANK_PROGRESSITEM *progressItem);


/*
[API]
[
    [DESCRIPTION]
    Query the current and maximum memory usage of the FSBank system.
    FSBank_Build.

    [PARAMETERS]
    'currentAllocated'      Address of a variable that receives the currently allocated memory at time of call. Optional. Specify 0 or NULL to ignore. 
    'maximumAllocated'      Address of a variable that receives the maximum allocated memory since FSBank_Init. Optional. Specify 0 or NULL to ignore.     
]
*/
FSBANK_RESULT FB_API FSBank_MemoryGetStats(unsigned int *currentAllocated, unsigned int *maximumAllocated);

#ifdef __cplusplus
}
#endif

#endif  // _FSBANK_H
