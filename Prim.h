#pragma once

#include "common.h"
#include "client.h"

#include "Enums.h"
#include "Viewport.h"
#include "DrawBatch.h"

class Prim {
public:
    int id;
	PRIMTYPE type;
	Vec2 a,b;
	Color color;
	int line_width;
    static int idgen;
    
	Prim( PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width = 1 );
    ~Prim() {
    }
	void draw( Viewport *vp, DrawBatchList *bl, Vec2 tr, Vec2 scl, float radrot );
};

