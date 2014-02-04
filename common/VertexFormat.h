#pragma once

#include "../cumino.h"

class VertexFormat {
public:
	// float only
	char types[4]; // 'v': {f,f,f} 'c':{f,f,f,f}  't':{f,f}, 'n':{f,f,f} normal
	int types_used;
	int num_float;
	int coord_offset, color_offset, texture_offset, normal_offset; // -1:not used
	VertexFormat() : types_used(0), num_float(0), coord_offset(-1), color_offset(-1), texture_offset(-1), normal_offset(-1) {
		for(unsigned int i=0;i<elementof(types);i++){
			types[i] = 0;
		}
	}
	void declareCoordVec3(){ addType('v'); }
	void declareColor(){ addType('c'); }
	void declareUV(){ addType('t'); }
	void declareNormal(){ addType('n'); }
	void addType(char t){
		assertmsg( types_used < elementof(types), "too many types");
		types[types_used++] = t;
		updateSize();
	}
	bool isCoordVec3Declared( int index ) {
		assert(index>=0 && index<types_used);
		return types[index] == 'v';
	}
	bool isColorDeclared( int index ) {
		assert(index>=0 && index<types_used);
		return types[index] == 'c';
	}
	bool isUVDeclared( int index ) {
		assert(index>=0 && index<types_used);
		return types[index] == 't';
	}
	bool isNormalDeclared( int index ) {
		assert(index>=0 && index<types_used);
		return types[index] == 'n';
	}
	void updateSize(){
		num_float = 0;
		for(int i=0;i<types_used;i++){
			switch(types[i]){
			case 'v':
				coord_offset = num_float;
				num_float += 3;
				break;
			case 'n':
				normal_offset = num_float;
				num_float += 3;
				break;
			case 'c':
				color_offset = num_float;
				num_float += 4;
				break;
			case 't':
				texture_offset = num_float;
				num_float += 2;
				break;
			default:
				assertmsg( false, "vertexformat: updateSize: invalid type name: '%c'", types[i]);
			}
		}        
	};
	inline size_t getNumFloat() {
		return num_float;
	}
	void dump();
};