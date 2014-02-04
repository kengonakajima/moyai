#pragma once

#include "Context_D3D.h"

class IndexBuffer_D3D 
{

public:

	int *buf;
	int array_len;
	UINT gl_name;

	IndexBuffer_D3D() : buf(0), array_len(0), gl_name(0) {}
	~IndexBuffer_D3D();

	void reserve(int l );
	void setIndex( int index, int i );
	int getIndex( int index );
	void set( int *in, int l );
	void bless();
	void dump();
};