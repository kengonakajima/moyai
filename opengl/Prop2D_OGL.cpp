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
				draw_deck = deck;
				glBindTexture( GL_TEXTURE_2D, deck->tex->tex );
			}
			if( grid->fragment_shader ){
				glUseProgram(grid->fragment_shader->program );
				grid->fragment_shader->updateUniforms();
			}

            if(!grid->mesh) {
                print("new grid mesh! wh:%d,%d", grid->width, grid->height );
                grid->mesh = new Mesh();
                VertexFormat *vf = getVertexFormat();
                IndexBuffer *ib = new IndexBuffer();
                VertexBuffer *vb = new VertexBuffer();
                vb->setFormat(vf);
                /*
                  3+--+--+--+--+
                   |  |  |  |  |
                  2+--+--+--+--+
                   |  |  |  |  |
                  1+--+--+--+--+
                   |  |  |  |  |
                  0+--+--+--+--+
                   0  1  2  3  4
                 */
                int quad_num = grid->width * grid->height;
                int triangle_num = quad_num * 2;
                int vert_num = quad_num * 4; // Can't share vertices because each vert has different UVs 
                vb->reserve( vert_num);
                ib->reserve( triangle_num*3 );
                grid->mesh->setVertexBuffer(vb);
                grid->mesh->setIndexBuffer(ib);
                grid->mesh->setPrimType( GL_TRIANGLES );

                grid->uv_changed = true;
                grid->color_changed = true;
            }

            if( grid->uv_changed || grid->color_changed ) {
                VertexBuffer *vb = grid->mesh->vb;
                IndexBuffer *ib = grid->mesh->ib;
                vb->unbless();

                int quad_cnt=0;
                for(int y=0;y<grid->height;y++) {
                    for(int x=0;x<grid->width;x++) {
                        int ind = x+y*grid->width;
                        if( grid->index_table[ind] == Grid::GRID_NOT_USED ) continue;
                        
                        Vec2 left_bottom, right_top;
                        float u0,v0,u1,v1;
                        draw_deck->getUVFromIndex( grid->index_table[ind], &u0,&v0,&u1,&v1,0,0,0);
                        if(grid->texofs_table) {
                            u0 += grid->texofs_table[ind].x;
                            v0 += grid->texofs_table[ind].y;
                            u1 += grid->texofs_table[ind].x;
                            v1 += grid->texofs_table[ind].y;
                        }

                        //
                        // Q (u0,v0) - R (u1,v0)      top-bottom upside down.
                        //      |           |
                        //      |           |                        
                        // P (u0,v1) - S (u1,v1)
                        //
                        if(grid->xflip_table && grid->xflip_table[ind]) {
                            swapf( &u0, &u1 );
                        }
                        if(grid->yflip_table && grid->yflip_table[ind]) {
                            swapf( &v0, &v1 );
                        }
                        Vec2 uv_p(u0,v1), uv_q(u0,v0), uv_r(u1,v0), uv_s(u1,v1);

                        
                        // left bottom
                        const float d = 1;
                        int vi = quad_cnt * 4;
                        vb->setCoord(vi,Vec3(d*x,d*y,0));
                        if(grid->rot_table && grid->rot_table[ind]) vb->setUV(vi,uv_s); else vb->setUV(vi,uv_p);
                        if(grid->color_table) vb->setColor(vi, grid->color_table[ind]); else vb->setColor(vi, Color(1,1,1,1));
                        // right bottom
                        vb->setCoord(vi+1,Vec3(d*(x+1),d*y,0));
                        if(grid->rot_table && grid->rot_table[ind]) vb->setUV(vi+1,uv_r); else vb->setUV(vi+1,uv_s);                        
                        if(grid->color_table) vb->setColor(vi+1,grid->color_table[ind]); else vb->setColor(vi+1, Color(1,1,1,1));
                        // left top
                        vb->setCoord(vi+2,Vec3(d*x,d*(y+1),0));
                        if(grid->rot_table && grid->rot_table[ind]) vb->setUV(vi+2,uv_p); else vb->setUV(vi+2,uv_q);
                        if(grid->color_table) vb->setColor(vi+2,grid->color_table[ind]); else vb->setColor(vi+2, Color(1,1,1,1));
                        // right top
                        vb->setCoord(vi+3,Vec3(d*(x+1),d*(y+1),0));
                        if(grid->rot_table && grid->rot_table[ind]) vb->setUV(vi+3,uv_q); else vb->setUV(vi+3,uv_r);
                        if(grid->color_table) vb->setColor(vi+3,grid->color_table[ind]); else vb->setColor(vi+3, Color(1,1,1,1));

                        // TODO: no need to update index every time it changes.
                       
                        int indi = quad_cnt * 6; // 2 triangles = 6 verts per quad
                        ib->setIndex(indi++, quad_cnt*4+0 );
                        ib->setIndex(indi++, quad_cnt*4+2 );
                        ib->setIndex(indi++, quad_cnt*4+1 );
                        ib->setIndex(indi++, quad_cnt*4+1 );
                        ib->setIndex(indi++, quad_cnt*4+2 );
                        ib->setIndex(indi++, quad_cnt*4+3 );

                        quad_cnt++; // next quad!
                    }
                }
            } 

            // draw
            if(!draw_deck) {
                print("no tex? (grid)");
                continue;
            }
            //            print("ggg:%p %p %f,%f", grid->mesh, draw_deck, loc.x, loc.y );
            drawMesh( grid->mesh, draw_deck );
		}
	}


	if(deck && index >= 0 ){
        if(!mesh) {
            mesh = new Mesh();
            VertexFormat *vf = getVertexFormat(); // TODO: 色が不要なときも色情報持っている。
            VertexBuffer *vb = new VertexBuffer();
            vb->setFormat(vf);
            vb->reserve(4);
            // 頂点は4、三角形は2個なのでindexは6個
            // C --- D
            // |     |
            // A --- B
            float d = 0.5;
            vb->setCoord(0, Vec3(-d,-d,0)); // A TODO: 2Dの座標で十分
            vb->setCoord(1, Vec3(d,-d,0)); // B
            vb->setCoord(2, Vec3(-d,d,0)); // C
            vb->setCoord(3, Vec3(d,d,0)); // D
            updateVertexColor(vb);
            updateVertexUV(vb);

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
            if( uv_changed ) {
                updateVertexUV(mesh->vb);
                uv_changed = false;
            }
            if( color_changed ) {
                updateVertexColor(mesh->vb);
                color_changed = false;
            }
        }

        if( fragment_shader ){
            glUseProgram( fragment_shader->program );
            fragment_shader->updateUniforms();
        }
        drawMesh( mesh, deck );
	}

	if( children_num > 0 && (render_children_first == false) ){
		for(int i=0;i<children_num;i++){
			Prop2D_OGL *p = (Prop2D_OGL*) children[i];
			if(p->visible) p->render( cam );
		}
	}

	// primitives should go over image sprites
	if( prim_drawer ){
        glDisable( GL_TEXTURE_2D );
        glLoadIdentity();
        glTranslatef( loc.x, loc.y, 0 );
        glRotatef( rot * (180.0f/M_PI), 0,0,1);
        glScalef( scl.x, scl.y, 1 );
		prim_drawer->drawAll();
	}
}

void Prop2D_OGL::drawMesh( Mesh *m, TileDeck *dk ) {
    if(!dk) {
        // no texture, nothing to do!
        return;
    }
    glEnable(GL_TEXTURE_2D);
    if(dk->tex->tex) {
        glBindTexture( GL_TEXTURE_2D, dk->tex->tex );
    } else {
        // no opengl texture name, nothing to do!
        print("no opengl tex?");
        return;
    }
    assert(m);
    assert(m->vb);
    assert(m->ib);
    assert(m->vb->fmt);            

    m->vb->bless();
    assert( m->vb->gl_name > 0 );        
    m->ib->bless();
    assert( m->ib->gl_name > 0 );

    int vert_sz = m->vb->fmt->getNumFloat() * sizeof(float);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m->ib->gl_name );
    glBindBuffer( GL_ARRAY_BUFFER, m->vb->gl_name );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
        
    // 以下prop3dからのコピペ、動いたら共通化する
    if( m->vb->fmt->coord_offset >= 0 ){
        glEnableClientState( GL_VERTEX_ARRAY );        
        glVertexPointer( 3, GL_FLOAT, vert_sz, (char*)0 + m->vb->fmt->coord_offset * sizeof(float) );
    }
    if( m->vb->fmt->color_offset >= 0 ){
        glEnableClientState( GL_COLOR_ARRAY );
        glColorPointer( 4, GL_FLOAT, vert_sz, (char*)0 + m->vb->fmt->color_offset * sizeof(float));
    }
    if( m->vb->fmt->texture_offset >= 0 ){
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );                    
        glTexCoordPointer( 2, GL_FLOAT, vert_sz, (char*)0 + m->vb->fmt->texture_offset * sizeof(float) );
    }
    if( m->vb->fmt->normal_offset >= 0 ) {
        glEnableClientState( GL_NORMAL_ARRAY );
        glNormalPointer( GL_FLOAT, vert_sz, (char*)0 + m->vb->fmt->normal_offset * sizeof(float) );
    }

    glDisable(GL_LIGHTING); // TODO: may be outside of this function
    glDisable(GL_LIGHT0);
        
    glLoadIdentity();    

    glTranslatef( loc.x, loc.y, 0 );
    glRotatef( rot * (180.0f/M_PI), 0,0,1);
    glScalef( scl.x, scl.y, 1 );
    assert( m->prim_type == GL_TRIANGLES );

    glDrawElements( m->prim_type, m->ib->array_len, GL_UNSIGNED_INT, 0);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
        
    if( fragment_shader ){
        glUseProgram(0);
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

void Prop2D_OGL::updateVertexUV( VertexBuffer *vb ) {
    vb->unbless();
    float u0,v0,u1,v1;
    deck->getUVFromIndex(index, &u0, &v0, &u1, &v1, 0, 0, 0 );
    // C --- D
    // |     |
    // A --- B
    if(xflip) swapf( &u0, &u1 );
    if(yflip) swapf( &v0, &v1 );
    Vec2 uv_a(u0,v1), uv_b(u1,v1), uv_c(u0,v0), uv_d(u1,v0);    
    if(uvrot) {
        Vec2 uvtmp = uv_d;
        uv_d = uv_c;
        uv_c = uv_a;        
        uv_a = uv_b;
        uv_b = uvtmp;
    }
    vb->setUV(0, uv_a );
    vb->setUV(1, uv_b );
    vb->setUV(2, uv_c ); 
    vb->setUV(3, uv_d );
}
void Prop2D_OGL::updateVertexColor( VertexBuffer *vb ) {
    vb->unbless();
    vb->setColor(0, color ); // A
    vb->setColor(1, color ); // B
    vb->setColor(2, color ); // C
    vb->setColor(3, color ); // D
}
#endif
