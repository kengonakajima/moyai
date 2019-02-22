


//#include "cumino.h"
#include "common.h"

#include "JPEGCoder.h"

JPEGCoder::JPEGCoder( int w, int h, int pixel_skip ) {
    orig_w = w;
    orig_h = h;
    capture_pixel_skip = pixel_skip;
    jpeg_quality = 70;
    
    capture_img = new Image();
    capture_img->setSize(w,h);    

    // same size as Image's
    int img_w = w/(pixel_skip+1), img_h = h/(pixel_skip+1);        
    sampary = (JSAMPARRAY) MALLOC( sizeof(JSAMPROW) * img_h );
    assert(sampary);
    for(int y=0;y<img_h;y++) {
        sampary[y] = (JSAMPROW) MALLOC( sizeof(JSAMPLE)*3*img_w);
        assert(sampary[y]);
    }
    compressed = (unsigned char*) MALLOC( MAX_COMPRESSED_SIZE );
    assert(compressed);
}

JPEGCoder::~JPEGCoder() {
    FREE(compressed);
    for(int y=0;y<capture_img->height;y++) {
        FREE(sampary[y]);
    }
    FREE(sampary);
    delete capture_img;
}
size_t JPEGCoder::encode(Image *tgt_image) {
    Image *input_image = tgt_image;
    if(!input_image) input_image = capture_img;
    double t0 = now();
    int step_w = orig_w/(capture_pixel_skip+1), step_h = orig_h/(capture_pixel_skip+1); 
    for(int y=0;y<step_h;y++) {
        int img_y = y * (capture_pixel_skip+1);
        for(int x=0;x<step_w;x++) {
            int img_x = x * (capture_pixel_skip+1);
            unsigned char r,g,b,a;            
            input_image->getPixelRaw( img_x, img_y, &r,&g,&b,&a);
            sampary[y][x*3+0] = r;
            sampary[y][x*3+1] = g;
            sampary[y][x*3+2] = b;
        }
    }
    double t1 = now();
    
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress( &cinfo);
    //    jpeg_stdio_dest(&cinfo,jfp);
    unsigned long outsize;
    jpeg_mem_dest( &cinfo, &compressed, &outsize );
    cinfo.image_width = step_w;
    cinfo.image_height = step_h;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults( &cinfo);
    jpeg_set_quality( &cinfo, jpeg_quality, true );
    jpeg_start_compress( &cinfo, true );
    jpeg_write_scanlines( &cinfo, sampary, step_h );
    jpeg_finish_compress( &cinfo );
    compressed_size = outsize;
    double t2 = now();

    if((t2-t1)>0.02) print("slow jpeg compress");
    if((t1-t0)>0.02) print("slow pixel copy");
    
    //    print("jpeg: time: %f, %f img:%d,%d", t1-t0, t2-t1, step_w, step_h);
    return outsize;
}

void JPEGCoder::setCompressedData( const unsigned char *indata, size_t len ) {
    assert(len <= MAX_COMPRESSED_SIZE);
    memcpy(compressed, indata,len );
    compressed_size = len;
}
void JPEGCoder::decode() {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error( &jerr );
    jpeg_create_decompress( &cinfo );

    jpeg_mem_src( &cinfo, compressed, compressed_size );
    jpeg_read_header( &cinfo, true );
    jpeg_start_decompress( &cinfo );    
    while( cinfo.output_scanline < cinfo.output_height ) {
        jpeg_read_scanlines( &cinfo,
                             sampary + cinfo.output_scanline,
                             cinfo.output_height - cinfo.output_scanline
                             );
    }
    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    //
    
    int step_w = orig_w/(capture_pixel_skip+1), step_h = orig_h/(capture_pixel_skip+1); 
    for(int y=0;y<step_h;y++) {
        for(int x=0;x<step_w;x++) {
            unsigned char r = sampary[y][x*3+0];
            unsigned char g = sampary[y][x*3+1];
            unsigned char b = sampary[y][x*3+2];
            unsigned char a = 0xff;
            capture_img->setPixelRaw( x, y, r,g,b,a);
        }
    }
}
