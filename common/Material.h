#pragma once

class Material {
public:
	Color diffuse;
	Color ambient;
	Color specular;
	Material() : diffuse(1,1,1,1), ambient(0,0,0,0), specular(0,0,0,0) {}
};