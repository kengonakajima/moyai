#pragma once

class AnimCurve {
public:
	int index_table[32];
	int index_used;
	float step_time;
	bool loop;
    int loop_to;
	AnimCurve( float st, bool do_loop, int *inds, int indslen ){
		assertmsg( indslen <= elementof(index_table), "animcurve length %d is onger than %d : ", indslen, elementof(index_table) );
		for(int i=0;i<indslen;i++){
			index_table[i] = inds[i];
		}
		index_used = indslen;
		step_time = st;
		loop = do_loop;
        loop_to = 0;
	}
    void setLoopTo( int ind ) { loop_to =  ind; }

	// t: アニメーション開始からの時間
	inline int getIndex( double t , bool *finished = NULL ){
		if(t<0) t=0;
		if( step_time == 0 ) {
			assert( index_used >= 0 );
			return index_table[0];
		}

		int ind = (int)(t / step_time);
		assert(ind>=0);
		if(finished) *finished = false;
		if(loop){
            if(loop_to == 0 || ind < loop_to ) {
                return index_table[ ind % index_used ];
            } else {
                int mod =  index_used - loop_to;
                return index_table[ loop_to + (ind % mod) ];
            }
		} else {
			if( ind >= index_used ){
				if(finished) *finished = true;
				return index_table[ index_used - 1];
			} else {
				return index_table[ ind ];
			}
		}
	}
	inline int getDefaultIndex() {
		assert( index_used > 0 );
		return index_table[0];
	}
};
