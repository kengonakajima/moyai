#include "client.h"

#include "Viewport.h"
#include "Prop2D.h"
#include "FragmentShader.h"
#include "AnimCurve.h"

FragmentShader *Prop2D::default_fs=NULL;

bool Prop2D::propPoll(double dt) {
	if( prop2DPoll(dt) == false ) return false;
    if(remote_vel.isZero()==false) { // for cloud syncing
        loc += remote_vel*dt;
    }

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
            if(seek_color_time!=0){
                onColorChanged();
            }
			seek_color_time = 0;
		} else {
			double rate = elt / seek_color_time;
			color = Color( seek_color_orig.r + ( seek_color_target.r - seek_color_orig.r ) * rate,
				seek_color_orig.g + ( seek_color_target.g - seek_color_orig.g ) * rate,
				seek_color_orig.b + ( seek_color_target.b - seek_color_orig.b ) * rate,
				seek_color_orig.a + ( seek_color_target.a - seek_color_orig.a ) * rate );
            onColorChanged();
		}
	}

	// children
	for(int i=0;i<children_num;i++){
		Prop2D *p = children[i];
		p->basePoll(dt);
	}

	return true;
}

void Prop2D::render(Camera *cam, DrawBatchList *bl ) {
    if( debug_id ) {
        assertmsg(deck || grid_used_num > 0 || children_num > 0 || prim_drawer , "no deck/grid/prim_drawer is set. deck:%p grid:%d child:%d prim:%p", deck, grid_used_num, children_num, prim_drawer );
    }
    
	float camx=0.0f;
	float camy=0.0f;
	if(cam){
		camx = cam->loc.x;
		camy = cam->loc.y;
	}

	if( children_num > 0 && render_children_first ){
		for(int i=0;i<children_num;i++){
			Prop2D *p = (Prop2D*) children[i];
            if( p->visible ) {
                if( !p->parent_group ) {
                    p->parent_group = parent_group;
                }
                p->render( cam, bl );
            }
		}
	}

	if( grid_used_num > 0 ){
		glEnable(GL_TEXTURE_2D);
		glColor4f(color.r,color.g,color.b,color.a);        
		for(int i=0;i<grid_used_num;i++){
			Grid *grid = grids[i];
			if(!grid)break;
			if(!grid->visible)continue;
            if(!grid->index_table)continue;

			Deck *draw_deck = deck;
			if( grid->deck ) draw_deck = grid->deck;
			if( grid->fragment_shader ){
#if !(TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE || defined(__linux__) )
				glUseProgram(grid->fragment_shader->program );
#endif                
				grid->fragment_shader->updateUniforms();
			}

            if(!grid->mesh) {
                //                print("new grid mesh! wh:%d,%d", grid->width, grid->height );
                grid->mesh = new Mesh();
                VertexFormat *vf = DrawBatch::getVertexFormat( VFTYPE_COORD_COLOR_UV );
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
                if(grid->debug) {
                    print("debug:%d Grid changed: uv:%d col:%d", grid->debug, grid->uv_changed, grid->color_changed );
                }
                grid->uv_changed = false;
                grid->color_changed = false;
                
                VertexBuffer *vb = grid->mesh->vb;
                IndexBuffer *ib = grid->mesh->ib;
                vb->unbless();
                ib->unbless();

                int quad_cnt=0;
                for(int y=0;y<grid->height;y++) {
                    for(int x=0;x<grid->width;x++) {
                        int ind = x+y*grid->width;
                        if(grid->debug) {
                            if(grid->texofs_table) {
                                prt("%.2f,%.2f ", grid->texofs_table[ind].x, grid->texofs_table[ind].y );
                            } else if( grid->index_table ) {
                                prt("%3d ", grid->index_table[ind] );
                            }
                        }
                        if( grid->index_table[ind] == Grid::GRID_NOT_USED ) continue;
                        
                        
                        Vec2 left_bottom, right_top;
                        float u0,v0,u1,v1;
                        draw_deck->getUVFromIndex( grid->index_table[ind], &u0,&v0,&u1,&v1,0,0,0);
                        if(grid->texofs_table) {
                            float u_per_cell = draw_deck->getUperCell();
                            float v_per_cell = draw_deck->getVperCell();
                            u0 += grid->texofs_table[ind].x * u_per_cell;
                            v0 += grid->texofs_table[ind].y * v_per_cell;
                            u1 += grid->texofs_table[ind].x * u_per_cell;
                            v1 += grid->texofs_table[ind].y * v_per_cell;
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
                        vb->setCoord(vi,Vec3(d*x-enfat_epsilon,d*y-enfat_epsilon,0));
                        if(grid->rot_table && grid->rot_table[ind]) vb->setUV(vi,uv_s); else vb->setUV(vi,uv_p);
                        if(grid->color_table) vb->setColor(vi, grid->color_table[ind]); else vb->setColor(vi, Color(1,1,1,1));
                        // right bottom
                        vb->setCoord(vi+1,Vec3(d*(x+1)+enfat_epsilon,d*y-enfat_epsilon,0));
                        if(grid->rot_table && grid->rot_table[ind]) vb->setUV(vi+1,uv_r); else vb->setUV(vi+1,uv_s);                        
                        if(grid->color_table) vb->setColor(vi+1,grid->color_table[ind]); else vb->setColor(vi+1, Color(1,1,1,1));
                        // left top
                        vb->setCoord(vi+2,Vec3(d*x-enfat_epsilon,d*(y+1)+enfat_epsilon,0));
                        if(grid->rot_table && grid->rot_table[ind]) vb->setUV(vi+2,uv_p); else vb->setUV(vi+2,uv_q);
                        if(grid->color_table) vb->setColor(vi+2,grid->color_table[ind]); else vb->setColor(vi+2, Color(1,1,1,1));
                        // right top
                        vb->setCoord(vi+3,Vec3(d*(x+1)+enfat_epsilon,d*(y+1)+enfat_epsilon,0));
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
                    if(grid->debug) print("");
                }
                ib->setRenderLen(quad_cnt*6);
            } 

            
            // draw
            if(!draw_deck) {
                print("no tex? (grid)");
                continue;
            }
            if( grid->mesh == NULL || grid->mesh->hasIndexesToRender() == false ) {
                continue;
            }
            FragmentShader *fs = fragment_shader;
            if( grid->fragment_shader ) fs = grid->fragment_shader;
            //            print("appendMesh, tex:%d vn:%d rn:%d", draw_deck->tex->tex, grid->mesh->vb->array_len, grid->mesh->ib->render_len );
            Vec2 finloc(loc.x+grid->rel_loc.x, loc.y+grid->rel_loc.y);
            Vec2 finscl(scl.x*grid->rel_scl.x, scl.y*grid->rel_scl.y);
            if(!fs)fs=default_fs;
            bl->appendMesh( getViewport(), fs, getBlendType(), draw_deck->tex->tex, finloc - Vec2(camx,camy), finscl, rot, grid->mesh, copy_mesh_at_draw );
		}
	}

	if(deck && index >= 0 ){
        float u0,v0,u1,v1;
        deck->getUVFromIndex( index, &u0,&v0,&u1,&v1,0,0,0);
        if(xflip) {
            swapf(&u0,&u1);
        }
        if(yflip) {
            swapf(&v0,&v1);
        }
        // Q (u0,v0) - R (u1,v0)      top-bottom upside down.
        //      |           |
        //      |           |                        
        // P (u0,v1) - S (u1,v1)
        Vec2 uv_p(u0,v1), uv_q(u0,v0), uv_r(u1,v0), uv_s(u1,v1);
        if(uvrot) {
            Vec2 tmp = uv_p;
            uv_p = uv_s;
            uv_s = uv_r;
            uv_r = uv_q;
            uv_q = tmp;
        }
        if(!fragment_shader)fragment_shader=default_fs;
        bl->appendSprite1( getViewport(),
                           fragment_shader,
                           getBlendType(),
                           deck->tex->tex,
                           color,
                           loc - Vec2(camx,camy) + draw_offset,
                           scl,
                           rot,
                           uv_p,
                           uv_q,
                           uv_r,
                           uv_s
                           );
	}

	if( children_num > 0 && (render_children_first == false) ){
		for(int i=0;i<children_num;i++){
			Prop2D *p = (Prop2D*) children[i];
			if(p->visible) {
                if( !p->parent_group ) {
                    p->parent_group = parent_group;
                }
                p->render( cam, bl );
            }
		}
	}

	// primitives should go over image sprites
	if( prim_drawer ){
		prim_drawer->drawAll( bl, getViewport(), loc - Vec2(camx,camy),scl,rot);
	}
}
void GLBINDTEXTURE( GLuint tex ) {
    static GLuint last_tex;
    if(tex!=last_tex) {
        glBindTexture( GL_TEXTURE_2D, tex );
        last_tex = tex;
    }
}




Prop *Prop2D::getNearestProp(){
	Prop *cur = parent_group->prop_top;
	float minlen = 999999999999999.0f;
	Prop *ans=0;
	while(cur){
		Prop2D *cur2d = (Prop2D*)cur;        
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

void Prop2D::updateMinMaxSizeCache(){
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
			float maxx = ((Prop2D*)p)->scl.x/2;
			float maxy = ((Prop2D*)p)->scl.y/2;
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


void Prop2D::onTrack( RemoteHead *rh, Prop2D *parentprop ) {
    if( !tracker ) {
        tracker = new Tracker2D(rh,this);
    }
    tracker->scanProp2D(parentprop);
    tracker->broadcastDiff( false );
    tracker->flipCurrentBuffer();

    // grids
    for(int i=0;i<grid_used_num;i++) {
        Grid *g = grids[i];
        if(!g->tracker) g->tracker = new TrackerGrid(rh,g);
        g->tracker->scanGrid();
        g->tracker->broadcastDiff(this, false );
        g->tracker->flipCurrentBuffer();
    }

    // shader
    if(fragment_shader) {
        ColorReplacerShader *crs = dynamic_cast<ColorReplacerShader*>(fragment_shader);
        if(crs) {
            crs->onTrack(rh);
        }
    }

    // prims
    if( prim_drawer ) {
        prim_drawer->onTrack(this,rh);
    }

    // dynamic image
    if( deck && deck->tex && deck->tex->image ) {
        deck->tex->image->onTrack( deck, rh );        
    }

    // children
    for(int i=0;i<children_num;i++) {
        Prop2D *p = (Prop2D*) children[i];
        p->onTrack( rh, this );
    }
}

void Prop2D::clearChildren() {
    if(tracker) {
        for(int i=0;i<children_num;i++) {
            Prop2D *p = (Prop2D*)children[i];
            tracker->parent_rh->notifyChildCleared(this,p);
        }
    }
    children_num=0;
}

bool Prop2D::clearChild( Prop2D *p ) {
    for(int i=0;i<elementof(children);i++) {
        if( children[i] ==p ) {
            for(int j=i;j<elementof(children)-1;j++){
                children[j] = children[j+1];
            }
            if(tracker) tracker->parent_rh->notifyChildCleared(this,p);
            children[children_num-1] = NULL;
            children_num --;
            return true;
        }
    }
    return false;
}

bool Prop2D::hitGrid(Vec2 at, float margin ) {        
    for(int i=0;i<grid_used_num;i++) {
        Grid *g = grids[i];
        Vec2 rt( scl.x * g->width, scl.y * g->height );
        if( (at.x >= loc.x-margin) && (at.x <= loc.x+rt.x+margin) &&
            (at.y >= loc.y-margin) && (at.y <= loc.y+rt.y+margin) ) {
            return true;
        }
    }
    return false;
}
void Prop2D::drawToDBL( Layer *l, DrawBatchList *bl, FragmentShader *fs, bool additive_blend, Deck *dk, int index, Color col, Vec2 loc, Vec2 scl, float rot ) {
    Prop2D p(dk,index,scl,loc);
    p.parent_group = l;
    p.use_additive_blend = additive_blend;
    p.setFragmentShader(fs);
    p.setColor(col);
    p.setRot(rot);
    p.render(l->camera,bl);
}

bool Prop2D::isInView( Vec2 *minv, Vec2 *maxv, Camera *cam ) {
    float camx=0,camy=0;
    if(cam) {
        camx=cam->loc.x;
        camy=cam->loc.y;
    }
    float scr_maxx = loc.x - camx + scl.x/2 + max_rt_cache.x;
    float scr_minx = loc.x - camx - scl.x/2 + min_lb_cache.x;
    float scr_maxy = loc.y - camy + scl.y/2 + max_rt_cache.y;
    float scr_miny = loc.y - camy - scl.y/2 + min_lb_cache.y;

    return ( scr_maxx >= minv->x && scr_minx <= maxv->x && scr_maxy >= minv->y && scr_miny <= maxv->y );
}
