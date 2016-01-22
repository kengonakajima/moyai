#include "Mesh.h"

void Mesh::dump() {
	print("mesh: primtype:%d transparent:%d", prim_type, transparent );
	if(vb) vb->dump(); else print("no-vb" );
	if(ib) ib->dump(); else print("no-ib" );
}