#pragma once

#include "Enums.h"
#include "Texture.h"
#include "Camera.h"
#include "TileDeck.h"
#include "DrawBatch.h"

class Renderable {
public:
	DIMENSION dimension;
	int priority;        

	bool visible;
	TileDeck *deck;
	float enfat_epsilon;
	int index;    
	Renderable() : dimension(DIMENSION_INVAL), priority(0), visible(true), deck(NULL), enfat_epsilon(0), index(-1) {
	}

	~Renderable() {}

	inline void setDeck( TileDeck *d ){
		deck = d;
		assert( d->cell_width > 0 );
		assert( d->cell_height > 0 );        
	}
	inline void setTexture( Texture *t ){
		assert(t->tex!=0);        
		TileDeck *d = new TileDeck(); // TODO: d leaks
		d->setTexture(t);
		int w,h;
		t->getSize(&w,&h);
		d->setSize( 1,1, w, h );
		deck = d;
		index = 0;
	}

	inline void setVisible(bool flg){ visible = flg; }
	inline bool getVisible() { return visible; }
    inline void setPriority(int prio) { priority = prio; }
    inline int getPriority() { return priority; }

	inline void swapPriority( Renderable *target ) {
		int p = priority;
		priority = target->priority;
		target->priority = p;
	}
	inline void ensureFront( Renderable *target ) {
		if( target->priority > priority ) {
			swapPriority(target);
		}
	}
	inline void ensurePriority( Renderable *lower ) {
		if( priority >= lower->priority ) {
			return;
		} else {
			int tmp = priority;
			priority = lower->priority;
			lower->priority = tmp;
		}
	}

	virtual void render(Camera *cam, DrawBatchList *bl ){};
};
