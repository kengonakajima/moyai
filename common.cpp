#include "common.h"


int Prop::idgen = 1;
int Group::idgen = 1;



int Moyai::pollAll(double dt){
    if( dt <0 || dt > 1 ){ print( "poll too slow or negative. dt:%f", dt ); }
    if(dt==0){
        dt = 0.0001;
    }
    int cnt = 0;
    for(int i=0;i<elementof(groups);i++){
        Group *g = groups[i];
        if(g) cnt += g->pollAllProps(dt);
    }
    return cnt;
}
int Group::pollAllProps(double dt ){
    int cnt=0;
    Prop *cur = prop_top;

    // poll
    Prop *to_clean[256]; // 1ループにこの個数まで
    int to_clean_cnt = 0;

    while(cur){
        cnt++;
        bool to_keep = cur->basePoll(dt);
        if(!to_keep) cur->to_clean = true;
        if( cur->to_clean ){
            if( to_clean_cnt < elementof(to_clean) ){
                to_clean[ to_clean_cnt ] = cur;
                to_clean_cnt ++;
            }
        }
        cur = cur->next;
    }

    // clean 高速版
    //    if( to_clean_cnt > 0 ) print("top-p:%p clean_n:%d", prop_top, to_clean_cnt );

    for(int i=0;i<to_clean_cnt;i++){
        Prop *p = to_clean[i];
        //        print("deleting p:%p prev:%p next:%p", p, p->prev, p->next );

        if(p == prop_top ){
            prop_top = p->next;
        }
        if(p->prev ){
            p->prev->next = p->next;
        }
        if(p->next){
            p->next->prev = p->prev;
        }
        p->next = NULL;
        p->onDelete();
        delete p;
    }
    last_poll_num = cnt;

    return cnt;
}
// TODO: avoid linear scan..
Prop *Group::findPropById( int id ) {
    Prop *cur = prop_top;
    while(cur){
        if( cur->id == id ) return cur;
        cur = cur->next;
    }
    return NULL;
}
bool Prop::basePoll(double dt){

    if(to_clean){
        return false;
    }

    accum_time += dt;
    poll_count ++;

    
    if( propPoll(dt) == false ){
        return false;
    }

    
    return true;
}


void Image::setSize(int w, int h ) {
    width = w; height = h;
    ensureBuffer();
}    
void Image::ensureBuffer() {
    if(!buffer){
        size_t sz = width*height*4;
        buffer = (unsigned char*) MALLOC(sz);
        assert(buffer);
        memset(buffer, 0, sz );
    }
}

void Image::setPixel( int x, int y, Color c ){
    assert( width > 0 && height > 0);
    ensureBuffer();
    if(x>=0&&y>=0&&x<width&&y<height){
        int index = ( x + y * width ) * 4;
        buffer[index] = c.r*255;
        buffer[index+1] = c.g*255;
        buffer[index+2] = c.b*255;
        buffer[index+3] = c.a*255;
    }
}
Color Image::getPixel( int x, int y ) {
    assert( width > 0 && height > 0 );
    assert(buffer);
    if(x>=0&&y>=0&&x<width&&y<height){
        int index = ( x + y * width ) * 4;
        unsigned char r = buffer[index];
        unsigned char g = buffer[index+1];
        unsigned char b = buffer[index+2];
        unsigned char a = buffer[index+3];
        return Color( ((float)r)/255.0, ((float)g)/255.0, ((float)b)/255.0, ((float)a)/255.0 );
    } else {
        return Color( 0,0,0,1 );
    }
}
void Image::getPixelRaw( int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a ) {
    assert( width > 0 && height > 0 );
    assert(buffer);
    if(x>=0&&y>=0&&x<width&&y<height){
        int index = ( x + y * width ) * 4;
        *r = buffer[index];
        *g = buffer[index+1];
        *b = buffer[index+2];
        *a = buffer[index+3];
    }
}
void Image::setPixelRaw( int x, int y, unsigned char r,  unsigned char g,  unsigned char b,  unsigned char a ) {
    assert( width > 0 && height > 0 );
    assert(buffer);
    if(x>=0&&y>=0&&x<width&&y<height){
        int index = ( x + y * width ) * 4;
        buffer[index] = r;
        buffer[index+1] = g;
        buffer[index+2] = b;
        buffer[index+3] = a;
    }
}

// http://stackoverflow.com/questions/11296644/loading-png-textures-to-opengl-with-libpng-only
void Image::loadPNG( const char *path ) {
    FILE *fp = fopen(path,"rb");

    png_byte header[8];
    assertmsg(fp, "can't open file:%s", path );

    // read the header
    fread(header, 1, 8, fp);

    if(png_sig_cmp(header, 0, 8)) assertmsg(false, "error: %s is not a PNG.", path );

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assertmsg( png_ptr, "png_create_read_struct returned 0 for %s", path );

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        assertmsg( false, "png_create_info_struct returned 0 for %s", path );
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        assertmsg(false, "png_create_info_struct returned 0 for %s", path );
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        assertmsg( false, "error from libpng for %s", path );
    }

    // init png reading
    png_init_io(png_ptr, fp);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type, NULL, NULL, NULL);

    width = temp_width;
    height = temp_height;


    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte * image_data = (png_byte*)MALLOC(rowbytes * temp_height * sizeof(png_byte)+15);
    if (image_data == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        assertmsg( false, "could not allocate memory for PNG image data for %s", path );
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_bytep * row_pointers = (png_bytep*) MALLOC(temp_height * sizeof(png_bytep));
    if (row_pointers == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        assertmsg( false, "could not allocate memory for PNG row pointers for %s", path );
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    for (unsigned int i = 0; i < temp_height; i++) {
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    ensureBuffer();

    for(int i=0;i<width*height;i++){
        int x = i % width;
        int y = height - 1 - (i / width);
        int ii = y * width + x;
        buffer[ii*4+0] = image_data[i*4+0]; // r
        buffer[ii*4+1] = image_data[i*4+1]; // g
        buffer[ii*4+2] = image_data[i*4+2]; // b
        buffer[ii*4+3] = image_data[i*4+3]; // a
    }

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    FREE(image_data);
    FREE(row_pointers);
    fclose(fp);    
}

// copy inside image
void Image::copyAlpha( int fromx0, int fromy0, int fromx1, int fromy1, int tox0, int toy0 ) {
    int w = fromx1 - fromx0;
    int h = fromy1 - fromy0;
    assert( w > 0 );
    assert( h > 0 );    
    
    for(int y=0;y<h;y++){
        for( int x=0;x<w;x++){
            int fromx = fromx0 + x, fromy = fromy0 + y;
            int tox = tox0 + x, toy = toy0 + y;
            unsigned char fromr,fromg,fromb,froma, tor, tog, tob, toa;
            getPixelRaw( fromx, fromy, &fromr, &fromg, &fromb, &froma );
            getPixelRaw( tox, toy, &tor, &tog, &tob, &toa );
            unsigned char r,g,b,a;

            float rate = (float)froma/(float)255.0;
            r = fromr * rate + tor * (1-rate);
            g = fromg * rate + tog * (1-rate);
            b = fromb * rate + tob * (1-rate);
            int total_a = froma + toa;
            if(total_a>255) total_a = 255;
            a = total_a;
            
            setPixelRaw( tox, toy, r, g, b, a );
        
            
        }
    }
}


bool Image::writePNG(const char *path) {
    assertmsg( buffer, "image not initialized?" );
    FILE *fp = fopen( path, "wb");

    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep * row_pointers = (png_bytep*) MALLOC( height * sizeof(png_bytep) );
    assert(row_pointers);
    for(int y=0;y<height;y++){
        png_byte* row = (png_byte*) MALLOC( width * 4 );
        row_pointers[y] = row;
        for(int x=0;x<width;x++){
            int bi = x + y * width;
            row[x*4+0] = buffer[bi*4+0];
            row[x*4+1] = buffer[bi*4+1];            
            row[x*4+2] = buffer[bi*4+2];            
            row[x*4+3] = buffer[bi*4+3];            
        }
    }

    png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
    if(!png_ptr) return false;
    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) return false;
    if( setjmp( png_jmpbuf(png_ptr))) return false;
    png_init_io( png_ptr, fp );

    // write header
    if( setjmp( png_jmpbuf(png_ptr))) return false;
    png_set_IHDR( png_ptr, info_ptr,
                  width, height,
                  8, // bit depth
                  6, // RGBA
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_BASE,
                  PNG_FILTER_TYPE_BASE 
                  );
    png_write_info( png_ptr, info_ptr );

    if( setjmp( png_jmpbuf(png_ptr))) return false;

    png_write_image( png_ptr, row_pointers );

    if( setjmp( png_jmpbuf(png_ptr))) return false;

    png_write_end(png_ptr,NULL);

    for(int y=0;y<height;y++){
        FREE(row_pointers[y]);
    }
    FREE(row_pointers);
    
    fclose(fp);

    
    
    return true;
}

