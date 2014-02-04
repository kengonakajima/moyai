#ifdef USE_OPENGL

#include "IndexBuffer_OGL.h"
#include "../cumino.h"

IndexBuffer_OGL::~IndexBuffer_OGL() {
	assert(buf);
	FREE(buf);
	glDeleteBuffers(1,&gl_name);
}

void IndexBuffer_OGL::reserve( int len ) {
	if(buf) FREE(buf);    
	buf = (int*) MALLOC( sizeof(int) * len );
	assert(buf);
	array_len = len;
}

void IndexBuffer_OGL::setIndex( int index_at, int val ) {
	assert(buf);
	assert(index_at >= 0 && index_at < array_len );
	buf[index_at] = val;
}

int IndexBuffer_OGL::getIndex( int index_at ) {
	assert(buf);
	assert(index_at >= 0 && index_at < array_len );
	return buf[index_at];
}

void IndexBuffer_OGL::set( int *in, int l ) {
	reserve(l);
	memcpy(buf, in, sizeof(int)*l);
}

void IndexBuffer_OGL::bless(){
	if( gl_name == 0 ){
		glGenBuffers(1, &gl_name);
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gl_name );
		// データがよく変わるときは GL_DYNAMIC_DRAWらしいけど、それはコンセプトから外れた使い方だからデフォルトはSTATICにしておく。
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * array_len, buf, GL_STATIC_DRAW );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	}
}

void IndexBuffer_OGL::dump() {
	print("ib: len:%d glname:%d", array_len, gl_name );
	if(buf){
		for(int i=0;i<array_len;i++) {
			print("[%d]=%d",i, buf[i] );
		}
	} else {
		print("ib:nobuf");
	}
}

#endif