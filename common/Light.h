#pragma once

class Light {
public:
	Vec3 pos;
	Color diffuse;
	Color ambient;
	Color specular;
	Light() : pos(0,0,0), diffuse(1,1,1,1), ambient(0,0,0,1), specular(0,0,0,0) {
	}
};