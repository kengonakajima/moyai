#ifdef USE_OPENGL
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

#include "Prop2D_OGL.h"
#include "../common/FragmentShader.h"
#include "../common/AnimCurve.h"

bool Prop2D_OGL::propPoll(double dt) {
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
		Prop2D_OGL *p = children[i];
		p->basePoll(dt);
	}

	return true;
}

void Prop2D_OGL::drawIndex( TileDeck *dk, int ind, float minx, float miny, float maxx, float maxy, bool hrev, bool vrev, float uofs, float vofs, bool uvrot, float radrot ) {

	float u0,v0,u1,v1;
	dk->getUVFromIndex(ind, &u0, &v0, &u1, &v1, uofs, vofs, tex_epsilon );
	float depth = 10;

	if(debug_id) print("UV: ind:%d %f,%f %f,%f uo:%f vo:%f", ind, u0,v0, u1,v1, uofs, vofs );

	if(hrev){
		float tmp = u1;
		u1 = u0;
		u0 = tmp;
	}
	if(vrev){
		float tmp = v1;
		v1 = v0;
		v0 = tmp;
	}

	// if not rot 
	// B-----C       A:min C:max
	// |     |
	// A-----D
	//
	// if rot
	Vec2 a,b,c,d;
	if(rot==0){
		if(uvrot){
			a = Vec2( maxx, miny );
			b = Vec2( minx, miny );
			c = Vec2( minx, maxy );
			d = Vec2( maxx, maxy );
		} else {
			a = Vec2( minx, miny );
			b = Vec2( minx, maxy );
			c = Vec2( maxx, maxy );
			d = Vec2( maxx, miny );
		}
	} else {
		float sn = sin(radrot);
		float cs = cos(radrot);
		float center_x = (minx+maxx)/2.0f;
		float center_y = (miny+maxy)/2.0f;
		minx -= center_x;
		miny -= center_y;
		maxx -= center_x;
		maxy -= center_y;

		if(uvrot){
			a = Vec2( maxx * cs - miny * sn, maxx * sn + miny * cs );
			b = Vec2( minx * cs - miny * sn, minx * sn + miny * cs );
			c = Vec2( minx * cs - maxy * sn, minx * sn + maxy * cs );
			d = Vec2( maxx * cs - maxy * sn, maxx * sn + maxy * cs );
		} else {
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

	// counter clockwise
	glTexCoord2f(u0,v1); glVertex3i( a.x, a.y, depth );
	glTexCoord2f(u1,v1); glVertex3i( d.x, d.y, depth );
	glTexCoord2f(u1,v0); glVertex3i( c.x, c.y, depth );
	glTexCoord2f(u0,v0); glVertex3i( b.x, b.y, depth ); 

}

void Prop2D_OGL::render(Camera *cam) {
	assertmsg(deck || grid_used_num > 0 || children_num > 0 || prim_drawer , "no deck/grid/prim_drawer is set. deck:%p grid:%d child:%d prim:%p", deck, grid_used_num, children_num, prim_drawer );
	float camx=0.0f;
	float camy=0.0f;
	if(cam){
		camx = cam->loc.x * -1;
		camy = cam->loc.y * -1;
	}

	if( children_num > 0 && render_children_first ){
		for(int i=0;i<children_num;i++){
			Prop2D_OGL *p = (Prop2D_OGL*) children[i];
            if( p->visible ) p->render( cam );
		}
	}

	// TODO: use vbo for grids
	if( grid_used_num > 0 ){
		glEnable(GL_TEXTURE_2D);
		glColor4f(color.r,color.g,color.b,color.a);        
		for(int i=0;i<grid_used_num;i++){
			Grid *grid = grids[i];
			if(!grid)break;
			if(!grid->visible)continue;

			TileDeck *draw_deck;
			if( grid->deck ){
				draw_deck = grid->deck;
				glBindTexture( GL_TEXTURE_2D, grid->deck->tex->tex );                
			} else {
				assertmsg( deck, "need deck when grid has no deck" );
				draw_deck = deck;
				glBindTexture( GL_TEXTURE_2D, deck->tex->tex );
			}
			if( grid->fragment_shader ){
				glUseProgram(grid->fragment_shader->program );
				grid->fragment_shader->updateUniforms();
			}
			glBegin(GL_QUADS);

			for(int y=0;y<grid->height;y++){
				for(int x=0;x<grid->width;x++){
					int ind = grid->get(x,y);
					if( ind != Grid::GRID_NOT_USED ){
						int ti = x + y * grid->width;

						bool yflip=false, xflip=false, uvrot=false;
						if( grid->xflip_table ) xflip = grid->xflip_table[ti];
						if( grid->yflip_table ) yflip = grid->yflip_table[ti];
						if( grid->rot_table ) uvrot = grid->rot_table[ti]; 
						float texofs_x=0, texofs_y=0;
						if( grid->texofs_table ){
							texofs_x = grid->texofs_table[ti].x;
							texofs_y = grid->texofs_table[ti].y;                            
						}
						if( grid->color_table ){
							glColor4f( grid->color_table[ti].r,
								grid->color_table[ti].g,
								grid->color_table[ti].b,
								grid->color_table[ti].a                                       
								);
						}
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
							0 );
					}
				}
			}
			glEnd();            
			if(grid->fragment_shader){
				glUseProgram(0);
			}
		}
	}


	if(deck && index >= 0 ){
		glEnable(GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D, deck->tex->tex );
		if( fragment_shader ){
			glUseProgram( fragment_shader->program );
			fragment_shader->updateUniforms();
		}
		glBegin(GL_QUADS);
		glColor4f(color.r,color.g,color.b,color.a);

		float minx, miny, maxx, maxy;
		minx = camx + loc.x - scl.x/2 + draw_offset.x - enfat_epsilon;
		miny = camy + loc.y - scl.y/2 + draw_offset.y - enfat_epsilon;
		maxx = camx + loc.x + scl.x/2 + draw_offset.x + enfat_epsilon;
		maxy = camy + loc.y + scl.y/2 + draw_offset.y + enfat_epsilon;

		drawIndex( deck, index, minx, miny, maxx, maxy, xflip, yflip, 0,0, uvrot, rot );
		glEnd();
		if( fragment_shader ){
			glUseProgram(0);
		}
	}

	if( children_num > 0 && (render_children_first == false) ){
		for(int i=0;i<children_num;i++){
			Prop2D_OGL *p = (Prop2D_OGL*) children[i];
			if(p->visible) p->render( cam );
		}
	}

	// primitives should go over image sprites
	if( prim_drawer ){
		prim_drawer->drawAll(loc.add(camx,camy) );
	}
}

Prop *Prop2D_OGL::getNearestProp(){
	Prop *cur = parent_group->prop_top;
	float minlen = 999999999999999.0f;
	Prop *ans=0;
	while(cur){
		Prop2D_OGL *cur2d = (Prop2D_OGL*)cur;        
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

void Prop2D_OGL::updateMinMaxSizeCache(){
	max_rt_cache.x=0;
	max_rt_cache.y=0;
	min_lb_cache.x=0;
	min_lb_cache.y=0;

	float grid_max_x=0, grid_max_y=0;
	if(grid_used_num>0){
		for(int i=0;i<grid_used_num;i++){
			Grid *g = grids[i];
			float maxx = g->width * scl.x;
			if(maxx>grid_max_x )grid_max_x = maxx;
			float maxy = g->height * scl.y;
			if(maxy>grid_max_y )grid_max_y = maxy;
		}
	}
	if( grid_max_x > max_rt_cache.x ) max_rt_cache.x = grid_max_x;
	if( grid_max_y > max_rt_cache.y ) max_rt_cache.y = grid_max_y;

	//

	float child_max_x=0, child_max_y = 0;
	if( children_num > 0){
		for(int i=0;i<children_num;i++){
			Prop *p = children[i];
			float maxx = ((Prop2D_OGL*)p)->scl.x/2;
			float maxy = ((Prop2D_OGL*)p)->scl.y/2;
			if( maxx > child_max_x ) child_max_x = maxx;
			if( maxy > child_max_y ) child_max_y = maxy;
		}
	}
	if( child_max_x > max_rt_cache.x ) max_rt_cache.x = child_max_x;
	if( child_max_y > max_rt_cache.y ) max_rt_cache.y = child_max_y;

	if( prim_drawer ){
		Vec2 minv, maxv;
		prim_drawer->getMinMax( &minv, &maxv );
		if( minv.x < min_lb_cache.x ) min_lb_cache.x = minv.x;
		if( minv.y < min_lb_cache.y ) min_lb_cache.y = minv.y;
		if( maxv.x > max_rt_cache.x ) max_rt_cache.x = maxv.x;
		if( maxv.y > max_rt_cache.y ) max_rt_cache.y = maxv.y;
	}
}

#endif
