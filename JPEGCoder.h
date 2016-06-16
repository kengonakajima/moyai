#ifndef _JPEGENCODER_
#define _JPEGENCODER_

#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#include "jpeg-8d/jpeglib.h"
#else
#include <jpeglib.h>
#endif

class Image;

class JPEGCoder {
public:
    Image *capture_img;
    int capture_pixel_skip; // Adjust sampling density of captures
    static const size_t MAX_COMPRESSED_SIZE = 1024*1024;
    unsigned char *compressed;
    size_t compressed_size;
    int orig_w, orig_h;
    int jpeg_quality;
    
    // for libjpeg use
    JSAMPARRAY sampary;
        
    JPEGCoder( int w, int h, int pixel_skip );
    ~JPEGCoder();

    Image *getImage() { return capture_img; }
    size_t encode();
    void setCompressedData( const unsigned char *indata, size_t len );
    void decode();
};

#endif
