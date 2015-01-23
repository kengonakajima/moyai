#ifdef USE_OPENGL

#ifdef __APPLE__
#include <OpenGL/glu.h>
#endif

#include "Layer_OGL.h"

#include "../common.h"
#include "../common/TileDeck.h"
#include "../common/Camera.h"
#include "../common/Material.h"
#include "../common/Prop2D.h"
#include "../common/Prop3D.h"

inline void Layer_OGL::drawBillboard(int billboard_index, TileDeck *deck, Vec3 *loc, Vec3 *scl  ) {
	if(billboard_index <0)return;    
	assert(deck);
	glEnable(GL_TEXTURE_2D);
	if( deck->tex->tex != last_tex_gl_id ) {
		glBindTexture( GL_TEXTURE_2D, deck->tex->tex );
		last_tex_gl_id = deck->tex->tex;
	}
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	glLoadIdentity();    
	Vec3 diff_camera = *loc - camera->loc;
	Vec3 up_v(0.0f, 1.0f, 0.0f);

	Vec3 cross_a = diff_camera.cross(up_v).normalize(1); 
	Vec3 cross_b = diff_camera.cross(cross_a).normalize(1); 

	// now you can use CrossA and CrossB and the billboard position to calculate the positions of the edges of the billboard-rectangle
	cross_a.x *= scl->x * 0.5;        
	cross_a.y *= scl->y * 0.5;
	cross_a.z *= scl->z * 0.5;
	cross_b.x *= scl->x * 0.5;        
	cross_b.y *= scl->y * 0.5;
	cross_b.z *= scl->z * 0.5;

	Vec3 p1 = *loc + cross_a + cross_b;
	Vec3 p2 = *loc - cross_a + cross_b;
	Vec3 p3 = *loc - cross_a - cross_b;
	Vec3 p4 = *loc + cross_a - cross_b;

#if 0
	print("loc:%f %f %f a:%.1f %.1f %.1f b:%.1f %.1f %.1f  p1:%.1f %.1f %.1f p2:%.1f %.1f %.1f p3:%.1f %.1f %.1f p4:%.1f %.1f %.1f",
		loc->x, loc->y, loc->z,
		cross_a.x, cross_a.y, cross_a.z,
		cross_b.x, cross_b.y, cross_b.z,
		p1.x - loc->x, p1.y - loc->y, p1.z - loc->z,
		p2.x - loc->x, p2.y - loc->y, p2.z - loc->z,
		p3.x - loc->x, p3.y - loc->y, p3.z - loc->z,
		p4.x - loc->x, p4.y - loc->y, p4.z - loc->z                           
		);
#endif

	float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
	if( deck ) deck->getUVFromIndex( billboard_index, &u0, &v0, &u1, &v1, 0, 0, 0.0001 );

	glBegin(GL_TRIANGLES);
	glColor4f( 1,1,1,1 );
	glTexCoord2f(u1,v1); glVertex3f( p1.x, p1.y, p1.z );
	glTexCoord2f(u0,v0); glVertex3f( p3.x, p3.y, p3.z );
	glTexCoord2f(u0,v1); glVertex3f( p2.x, p2.y, p2.z );

	glTexCoord2f(u1,v1); glVertex3f( p1.x, p1.y, p1.z );        
	glTexCoord2f(u1,v0); glVertex3f( p4.x, p4.y, p4.z );
	glTexCoord2f(u0,v0); glVertex3f( p3.x, p3.y, p3.z );        
	glEnd();

}

inline void Layer_OGL::drawMesh( int dbg, Mesh *mesh, TileDeck *deck, Vec3 *loc, Vec3 *locofs, Vec3 *scl, Vec3 *rot, Vec3 *localloc, Vec3 *localscl, Vec3 *localrot, Material *material  ) {
	if( !mesh || mesh->vb->array_len == 0 || mesh->ib->array_len == 0 ) return; // nothing to render!

	if( deck ) {
		glEnable(GL_TEXTURE_2D);
		if( deck->tex->tex != last_tex_gl_id ) {
			glBindTexture( GL_TEXTURE_2D, deck->tex->tex );
			last_tex_gl_id = deck->tex->tex;
		}
	} else {
		glDisable(GL_TEXTURE_2D);            
	}

	mesh->vb->bless();
	assert( mesh->vb->gl_name > 0 );
	mesh->ib->bless();
	assert( mesh->ib->gl_name > 0 );
	int vert_sz = mesh->vb->fmt->getNumFloat() * sizeof(float);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->ib->gl_name );
	glBindBuffer( GL_ARRAY_BUFFER, mesh->vb->gl_name );


	if( dbg != 0  ){
		print("draw mesh! dbg:%d deck:%p mesh:%p vbname:%d ibname:%d coordofs:%d colofs:%d texofs:%d normofs:%d vert_sz:%d varray_len:%d iarray_len:%d loc:%f %f %f",
			dbg,
			deck,
			mesh,
			mesh->vb->gl_name,
			mesh->ib->gl_name,
			mesh->vb->fmt->coord_offset,
			mesh->vb->fmt->color_offset,
			mesh->vb->fmt->texture_offset,
			mesh->vb->fmt->normal_offset,
			vert_sz,
			mesh->vb->array_len,
			mesh->ib->array_len,              
			loc->x, loc->y, loc->z
			);
	}

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );

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

	glLoadIdentity();    

	// ライトが設定されてるメッシュの中でも、マテリアルが設定されてないメッシュが混ざってるときはライトつけないことが必要。
	// TODO: すでにライトあててるときは再設定必要ない
	if( light && material ) {
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		float ambient[4] = { light->ambient.r, light->ambient.g, light->ambient.b, light->ambient.a };
		glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
		float diffuse[4] = { light->diffuse.r, light->diffuse.g, light->diffuse.b, light->diffuse.a };
		glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
		float pos[4] = { light->pos.x, light->pos.y, light->pos.z, 0 };
		glLightfv( GL_LIGHT0, GL_POSITION, pos );
		float specular[4] = { light->specular.r, light->specular.g, light->specular.b, light->specular.a };        
		glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
	} else {
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}


	glTranslatef( loc->x + locofs->x, loc->y + locofs->y, loc->z + locofs->z );
	if( rot->x != 0 ) glRotatef( rot->x, 1,0,0);     
	if( rot->y != 0 ) glRotatef( rot->y, 0,1,0);     
	if( rot->z != 0 ) glRotatef( rot->z, 0,0,1);

	glScalef( scl->x, scl->y, scl->z );


	if( localloc ) {
		glTranslatef( localloc->x, localloc->y, localloc->z );
		if( localrot->x != 0 ) glRotatef( localrot->x, 1,0,0);     
		if( localrot->y != 0 ) glRotatef( localrot->y, 0,1,0);     
		if( localrot->z != 0 ) glRotatef( localrot->z, 0,0,1);
		glScalef( localscl->x, localscl->y, localscl->z );
	}        

	if(material) {
		float diffuse[4] = { material->diffuse.r, material->diffuse.g, material->diffuse.b, material->diffuse.a };
		glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse );
		float ambient[4] = { material->ambient.r, material->ambient.g, material->ambient.b, material->ambient.a };
		glMaterialfv( GL_FRONT, GL_AMBIENT, ambient );
		float specular[4] = { material->specular.r, material->specular.g, material->specular.b, material->specular.a };
		glMaterialfv( GL_FRONT, GL_SPECULAR, specular);
	}

	if( mesh->prim_type == GL_LINES || mesh->prim_type == GL_LINE_STRIP ) {
		glLineWidth(mesh->line_width);
	}
	glDrawElements( mesh->prim_type, mesh->ib->array_len, GL_UNSIGNED_INT, 0);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

}

int Layer_OGL::renderAllProps(){
	assertmsg( viewport, "no viewport in a layer id:%d setViewport missed?", id );
	if( viewport->dimension == DIMENSION_2D ) {
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
		glDisable(GL_DEPTH_TEST);


		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho( -viewport->scl.x/2, viewport->scl.x/2, -viewport->scl.y/2, viewport->scl.y/2,-100,100);  // center is always (0,0)
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();        

		static SorterEntry tosort[1024*32];

		int cnt = 0;
		int drawn = 0;
		Prop *cur = prop_top;

		Vec2 minv, maxv;
		viewport->getMinMax(&minv, &maxv);
		while(cur){
			Prop2D_OGL *cur2d = (Prop2D_OGL*)cur;

			assert( cur2d->dimension == viewport->dimension );

			// culling
			float camx=0,camy=0;
			if(camera){
				camx = camera->loc.x;
				camy = camera->loc.y;
			}

			float scr_maxx = cur2d->loc.x - camx + cur2d->scl.x/2 + cur2d->max_rt_cache.x;
			float scr_minx = cur2d->loc.x - camx - cur2d->scl.x/2 + cur2d->min_lb_cache.x;
			float scr_maxy = cur2d->loc.y - camy + cur2d->scl.y/2 + cur2d->max_rt_cache.y;
			float scr_miny = cur2d->loc.y - camy - cur2d->scl.y/2 + cur2d->min_lb_cache.y;

			if( scr_maxx >= minv.x && scr_minx <= maxv.x && scr_maxy >= minv.y && scr_miny <= maxv.y ){
				tosort[cnt].val = cur2d->priority;
				tosort[cnt].ptr = cur2d;
				cnt++;
				if(cnt>= elementof(tosort)){
					print("WARNING: too many props in a layer : %d", cnt );
					break;
				}
				drawn ++;
			} 
			cur = cur->next;
		}

		quickSortF( tosort, 0, cnt-1 );
		//        for(int i=cnt-1;i>=0;i--){
		for(int i=0;i<cnt;i++) {
			Prop2D_OGL *p = (Prop2D_OGL*) tosort[i].ptr;
			if(p->visible){
				//                { Prop2D *p2d = (Prop2D*)p; print("prio:%f %d %d", p2d->loc.y, p2d->priority, p2d->id  ); }
				p->render(camera);
			}
		}
		return drawn;
	} else { // 3D
		assertmsg(camera, "3d render need camera.");

		glEnable(GL_DEPTH_TEST);

		setupProjectionMatrix3D();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// もともとライト設定はこの位置にあったが drawmeshに移動した

		int cnt=0;
		last_tex_gl_id = 0;

		Prop *cur = prop_top;
		while(cur){
			assert(cur->id>0);            
			Prop3D_OGL *cur3d = (Prop3D_OGL*)cur;            
			assert(cur3d->dimension == viewport->dimension );



			assertmsg( cur3d->mesh || cur3d->children_num > 0 || cur3d->billboard_index>=0, "mesh or children is required for 3d prop %p", cur3d );

			cnt++;

			if( cur3d->visible ) {
				cur3d->performRenderOptions();
				if( cur3d->billboard_index >= 0 ) {
					drawBillboard( cur3d->billboard_index, cur3d->deck, & cur3d->loc, & cur3d->scl  );
				} else if( cur3d->mesh ) {
					drawMesh( cur3d->debug_id, cur3d->mesh, cur3d->deck,
						& cur3d->loc, &cur3d->draw_offset, & cur3d->scl, & cur3d->rot,
						NULL, NULL, NULL, cur3d->material );
				}
				cur3d->cleanRenderOptions();


				if( cur3d->children_num > 0 ) {
					int opaque_n=0;
					int transparent_n=0;
					for(int i=0;i<cur3d->children_num;i++) {
						cnt++;
						Prop3D_OGL *child = cur3d->children[i];
						if(child && child->visible ) {
							float l = camera->loc.len( cur3d->loc + child->loc + child->sort_center );
							if( child->billboard_index > 0 ) {
								print("child node doesn't support billboarding");
							}
							assert( child->mesh );
							if( child->mesh->transparent ) {
								sorter_transparent[transparent_n].ptr = (void*)cur3d->children[i];
								sorter_transparent[transparent_n].val = l;
								transparent_n++;
							} else {
								sorter_opaque[opaque_n].ptr = (void*)cur3d->children[i];
								sorter_opaque[opaque_n].val = l;
								opaque_n++;
							}
						}
					}
					if( opaque_n > 0 ) quickSortF( sorter_opaque, 0, opaque_n-1 );
					if( transparent_n > 0 ) quickSortF( sorter_transparent, 0, transparent_n-1 );

					// draw opaque mesh first
					for(int i=opaque_n-1;i>=0;i--) {
						Prop3D_OGL *child = (Prop3D_OGL*)sorter_opaque[i].ptr;
						child->performRenderOptions();
						if( child->skip_rot ) {
							Vec3 fixedrot(0,0,0);
							drawMesh( child->debug_id, child->mesh, child->deck,
								& cur3d->loc, &cur3d->draw_offset, & cur3d->scl, & fixedrot,
								& child->loc, & child->scl, & child->rot,
								child->material
								);

						} else { 
							drawMesh( child->debug_id, child->mesh, child->deck,
								& cur3d->loc, &cur3d->draw_offset, & cur3d->scl, & cur3d->rot,
								& child->loc, & child->scl, & child->rot,
								child->material
								);
						}

						child->cleanRenderOptions();                        
					}
					for(int i=transparent_n-1;i>=0;i--){
						Prop3D_OGL *child = (Prop3D_OGL*)sorter_transparent[i].ptr;
						child->performRenderOptions();
						if( child->skip_rot ) {
							Vec3 fixedrot(0,0,0);
							drawMesh( child->debug_id, child->mesh, child->deck,
								& cur3d->loc, &cur3d->draw_offset, & cur3d->scl, & fixedrot,
								& child->loc, & child->scl, & child->rot,
								child->material
								);

						} else {
							drawMesh( child->debug_id, child->mesh, child->deck,
								& cur3d->loc, &cur3d->draw_offset, & cur3d->scl, & cur3d->rot,
								& child->loc, & child->scl, & child->rot,
								child->material
								);
						}
						child->cleanRenderOptions();
					}
				}
			}                                   

			cur = cur->next;
		}
		return cnt;        
	}
}

void Layer_OGL::setupProjectionMatrix3D() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective( 60, (GLdouble)viewport->screen_width/(GLdouble)viewport->screen_height, viewport->near_clip, viewport->far_clip );
	gluLookAt( camera->loc.x,camera->loc.y,camera->loc.z,
		camera->look_at.x,camera->look_at.y,camera->look_at.z,
		camera->look_up.x,camera->look_up.y,camera->look_up.z );
}

Vec2 Layer_OGL::getScreenPos( Vec3 at ) {
	setupProjectionMatrix3D();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	double projection[16], modelview[16];
	double sx,sy,sz;
	int vp[4];

	glGetIntegerv( GL_VIEWPORT, vp );    
	glGetDoublev(GL_PROJECTION_MATRIX, projection );
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview );

	gluProject( at.x, at.y, at.z, modelview, projection, vp, &sx, &sy, &sz );
	return Vec2( sx,sy );
}

Vec3 Layer_OGL::getWorldPos( Vec2 scrpos ) {
	setupProjectionMatrix3D();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	double projection[16], modelview[16];
	int vp[4];
	glGetIntegerv( GL_VIEWPORT, vp );
	glGetDoublev(GL_PROJECTION_MATRIX, projection );
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview );

	float z;
	double ox,oy,oz;
	Vec3 out;
	glReadPixels( scrpos.x, scrpos.y, 1,1, GL_DEPTH_COMPONENT, GL_FLOAT, &z );
	gluUnProject( scrpos.x, scrpos.y, z, modelview, projection, vp, &ox, &oy, &oz );
	return Vec3(ox,oy,oz);
}

int Layer_OGL::getHighestPriority() {
	int prio = 0;
	Prop *cur = prop_top;
	while(cur) {
		Prop2D_OGL *p = (Prop2D_OGL*)cur;
		if( p->priority > prio ) prio = p->priority;
		cur = cur->next;
	}
	return prio;
}

void Layer_OGL::selectCenterInside( Vec2 minloc, Vec2 maxloc, Prop*out[], int *outlen ){
	assertmsg( viewport->dimension == DIMENSION_2D, "selectCenterInside isn't implemented for 3d viewport" );

	Prop *cur = prop_top;
	int out_max = *outlen;
	int cnt=0;

	while(cur){
		Prop2D_OGL *cur2d = (Prop2D_OGL*) cur;
		if( cur2d->dimension == DIMENSION_2D ){
			if( !cur->to_clean && cur2d->isCenterInside(minloc, maxloc) ){
				if( cnt < out_max){
					out[cnt] = cur;
					cnt++;
					if(cnt==out_max)break;
				}
			}
		}
		cur = cur->next;
	}
	*outlen = cnt;
}


#endif
