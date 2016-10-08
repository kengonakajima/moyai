#pragma once

#include "common.h"
#include "client.h"

class Keyboard;
class Pad {
public:
	bool up, down, left, right;

	Pad() : up(false), down(false), left(false), right(false) {
	}
    void readKeyboard(Keyboard *kbd);
	void getVec( Vec2 *v ){
		float dx=0,dy=0;
		if( up ) dy=1.0;
		if( down ) dy=-1.0;
		if( right ) dx=1.0;
		if( left ) dx=-1.0;
		if(dx!=0 || dy!=0){
			normalize( &dx, &dy, 1 );
		}
		v->x = dx;
		v->y = dy;
	}
    DIR4 getDir() {
        if(up) return DIR4_UP; else if(down) return DIR4_DOWN; else if(right) return DIR4_RIGHT; else if(left) return DIR4_LEFT; else return DIR4_NONE;
    }
    void setUpKey( bool u) { up = u; }
    void setDownKey( bool d) { down = d; }
    void setLeftKey( bool l ) { left = l; }
    void setRightKey( bool r ) { right = r; }
};
