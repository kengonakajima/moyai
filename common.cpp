#include "common.h"
#define LODEPNG_COMPILE_DISK
#define LODEPNG_COMPILE_ERROR_TEXT
#include "lodepng.h"

int Prop::idgen = 1;
int Group::idgen = 1;
int Image::idgen = 1;


int Moyai::poll(double dt){
    if( dt <0 || dt > 1 ){ print( "poll too slow or negative. dt:%f(sec)", dt ); }
    if(dt==0){
        dt = 0.0001;
    }
    int cnt = 0;
    for(int i=0;i<elementof(groups);i++){
        Group *g = groups[i];
        if(g && g->skip_poll == false ) cnt += g->pollAllProps(dt);
    }
    return cnt;
}


bool Moyai::insertEvent( double duration, void (*cbf)( void *argptr), void *argptr ) {
    for(int i=0;i<elementof(events);i++) {
        if( events[i].isUsed() == false ) {
            events[i].cb = cbf;
            events[i].duration = duration;
            events[i].argptr = argptr;
            return true;
        }
    }
    print("insertEvent: event full");
    return false;
}
void Moyai::clearEvents() {
    for(int i=0;i<elementof(events);i++) {
        events[i].duration = 0;
        events[i].cb = NULL;
    }
}
void Moyai::pollEvents( double dt ) {
    for(int i=0;i<elementof(events);i++) {
        if( events[i].isUsed() ) {
            //       print("ev[%d]: used, act:%f dur:%f", i, events[i].accum_time, events[i].duration );
            events[i].accum_time += dt;
            if( events[i].accum_time > events[i].duration ) {
                events[i].cb( events[i].argptr );
                events[i].duration = events[i].accum_time = 0;
                return;
            }
        }
    }
}

int Group::countProps() {
    int n=0;
    Prop *cur = prop_top;
    while(cur) {
        n++;
        cur = cur->next;
    }
    return n;
}


int Group::pollAllProps(double dt ){
    int cnt=0;
    Prop *cur = prop_top;

    // poll
    Prop *to_clean[32*1024]; // 1ループにこの個数までclean
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
        if( p->debug_id ) print("deleting p:%p prev:%p next:%p", p, p->prev, p->next );

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
#ifdef __linux__
        std::map<int,Prop*>::iterator ii = idmap.find(p->id);
#else        
        std::unordered_map<int,Prop*>::iterator ii = idmap.find(p->id);
#endif        
        idmap.erase(ii);
        delete p;
    }
    last_poll_num = cnt;

    return cnt;
}

Prop *Group::findPropById( int id ) {
#if 0    
    Prop *cur = prop_top;
    while(cur){
        if( cur->id == id ) return cur;
        cur = cur->next;
    }
    return NULL;
#else
    return idmap[id];
#endif    
}

double Prop::frame_step_time = 1.0f/60.0f;

bool Prop::pollCount(unsigned int value, double &timestamp)
{ 
	double requestedElapsedTime = value * frame_step_time;
	double elapsedTime = accum_time - timestamp;

	if (elapsedTime >= requestedElapsedTime)
	{
		timestamp = accum_time - (elapsedTime - requestedElapsedTime);
		return true;
	}

	return false;
}

bool Prop::basePoll(double dt){

    if(to_clean){
        return false;
    }

    accum_time += dt;
	poll_accum_time += dt;

	while (poll_accum_time >= frame_step_time) {
		++poll_count;
		poll_accum_time -= frame_step_time;
	}
    
    if( propPoll(dt) == false ){
        return false;
    }

    
    return true;
}
bool Prop::updateInterval( int timer_ind, double t ) {
    assert(timer_ind>=0 && timer_ind<MAXINTERVAL);
    if( accum_time > last_interval_at[timer_ind] + t ) {
        last_interval_at[timer_ind] = accum_time;
        return true;
    } else {
        return false;
    }
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
        buffer[index] = (unsigned char) (c.r*255);
        buffer[index+1] = (unsigned char) (c.g*255);
        buffer[index+2] = (unsigned char) (c.b*255);
        buffer[index+3] = (unsigned char) (c.a*255);
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
        return Color( ((float)r)/255.0f, ((float)g)/255.0f, ((float)b)/255.0f, ((float)a)/255.0f );
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
// x0,y0 can be negative, x0+w, y0+h can be out of image.
// set color 0x0 when out of image
void Image::getAreaRaw( int x0, int y0, int w, int h, unsigned char *out, size_t outsz ) {
    size_t reqsize = w*h*4;
    assert( outsz >= reqsize );    
    for(int dy=0;dy<h;dy++) {
        for(int dx=0;dx<w;dx++) {
            int out_index = ( dx + dy * w ) * 4;
            int x = x0+dx, y = y0+dy;
            if(x<0||y<0||x>=width||y>=height) {
                out[out_index] = out[out_index+1] = out[out_index+2] = 0;
                out[out_index+3] = 0xff;
            } else {
                int index = ( x + y * width ) * 4;
                out[out_index] = buffer[index]; // r
                out[out_index+1] = buffer[index+1]; // g
                out[out_index+2] = buffer[index+2]; // b
                out[out_index+3] = buffer[index+3]; // a
            }
        }
    }
}
// x0,y0 can be negative, x0+w, y0+h can be out of image. (colors are ignored)
void Image::setAreaRaw( int x0, int y0, int w, int h, unsigned char *in, size_t insz ) {
    size_t reqsize = w*h*4;
    assert( insz >= reqsize );
    for(int dy=0;dy<h;dy++) {
        for(int dx=0;dx<w;dx++) {
            int x = x0+dx, y = y0+dy;
            if(x<0||y<0||x>=width||y>=height)continue;            
            int index = ( x + y * width ) * 4;
            int in_index = ( dx + dy * w ) * 4;
            buffer[index] = in[in_index]; // r
            buffer[index+1] = in[in_index+1]; // g
            buffer[index+2] = in[in_index+2]; // b
            buffer[index+3] = in[in_index+3]; // a            
        }
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


bool Image::loadPNG( const char *path ) {
    FILE *fp = fopen(path,"rb");
    if(!fp) {
        print( "Image::loadPNG: can't open file:%s", path );
        return false;
    }

    unsigned error;
    unsigned char* image_data = (unsigned char*) MALLOC( 2048 * 2048 * 4 );
    unsigned w, h;

    error = lodepng_decode32_file(&image_data, &w, &h, path );

    if(error) {
        fprintf(stderr, "decoder error %u: %s\n", error, lodepng_error_text(error) );
        FREE(image_data);
        fclose(fp);
        return false;
    }

    width = w;
    height = h;
    
    ensureBuffer();
#define IMAGE_BUFFER_COPY \
    for(int i=0;i<width*height;i++){ \
        int x = i % width; \
        int y = (i / width); \
        int ii = y * width + x; \
        buffer[ii*4+0] = image_data[i*4+0]; /*r*/   \
        buffer[ii*4+1] = image_data[i*4+1]; /*g*/   \
        buffer[ii*4+2] = image_data[i*4+2]; /*b*/   \
        buffer[ii*4+3] = image_data[i*4+3]; /*a*/   \
    }

    IMAGE_BUFFER_COPY;
    
    // clean up
    FREE(image_data);
    fclose(fp);

    return true;
}
bool Image::loadPNGMem( unsigned char *ptr, size_t sz ) {
    unsigned error;
    unsigned char* image_data = (unsigned char*) MALLOC( 2048 * 2048 * 4 );
    unsigned w, h;    
    error = lodepng_decode32( &image_data, &w, &h,  ptr,sz );
    if(error) {
        fprintf(stderr, "decoder error %u: %s\n", error, lodepng_error_text(error) );
        FREE(image_data);
        return false;
    }
    width = w;
    height = h;
    ensureBuffer();
    IMAGE_BUFFER_COPY;
    FREE(image_data);
    return true;    
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
            r = (unsigned char)( fromr * rate + tor * (1-rate) );
            g = (unsigned char)( fromg * rate + tog * (1-rate) );
            b = (unsigned char)( fromb * rate + tob * (1-rate) );
            int total_a = froma + toa;
            if(total_a>255) total_a = 255;
            a = total_a;
            
            setPixelRaw( tox, toy, r, g, b, a );
        
            
        }
    }
}

bool Image::writePNGMem( unsigned char **out, size_t *outsize ) {
    assertmsg( buffer!=0 , "image not initialized?" );
    assertmsg( width <= 2048 && height <= 2048, "image too big" );
    unsigned error;
    error = lodepng_encode32( out, outsize, buffer, width, height );
    if(error) {
        fprintf(stderr, "lodepng_encode32_file failed%d", error );
        return false;
    }
    return true;        
}
bool Image::writePNG(const char *path) {

    assertmsg( buffer!=0 , "image not initialized?" );
    assertmsg( width <= 2048 && height <= 2048, "image too big" );
    

    /*Same as lodepng_encode_file, but always encodes from 32-bit RGBA raw image.*/
    unsigned error;
    error = lodepng_encode32_file( path, buffer, width, height );
    if(error) {
        fprintf(stderr, "lodepng_encode32_file failed%d", error );
        return false;
    }
    return true;
}
bool Image::writeRaw( const char *path ) {
    return writeFile( path, (char*) buffer, width*height*4 );
}
bool Image::loadRaw( const char *path ) {
    size_t expectsz = width*height*4;
    size_t sz = expectsz;
    bool ret = readFile( path, (char*) buffer, &sz );
    if(!ret)return false;
    if(sz!=expectsz) return false;
    return true;
}

void Image::fill( Color c ) {
    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++) {
            setPixel(x,y,c);
        }
    }
}

void Image::setOptionalLoadPath( const char *cstrpath ) {
    strncpy( optional_load_path, cstrpath, sizeof(optional_load_path) );
}
