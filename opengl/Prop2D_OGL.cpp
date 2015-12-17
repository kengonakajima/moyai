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
VertexFormat *Prop2D_OGL::vf_single_sprite = NULL;
VertexFormat *Prop2D_OGL::getVertexFormat() {
    if( !vf_single_sprite ) {
        vf_single_sprite = new VertexFormat();
        vf_single_sprite->declareCoordVec3(); 
        vf_single_sprite->declareColor();
        vf_single_sprite->declareUV();
    }
    return vf_single_sprite;
}

void Prop2D_OGL::render(Camera *cam) {
	assertmsg(deck || grid_used_num > 0 || children_num > 0 || prim_drawer , "no deck/grid/prim_drawer is set. deck:%p grid:%d child:%d prim:%p", deck, grid_used_num, children_num, prim_drawer );

    if( use_additive_blend ) glBlendFunc(GL_ONE, GL_ONE ); else glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    
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
#if 1 // glDrawElements version
        if(!mesh) {
            print("new mesh!");
            mesh = new Mesh();
            VertexFormat *vf = getVertexFormat();
            VertexBuffer *vb = new VertexBuffer();
            vb->setFormat(vf);
            vb->reserve(4);
            // 頂点は4、三角形は2個なのでindexは6個
            // C --- D
            // |     |
            // A --- B
            float d = 0.5;
            vb->setCoord(0, Vec3(-d,-d,0)); // A
            vb->setCoord(1, Vec3(d,-d,0)); // B
            vb->setCoord(2, Vec3(-d,d,0)); // C
            vb->setCoord(3, Vec3(d,d,0)); // D
            vb->setColor(0, Color(1,1,1,1) ); // A
            vb->setColor(1, Color(1,1,1,1) ); // B
            vb->setColor(2, Color(1,1,1,1) ); // C
            vb->setColor(3, Color(1,1,1,1) ); // D
            updateUV(vb);

            IndexBuffer *ib = new IndexBuffer();
            static int inds[6] = {
                0,2,1, // ACB
                1,2,3, // BCD
            };
            ib->set(inds,6);
            mesh->setVertexBuffer(vb);
            mesh->setIndexBuffer(ib);
            mesh->setPrimType(GL_TRIANGLES);
        } else {
            if( index_changed ) {
                updateUV( mesh->vb );
                index_changed = false;
            }
        }
        if(deck) {
            glEnable(GL_TEXTURE_2D);
            if(deck->tex->tex) {
                glBindTexture( GL_TEXTURE_2D, deck->tex->tex );
            }
        } else {
            print("no tex?");
            glDisable(GL_TEXTURE_2D);
        }
        mesh->vb->bless();
        assert( mesh->vb->gl_name > 0 );        
        mesh->ib->bless();
        assert( mesh->ib->gl_name > 0 );
        
        int vert_sz = mesh->vb->fmt->getNumFloat() * sizeof(float);
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->ib->gl_name );
        glBindBuffer( GL_ARRAY_BUFFER, mesh->vb->gl_name );
        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_COLOR_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        glDisableClientState( GL_NORMAL_ARRAY );
        
        // 以下prop3dからのコピペ、動いたら共通化する
        if( mesh->vb->fmt->coord_offset >= 0 ){
            glEnableClientState( GL_VERTEX_ARRAY );        
            glVertexPointer( 3, GL_FLOAT, vert_sz, (char*)0 + mesh->vb->fmt->coord_offset * sizeof(float) );
        }
        if( mesh->vb->fmt->color_offset >= 0 ){
            glEnableClientState( GL_COLOR_ARRAY );
            glColorPointer( 4, GL_FLOAT, vert_sz, (char*)0 + mesh->vb->fmt->color_offset * sizeof(float));
        }
        if( mesh->vb->fmt->texture_offset >= 0 ){
            glEnableClientState( GL_TEXTURE_COORD_ARRAY );                    
            glTexCoordPointer( 2, GL_FLOAT, vert_sz, (char*)0 + mesh->vb->fmt->texture_offset * sizeof(float) );
        }
        if( mesh->vb->fmt->normal_offset >= 0 ) {
            glEnableClientState( GL_NORMAL_ARRAY );
            glNormalPointer( GL_FLOAT, vert_sz, (char*)0 + mesh->vb->fmt->normal_offset * sizeof(float) );
        }

		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
        
        glLoadIdentity();    

        glTranslatef( loc.x, loc.y, 0 );
        glRotatef( rot, 0,0,1);
        //	if( rot->x != 0 ) glRotatef( rot->x, 1,0,0);     
        //	if( rot->y != 0 ) glRotatef( rot->y, 0,1,0);     
        //	if( rot->z != 0 ) glRotatef( rot->z, 0,0,1);
        glScalef( scl.x, scl.y, 1 );

        assert( mesh->prim_type == GL_TRIANGLES );

        glDrawElements( mesh->prim_type, mesh->ib->array_len, GL_UNSIGNED_INT, 0);
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        
#endif
        
#if 0 // glBegin version
		glBegin(GL_QUADS);
		glColor4f(color.r,color.g,color.b,color.a);

		float minx, miny, maxx, maxy;
		minx = camx + loc.x - scl.x/2 + draw_offset.x - enfat_epsilon;
		miny = camy + loc.y - scl.y/2 + draw_offset.y - enfat_epsilon;
		maxx = camx + loc.x + scl.x/2 + draw_offset.x + enfat_epsilon;
		maxy = camy + loc.y + scl.y/2 + draw_offset.y + enfat_epsilon;

		drawIndex( deck, index, minx, miny, maxx, maxy, xflip, yflip, 0,0, uvrot, rot );
		glEnd();
#endif        
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

void Prop2D_OGL::updateUV(VertexBuffer *vb ) {
    vb->unbless();
    float u0,v0,u1,v1;
    deck->getUVFromIndex(index, &u0, &v0, &u1, &v1, 0, 0, 0 );
    vb->setUV(0, Vec2(u0,v1)); // A
    vb->setUV(1, Vec2(u1,v1)); // B
    vb->setUV(2, Vec2(u0,v0)); // C
    vb->setUV(3, Vec2(u1,v0)); // D
}

#endif
