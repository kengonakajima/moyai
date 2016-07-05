#include "client.h"

#include "Mesh.h"

void Mesh::dump() {
	print("mesh: primtype:%d transparent:%d", prim_type, transparent );
	if(vb) vb->dump(10); else print("no-vb" );
	if(ib) ib->dump(10); else print("no-ib" );
}
