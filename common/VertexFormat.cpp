#include "VertexFormat.h"

void VertexFormat::dump() {
	print("vfmt: types_used:%d num_float:%d coord_ofs:%d color_ofs:%d tex_ofs:%d normal_ofs:%d",
		types_used, num_float, coord_offset, color_offset, texture_offset, normal_offset );
	for(int i=0;i<elementof(types);i++) {
		print("type[%d]=%c", i, types[i]);
	}
}