#pragma once

#include "FragmentShader.h"
#include "common.h"

class ColorReplacerShader : public FragmentShader {
public:
	float epsilon;
	Color from_color;
	Color to_color;
	ColorReplacerShader() : epsilon(0), from_color(0,0,0,0), to_color(0,0,0,0){};
	bool init();
	void setColor( Color from, Color to, float eps ) {
		epsilon = eps;
		to_color = to;
		from_color = from;
	}
	virtual void updateUniforms();
};
