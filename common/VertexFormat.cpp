#include "VertexFormat.h"

VertexFormat::VertexFormat() 
	: types_used(0)
	, num_float(0)
	, coord_offset(-1)
	, color_offset(-1)
	, texture_offset(-1)
	, normal_offset(-1) 
	, pos_offset(-1)
	, instanceFloatCount(0)
	, instanceElementCount(0)
{
	for(unsigned int i=0;i<elementof(types);i++){
		types[i] = 0;
	}

	memset(instanceElements, 0, sizeof(instanceElements));
}

void VertexFormat::dump() {
	print("vfmt: types_used:%d num_float:%d coord_ofs:%d color_ofs:%d tex_ofs:%d normal_ofs:%d pos_ofs:%d",
		types_used, num_float, coord_offset, color_offset, texture_offset, normal_offset, pos_offset );
	for(int i=0;i<elementof(types);i++) {
		print("type[%d]=%c", i, types[i]);
	}
}

bool VertexFormat::addInstanceElement(VertexSemantic semantic, unsigned int index, unsigned int floatSize)
{
	assertmsg(instanceElementCount < MaxInstanceElementCount, "Cannot add any more instance element");

	if (instanceElementCount < MaxInstanceElementCount)
	{
		instanceFloatCount += floatSize;
		Element &element = instanceElements[instanceElementCount++];
		element.floatSize = floatSize;
		element.semantic = semantic;
		element.semanticIndex = index;

		return true;
	}

	return false;
}

const VertexFormat::Element* VertexFormat::getInstanceElement(int index) const
{
	assertmsg(index < instanceElementCount, "Invalid index");

	if (index < instanceElementCount)
	{
		return &instanceElements[index];
	}

	return nullptr;
}
