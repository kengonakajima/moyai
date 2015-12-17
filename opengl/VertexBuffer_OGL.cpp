#ifdef USE_OPENGL

#include "VertexBuffer_OGL.h"

void VertexBuffer_OGL::reserve(int cnt){
	assertmsg(fmt, "vertex format is not set" );
	array_len = cnt;
	unit_num_float = fmt->getNumFloat();
	total_num_float = array_len * unit_num_float;
	buf = (float*)MALLOC( total_num_float * sizeof(float));
	assert(buf);
}

void VertexBuffer_OGL::copyFromBuffer( float *v, int vert_cnt ) {
	assertmsg( unit_num_float > 0, "call setFormat() before this." );
	assertmsg( vert_cnt <= array_len, "size too big");
	array_len = vert_cnt;
	total_num_float = vert_cnt * unit_num_float;
	memcpy( buf, v, vert_cnt * unit_num_float * sizeof(float) );
}
void VertexBuffer_OGL::setCoord( int index, Vec3 v ) {
	assertmsg(fmt, "vertex format is not set" );
	assertmsg( index < array_len, "invalid index:%d array_len:%d", index, array_len );
	int ofs = fmt->coord_offset;
	assertmsg( ofs >= 0, "coord have not declared in vertex format" );
	int index_in_array = index * unit_num_float + ofs;
	buf[index_in_array] = v.x;
	buf[index_in_array+1] = v.y;
	buf[index_in_array+2] = v.z;
}

Vec3 VertexBuffer_OGL::getCoord( int index ) {
	assertmsg(fmt, "vertex format is not set" );
	assert( index < array_len );
	int ofs = fmt->coord_offset;
	assertmsg( ofs >= 0, "coord have not declared in vertex format" );
	int index_in_array = index * unit_num_float + ofs;
	return Vec3( buf[index_in_array], buf[index_in_array+1], buf[index_in_array+2] );
}

void VertexBuffer_OGL::setCoordBulk( Vec3 *v, int num ) {
	for(int i=0;i<num;i++) {
		setCoord( i, v[i] );
	}
}
void VertexBuffer_OGL::setColor( int index, Color c ) {
	assertmsg(fmt, "vertex format is not set");
	assert( index < array_len );
	int ofs = fmt->color_offset;
	assertmsg( ofs >= 0, "color have not declared in vertex format");
	int index_in_array = index * unit_num_float + ofs;
	buf[index_in_array] = c.r;
	buf[index_in_array+1] = c.g;
	buf[index_in_array+2] = c.b;
	buf[index_in_array+3] = c.a;        
}

Color VertexBuffer_OGL::getColor( int index ) {
	assertmsg(fmt, "vertex format is not set" );
	assert( index < array_len );
	int ofs = fmt->color_offset;
	assertmsg( ofs >= 0, "color have not declared in vertex format" );
	int index_in_array = index * unit_num_float + ofs;
	return Color( buf[index_in_array], buf[index_in_array+1], buf[index_in_array+2], buf[index_in_array+3] );    
}

void VertexBuffer_OGL::setUV( int index, Vec2 uv ) {
	assertmsg(fmt, "vertex format is not set");
	assert( index < array_len );
	int ofs = fmt->texture_offset;
	assertmsg( ofs >= 0, "texcoord have not declared in vertex format");
	int index_in_array = index * unit_num_float + ofs;
	buf[index_in_array] = uv.x;
	buf[index_in_array+1] = uv.y;
}

Vec2 VertexBuffer_OGL::getUV( int index ) {
	assertmsg(fmt, "vertex format is not set" );
	assert( index < array_len );
	int ofs = fmt->texture_offset;
	assertmsg( ofs >= 0, "texuv have not declared in vertex format" );
	int index_in_array = index * unit_num_float + ofs;
	return Vec2( buf[index_in_array], buf[index_in_array+1] );
}

void VertexBuffer_OGL::setUVBulk( Vec2 *uv, int num ) {
	for(int i=0;i<num;i++) {
		setUV( i, uv[i] );
	}
}

void VertexBuffer_OGL::setNormal( int index, Vec3 v ) { 
	assertmsg(fmt, "vertex format is not set");
	assert( index < array_len );
	int ofs = fmt->normal_offset;
	assertmsg( ofs >= 0, "normal have not declared in vertex format" );
	int index_in_array = index * unit_num_float + ofs;
	buf[index_in_array] = v.x;
	buf[index_in_array+1] = v.y;
	buf[index_in_array+2] = v.z;        
}

Vec3 VertexBuffer_OGL::getNormal( int index ) {
	assertmsg(fmt, "vertex format is not set" );
	assert( index < array_len );
	int ofs = fmt->normal_offset;
	assertmsg( ofs >= 0, "normal have not declared in vertex format" );
	int index_in_array = index * unit_num_float + ofs;
	return Vec3( buf[index_in_array], buf[index_in_array+1], buf[index_in_array+2] );    
}

void VertexBuffer_OGL::setNormalBulk( Vec3 *v, int num ) {
	for(int i=0;i<num;i++) {
		setNormal( i, v[i] );
	}
}

void VertexBuffer_OGL::bless(){
	assert(fmt);
	if( gl_name == 0 ){
		glGenBuffers(1, &gl_name);
		glBindBuffer( GL_ARRAY_BUFFER, gl_name );
		glBufferData( GL_ARRAY_BUFFER, total_num_float * sizeof(float), buf, GL_STATIC_DRAW );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
	}
}

Vec3 VertexBuffer_OGL::calcCenterOfCoords() {
	Vec3 c(0,0,0);
	for(int i=0;i<array_len;i++) {
		c += getCoord(i);
	}
	c /= (float)array_len;
	return c;
}

void VertexBuffer_OGL::dump() {
	print("vb: len:%d nfloat:%d unitfloat:%d glname:%d", array_len, total_num_float, unit_num_float, gl_name );
	if(fmt) fmt->dump(); else print("vb:nofmt");
	if(buf) {
		for(int i=0;i<array_len;i++){
			for(int j=0;j<unit_num_float;j++){
				print("[%d][%d]=%.10f", i, j, buf[i*unit_num_float+j]);
			}
		}
	}
}

#endif
