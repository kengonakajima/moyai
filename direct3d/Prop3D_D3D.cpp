#ifdef USE_D3D

#include "Prop3D_D3D.h"
#include "../common/Layer.h"


Prop3D_D3D::Prop3D_D3D() 
	: Prop()
	, Renderable()
	, loc(0,0,0)
	, scl(1,1,1)
	, rot(0,0,0)
	, mesh(nullptr)
	, children(nullptr)
	, children_num(0)
	, children_max(0)
	, material(nullptr)
	, sort_center(0,0,0)
	, skip_rot(false)
	, billboard_index(-1)
	, fragment_shader(nullptr)
	, depth_mask(true)
	, alpha_test(false)
	, cull_back_face(true)
	, draw_offset(0,0,0) 
{
	priority = id;        
	dimension = DIMENSION_3D;
}

Prop3D_D3D::~Prop3D_D3D()
{

}

void Prop3D_D3D::setMaterialChildren( Material *mat ) 
{        
	for(int i=0;i<children_num;i++) 
	{
		if( children[i] ) 
		{
			children[i]->setMaterial(mat);
		}
	}
}    

Vec2 Prop3D_D3D::getScreenPos() 
{
	Layer *l = (Layer*) parent_group;
	return l->getScreenPos(loc);
}

int Prop3D_D3D::countSpareChildren() 
{
	int cnt=0;
	for(int i=0;i<children_num;i++) 
	{
		if( children[i] == NULL ) cnt ++;
	}

	return cnt;
}

void Prop3D_D3D::reserveChildren( int n ) 
{
	assert( n <= CHILDREN_ABS_MAX );
	size_t sz = sizeof(Prop3D_D3D*) * n;

	if( children ) 
	{
		Prop3D_D3D **newptr = (Prop3D_D3D**) MALLOC( sz );

		for(int i=0;i<n;i++) newptr[i] = NULL;        

		for(int i=0;i<children_max;i++) 
		{
			newptr[i] = children[i];
		}

		FREE(children);
		children = newptr;
	} 
	else 
	{
		children = (Prop3D_D3D**) MALLOC( sz );
		for(int i=0;i<n;i++) children[i] = NULL;
	}

	children_max = n;
}

void Prop3D_D3D::addChild( Prop3D_D3D *p ) 
{
	assert(p);
	assert( children_num < children_max );
	children[children_num] = p;
	children_num ++;
}

void Prop3D_D3D::deleteChild( Prop3D_D3D *p ) 
{
	assert(p);
	for(int i=0;i<children_num;i++) 
	{
		if( children[i] == p ) 
		{
			for(int j=i+1;j<children_num;j++) 
			{
				children[j-1] = children[j];
			}

			children_num --;
			return;
		}
	}
}

bool Prop3D_D3D::propPoll(double dt) 
{    
	if( prop3DPoll(dt) == false ) return false;

	if( children_num > 0 ) 
	{ 
		// children
		for(int i=0;i<children_num;i++)
		{
			Prop3D_D3D *p = children[i];
			p->basePoll(dt);
		}
	}

	return true;
}

#endif