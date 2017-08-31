#include "client.h"

#include "IndexBuffer.h"


IndexBuffer::~IndexBuffer() {
	assert(buf);
	FREE(buf);
    unbless();
}

void IndexBuffer::reserve( int len ) {
	if(buf) FREE(buf);    
	buf = (IndexBufferType*) MALLOC( sizeof(IndexBufferType) * len );
	assert(buf);
	array_len = len;
    render_len = len;
}

void IndexBuffer::setIndex( int index_at, IndexBufferType val ) {
	assert(buf);
	assert(index_at >= 0 && index_at < array_len );
	buf[index_at] = val;
}

IndexBufferType IndexBuffer::getIndex( int index_at ) {
	assert(buf);
	assert(index_at >= 0 && index_at < array_len );
	return buf[index_at];
}

void IndexBuffer::set( IndexBufferType *in, int l ) {
	reserve(l);
	memcpy(buf, in, sizeof(IndexBufferType)*l);
}

void IndexBuffer::bless(){
#if !defined(__linux__)    
	if( gl_name == 0 ){
		glGenBuffers(1, &gl_name);
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gl_name );
		// データがよく変わるときは GL_DYNAMIC_DRAWらしいけど、それはコンセプトから外れた使い方だからデフォルトはSTATICにしておく。
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(IndexBufferType) * render_len, buf, GL_STATIC_DRAW );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	}
#endif    
}
void IndexBuffer::unbless() {
#if !defined(__linux__)    
    if( gl_name != 0 ) {
        glDeleteBuffers(1,&gl_name);
        gl_name = 0;
    }
#endif    
}

void IndexBuffer::dump(int lim) {
	print("ib: len:%d glname:%d", array_len, gl_name );
	if(buf){
		for(int i=0;i<array_len && i<lim;i++) {
			print("[%d]=%d",i, buf[i] );
		}
	} else {
		print("ib:nobuf");
	}
}
void IndexBuffer::setRenderLen(int l) {        
    assert( l <= array_len );
    render_len = l;        
}

void IndexBuffer::copyFromBuffer( IndexBufferType *inbuf, int ind_cnt ) {
	assertmsg( ind_cnt <= array_len, "size too big");
	render_len = ind_cnt;
	memcpy( buf, inbuf, ind_cnt * sizeof(IndexBufferType) );
}
