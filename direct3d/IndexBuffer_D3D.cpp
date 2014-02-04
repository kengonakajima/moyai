#ifdef USE_D3D

#include "IndexBuffer_D3D.h"
#include "../cumino.h"

IndexBuffer_D3D::~IndexBuffer_D3D() 
{
	assert(buf);
	FREE(buf);
	//glDeleteBuffers(1,&gl_name);
}

void IndexBuffer_D3D::reserve( int len ) 
{
	if(buf) FREE(buf);    
	buf = (int*) MALLOC( sizeof(int) * len );
	assert(buf);
	array_len = len;
}

void IndexBuffer_D3D::setIndex( int index_at, int val ) 
{
	assert(buf);
	assert(index_at >= 0 && index_at < array_len );
	buf[index_at] = val;
}

int IndexBuffer_D3D::getIndex( int index_at ) 
{
	assert(buf);
	assert(index_at >= 0 && index_at < array_len );
	return buf[index_at];
}

void IndexBuffer_D3D::set( int *in, int l ) 
{
	reserve(l);
	memcpy(buf, in, sizeof(int)*l);
}

void IndexBuffer_D3D::bless()
{
	
}

void IndexBuffer_D3D::dump() 
{
	print("ib: len:%d glname:%d", array_len, gl_name );

	if(buf)
	{
		for(int i=0;i<array_len;i++) 
		{
			print("[%d]=%d",i, buf[i] );
		}
	} 
	else 
	{
		print("ib:nobuf");
	}
}

#endif