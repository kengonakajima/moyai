#ifdef USE_D3D

#include "Prop2D_D3D.h"
#include "../common/FragmentShader.h"
#include "../common/AnimCurve.h"
#include "VertexBuffer_D3D.h"

Prop2D_D3D::Prop2D_D3D()
	: Prop(), Renderable() 
	, m_pVertexBuffer(nullptr)
{
	priority = id;
	dimension = DIMENSION_2D;

	color = Color(1,1,1,1);

	children_num = 0;

	loc.x = loc.y = 0;
	draw_offset.x = draw_offset.y = 0;
	scl.x = scl.y = 32;
	anim_curve = NULL;

	anim_start_at = 0;
	grid_used_num = 0;

	xflip = yflip = uvrot = false;

	rot = 0;
	seek_scl_time = seek_scl_started_at = 0;
	seek_rot_time = seek_rot_started_at = 0;
	seek_color_time = seek_color_started_at = 0;

	fragment_shader = NULL;
	prim_drawer = NULL;
	max_rt_cache = Vec2(0,0);
	min_lb_cache = Vec2(0,0);

	tex_epsilon = 0;

	render_children_first = false;

	init();
}

Prop2D_D3D::~Prop2D_D3D()
{
	for(int i=0;i<grid_used_num;i++)
	{
		if(grids[i]) delete grids[i];
	}

	for(int i=0;i<children_num;i++)
	{
		if(children[i]) delete children[i];
	}        

	if(prim_drawer) delete prim_drawer;

	SafeDelete(m_pVertexBuffer);
}

void Prop2D_D3D::init()
{
	// Create vertex buffer
	{
		VertexFormat format;
		format.declareCoordVec3();
		format.declareColor();
		format.declareUV();

		m_pVertexBuffer = new VertexBuffer_D3D(format, 6, g_context.m_pShaderManager->GetShader(ShaderManager_D3D::SHADER_DEFAULT));
	}

	fragment_shader = g_context.m_pShaderManager->GetShader(ShaderManager_D3D::SHADER_DEFAULT);
}

void Prop2D_D3D::setParentGroup(Group *group)
{
	Prop::setParentGroup(group);
	
	for (int i = 0; i < children_num; ++i)
	{
		Prop2D_D3D *p = children[i];
		p->setParentGroup(group);
	}
}

Layer_D3D::RenderData& Prop2D_D3D::getNewRenderData()
{
	return static_cast<Layer_D3D*>(parent_group)->getNewRenderData();
}

bool Prop2D_D3D::propPoll(double dt) 
{
	if( prop2DPoll(dt) == false ) return false;

	// animation of index
	if(anim_curve){
		int previndex = index;
		bool finished = false;
		index = anim_curve->getIndex( accum_time - anim_start_at, &finished );
		if( index != previndex ){
			onIndexChanged(previndex);
		}
		if(finished) {
			onAnimFinished();
		}
	}
	// animation of scale
	if( seek_scl_time != 0 ){
		double elt = accum_time - seek_scl_started_at;
		if( elt > seek_scl_time ){
			scl = seek_scl_target;
			seek_scl_time = 0;
		} else {
			double rate = elt / seek_scl_time;
			scl.x = seek_scl_orig.x + ( seek_scl_target.x - seek_scl_orig.x ) * rate;
			scl.y = seek_scl_orig.y + ( seek_scl_target.y - seek_scl_orig.y ) * rate;
		}
	}
	// animation of rotation
	if( seek_rot_time != 0 ){
		double elt = accum_time - seek_rot_started_at;
		if( elt > seek_rot_time ){
			rot = seek_rot_target;
			seek_rot_time = 0;
		} else {
			double rate = elt / seek_rot_time;
			rot = seek_rot_orig + ( seek_rot_target - seek_rot_orig ) * rate;
		}
	}
	// animation of color
	if( seek_color_time != 0 ){
		double elt = accum_time - seek_color_started_at;
		if( elt > seek_color_time ){
			color = seek_color_target;
			seek_color_time = 0;
		} else {
			double rate = elt / seek_color_time;
			color = Color( seek_color_orig.r + ( seek_color_target.r - seek_color_orig.r ) * rate,
				seek_color_orig.g + ( seek_color_target.g - seek_color_orig.g ) * rate,
				seek_color_orig.b + ( seek_color_target.b - seek_color_orig.b ) * rate,
				seek_color_orig.a + ( seek_color_target.a - seek_color_orig.a ) * rate );
		}
	}

	// children
	for(int i=0;i<children_num;i++){
		Prop2D_D3D *p = children[i];
		p->basePoll(dt);
	}

	return true;
}

void Prop2D_D3D::drawIndex(TileDeck *dk, int ind, float minx, float miny, float maxx, float maxy, bool hrev, bool vrev, float uofs, float vofs, bool uvrot, float radrot, Layer_D3D::InstanceData &instanceData) 
{
	float u0,v0,u1,v1;
	dk->getUVFromIndex(ind, &u0, &v0, &u1, &v1, uofs, vofs, tex_epsilon );
	float depth = 10;

	instanceData.uvOffset.x = u0;
	instanceData.uvOffset.y = v0;
	instanceData.uvScale.x = u1 - u0;
	instanceData.uvScale.y = v1 - v0;

	if(debug_id) print("UV: ind:%d %f,%f %f,%f uo:%f vo:%f", ind, u0,v0, u1,v1, uofs, vofs );

	if(hrev)
	{
		float tmp = u1;
		u1 = u0;
		u0 = tmp;

		instanceData.uvScale.x = -instanceData.uvScale.x;
	}
	if(vrev)
	{
		float tmp = v1;
		v1 = v0;
		v0 = tmp;

		instanceData.uvScale.y = -instanceData.uvScale.y;
	}

	// if not rot 
	// B-----C       A:min C:max
	// |     |
	// A-----D
	//
	// if rot

	Vec2 a,b,c,d;
	float center_x = (minx+maxx)/2.0f;
	float center_y = (miny+maxy)/2.0f;
	instanceData.offset.x = center_x;
	instanceData.offset.y = center_y;
	instanceData.scale.x = maxx - minx;
	instanceData.scale.y = maxy - miny;
	instanceData.rotation = 0.0f;

	if(rot==0){
		if(uvrot){ // -pi/2 rotation
			instanceData.rotation = -M_PI_2;
			a = Vec2( maxx, miny );
			b = Vec2( minx, miny );
			c = Vec2( minx, maxy );
			d = Vec2( maxx, maxy );
		} else { // no rotation
			a = Vec2( minx, miny );
			b = Vec2( minx, maxy );
			c = Vec2( maxx, maxy );
			d = Vec2( maxx, miny );
		}
	} else {
		float sn = sin(radrot);
		float cs = cos(radrot);
		minx -= center_x;
		miny -= center_y;
		maxx -= center_x;
		maxy -= center_y;

		if(uvrot) // -pi/2 + radrot rotation
		{ 
			instanceData.rotation = -M_PI_2 + radrot;

			a = Vec2( maxx * cs - miny * sn, maxx * sn + miny * cs );
			b = Vec2( minx * cs - miny * sn, minx * sn + miny * cs );
			c = Vec2( minx * cs - maxy * sn, minx * sn + maxy * cs );
			d = Vec2( maxx * cs - maxy * sn, maxx * sn + maxy * cs );
		} 
		else // radrot rotation
		{ 
			instanceData.rotation = radrot;

			a = Vec2( minx * cs - miny * sn, minx * sn + miny * cs );
			b = Vec2( minx * cs - maxy * sn, minx * sn + maxy * cs );
			c = Vec2( maxx * cs - maxy * sn, maxx * sn + maxy * cs );
			d = Vec2( maxx * cs - miny * sn, maxx * sn + miny * cs );
		}

		a = a.add( center_x, center_y );
		b = b.add( center_x, center_y );
		c = c.add( center_x, center_y );
		d = d.add( center_x, center_y );                    
	}

	const UINT vertexCount = 6;
	Vertex_PCUV vertices[] = 
	{
		{ XMFLOAT3(a.x, a.y, depth), XMFLOAT2(u0, v1), XMFLOAT4(color.r, color.g, color.b, color.a) },
		{ XMFLOAT3(b.x, b.y, depth), XMFLOAT2(u0, v0), XMFLOAT4(color.r, color.g, color.b, color.a) },
		{ XMFLOAT3(c.x, c.y, depth), XMFLOAT2(u1, v0), XMFLOAT4(color.r, color.g, color.b, color.a) },

		{ XMFLOAT3(a.x, a.y, depth), XMFLOAT2(u0, v1), XMFLOAT4(color.r, color.g, color.b, color.a) },
		{ XMFLOAT3(c.x, c.y, depth), XMFLOAT2(u1, v0), XMFLOAT4(color.r, color.g, color.b, color.a) },
		{ XMFLOAT3(d.x, d.y, depth), XMFLOAT2(u1, v1), XMFLOAT4(color.r, color.g, color.b, color.a) }
	};

	/*
	m_pVertexBuffer->copyFromBuffer(vertices, vertexCount);
	m_pVertexBuffer->copyToGPU();
	m_pVertexBuffer->bind();

	g_context.m_pDeviceContext->Draw(6, 0);
	*/
}

void Prop2D_D3D::render(Camera *cam) 
{
	assertmsg(deck || grid_used_num > 0 || children_num > 0 || prim_drawer , "no deck/grid/prim_drawer is set ");

	Color currentColor;
	float camx=0.0f;
	float camy=0.0f;

	if(cam)
	{
		camx = cam->loc.x * -1;
		camy = cam->loc.y * -1;
	}

	if( children_num > 0 && render_children_first )
	{
		for(int i=0;i<children_num;i++)
		{
			Prop2D_D3D *p = (Prop2D_D3D*) children[i];
            if( p->visible ) p->render( cam );
		}
	}

	if( grid_used_num > 0 )
	{
		currentColor = color;

		for(int i=0;i<grid_used_num;i++)
		{
			Grid *grid = grids[i];
			if(!grid)break;
			if(!grid->visible)continue;

			TileDeck *draw_deck;
			FragmentShader_D3D *shader;
			Texture_D3D *texture;

			if( grid->deck )
			{
				draw_deck = grid->deck;
				texture = grid->deck->tex;

				grid->deck->tex->bind();
			} 
			else 
			{
				assertmsg( deck != nullptr, "need deck when grid has no deck" );
				draw_deck = deck;
				texture = deck->tex;

				deck->tex->bind();
			}

			assertmsg(grid->fragment_shader, "Grid needs a fragment shader assigned");
			shader = grid->fragment_shader;

			grid->fragment_shader->bind();
			grid->fragment_shader->updateUniforms();

			
			for(int y=0;y<grid->height;y++)
			{
				for(int x=0;x<grid->width;x++)
				{
					int ind = grid->get(x,y);
					if( ind != Grid::GRID_NOT_USED )
					{
						int ti = x + y * grid->width;
						bool yflip=false, xflip=false, uvrot=false;

						if( grid->xflip_table ) xflip = grid->xflip_table[ti];
						if( grid->yflip_table ) yflip = grid->yflip_table[ti];
						if( grid->rot_table ) uvrot = grid->rot_table[ti]; 

						float texofs_x=0, texofs_y=0;

						if( grid->texofs_table )
						{
							texofs_x = grid->texofs_table[ti].x;
							texofs_y = grid->texofs_table[ti].y;                            
						}

						if( grid->color_table )
						{
							currentColor = grid->color_table[ti];
						}

						Layer_D3D::RenderData &renderData = getNewRenderData();
						renderData.shader = shader;
						renderData.texture = texture;
						renderData.instanceData->color = currentColor;

						drawIndex( draw_deck,
							ind,
							camx + loc.x + x * scl.x + draw_offset.x - grid->enfat_epsilon,
							camy + loc.y + y * scl.y + draw_offset.y - grid->enfat_epsilon,
							camx + loc.x + (x+1) * scl.x + draw_offset.x + grid->enfat_epsilon,
							camy + loc.y + (y+1)*scl.y + draw_offset.y + grid->enfat_epsilon,
							xflip,
							yflip,
							texofs_x,
							texofs_y,
							uvrot,
							0,
							*renderData.instanceData);
					}
				}
			}
		}
	}


	if(deck && index >= 0 )
	{
		assertmsg(fragment_shader, "Prop2D needs a fragment shader assigned");
		currentColor = color;

		fragment_shader->bind();
		fragment_shader->updateUniforms();

		float minx, miny, maxx, maxy;
		minx = camx + loc.x - scl.x/2 + draw_offset.x - enfat_epsilon;
		miny = camy + loc.y - scl.y/2 + draw_offset.y - enfat_epsilon;
		maxx = camx + loc.x + scl.x/2 + draw_offset.x + enfat_epsilon;
		maxy = camy + loc.y + scl.y/2 + draw_offset.y + enfat_epsilon;

		Layer_D3D::RenderData &renderData = getNewRenderData();
		renderData.shader = fragment_shader;
		renderData.texture = deck->tex;
		renderData.instanceData->color = currentColor;

		drawIndex( deck, index, minx, miny, maxx, maxy, xflip, yflip, 0,0, uvrot, rot, *renderData.instanceData );
	}

	if( children_num > 0 && (render_children_first == false) )
	{
		for(int i=0;i<children_num;i++)
		{
			Prop2D_D3D *p = (Prop2D_D3D*) children[i];
			if( p->visible ) p->render( cam );
		}
	}

	// primitives should go over image sprites
	if( prim_drawer )
	{
		prim_drawer->drawAll(loc.add(camx,camy) );
	}
}

Prop *Prop2D_D3D::getNearestProp()
{
	Prop *cur = parent_group->prop_top;
	float minlen = 999999999999999.0f;
	Prop *ans=0;
	while(cur){
		Prop2D_D3D *cur2d = (Prop2D_D3D*)cur;        
		if( cur2d->dimension == DIMENSION_2D ) {
			float l = cur2d->loc.len(loc);
			if(l<minlen && cur != this ){
				ans = cur;
				minlen = l;
			}
		}
		cur = cur->next;
	}
	return ans;
}

void Prop2D_D3D::updateMinMaxSizeCache()
{
	max_rt_cache.x=0;
	max_rt_cache.y=0;
	min_lb_cache.x=0;
	min_lb_cache.y=0;

	float grid_max_x=0, grid_max_y=0;
	if(grid_used_num>0)
	{
		for(int i=0;i<grid_used_num;i++)
		{
			Grid *g = grids[i];
			float maxx = g->width * scl.x;
			if(maxx>grid_max_x )grid_max_x = maxx;
			float maxy = g->height * scl.y;
			if(maxy>grid_max_y )grid_max_y = maxy;
		}
	}

	if( grid_max_x > max_rt_cache.x ) max_rt_cache.x = grid_max_x;
	if( grid_max_y > max_rt_cache.y ) max_rt_cache.y = grid_max_y;

	float child_max_x=0, child_max_y = 0;
	if( children_num > 0)
	{
		for(int i=0;i<children_num;i++)
		{
			Prop *p = children[i];
			float maxx = ((Prop2D_D3D*)p)->scl.x/2;
			float maxy = ((Prop2D_D3D*)p)->scl.y/2;
			if( maxx > child_max_x ) child_max_x = maxx;
			if( maxy > child_max_y ) child_max_y = maxy;
		}
	}

	if( child_max_x > max_rt_cache.x ) max_rt_cache.x = child_max_x;
	if( child_max_y > max_rt_cache.y ) max_rt_cache.y = child_max_y;

	if( prim_drawer )
	{
		Vec2 minv, maxv;
		prim_drawer->getMinMax( &minv, &maxv );
		if( minv.x < min_lb_cache.x ) min_lb_cache.x = minv.x;
		if( minv.y < min_lb_cache.y ) min_lb_cache.y = minv.y;
		if( maxv.x > max_rt_cache.x ) max_rt_cache.x = maxv.x;
		if( maxv.y > max_rt_cache.y ) max_rt_cache.y = maxv.y;
	}
}

#endif
