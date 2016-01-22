#pragma once

#include "AnimCurve.h"

class Animation {
public:
	AnimCurve *curves[32];
	Animation() {
		memset( curves, 0, sizeof(curves));
	}
	void reg( int index, AnimCurve *cv ) {
		assertmsg( curves[index] == NULL, "can't register anim curve twice" ); 
		curves[index] = cv;
	}

	int getIndex( int curve_ind, double start_at, double t, bool *finished  ) {
		assert( curve_ind >= 0 && curve_ind < elementof(curves) ); 
		AnimCurve *ac = curves[curve_ind];
		if(!ac) return 0;
		return ac->getIndex( t - start_at, finished );
	}
    AnimCurve *getCurve(int index) {
        return curves[index];
    }    
};
