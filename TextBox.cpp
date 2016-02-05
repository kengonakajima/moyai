#include "client.h"

#include "Viewport.h"
#include "TextBox.h"



void TextBox::render(Camera *cam, DrawBatchList *bl ) {
    if(!str) return;

    //    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    

    updateMesh();
    Vec2 camloc;
    if(cam) {
        camloc.x = cam->loc.x;
        camloc.y = cam->loc.y;
    }
    bl->appendMesh( getViewport(), fragment_shader, getBlendType(), font->atlas->id, loc - camloc, scl, rot, mesh );
}

void TextBox::updateMesh() {
    if(!mesh) {
        mesh = new Mesh();
        VertexBuffer *vb = new VertexBuffer();
        int l = wcslen(str);

        int line_num = 0;
        for(unsigned int i=0; i < wcslen(str); ++i ){
            if( str[i] == L"\n"[0] ){
                line_num++;
            }
        }
        int quad_num = l;
        VertexFormat *vf = DrawBatch::getVertexFormat(VFTYPE_COORD_COLOR_UV);
        vb->setFormat(vf);
        vb->reserve(quad_num*4);
        IndexBuffer *ib = new IndexBuffer();
        ib->reserve(quad_num*6);
        mesh->setVertexBuffer(vb);
        mesh->setIndexBuffer(ib);
        mesh->setPrimType(GL_TRIANGLES);
        

        Vec2 start_lb(0, line_num * font->pixel_size ); // render starts from bottom line and go up to the first line
        Vec2 cur_lb = start_lb;


        float y_margin = ( (float)font->pixel_size / 7.0f ); // suitable for ascii/japanese(CJK) mixed text

        int vi=0, ii=0;
        
        for( int i=0; i<l; ++i ){
            if( str[i] == L"\n"[0] ){
                cur_lb.x = start_lb.x;
                cur_lb.y -= font->pixel_size;
                continue;
            }
            texture_glyph_t *glyph = texture_font_get_glyph( font->font, str[i] );
            if( glyph == NULL ) continue;

            int kerning = 0;
            if( i > 0){
                kerning = texture_glyph_get_kerning( glyph, str[i-1] );
            }
            cur_lb.x += kerning;
            float x0  = ( cur_lb.x + glyph->offset_x);
            float y0  = ( cur_lb.y + glyph->offset_y) + y_margin;
            float x1  = ( x0 + glyph->width);
            float y1  = ( y0 - glyph->height) + y_margin;
            //            print("[%d] %d: %f %f %f %f  :  %d %d", id, i, x0,y0,x1,y1, glyph->width, glyph->height);

            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;
            float depth = 10;
            
            vb->setCoord(vi+0, Vec3(x0,y0,depth) ); vb->setUV(vi+0, Vec2(s0,t0) ); vb->setColor(vi+0,color);
            vb->setCoord(vi+1, Vec3(x0,y1,depth) ); vb->setUV(vi+1, Vec2(s0,t1) ); vb->setColor(vi+1,color);
            vb->setCoord(vi+2, Vec3(x1,y1,depth) ); vb->setUV(vi+2, Vec2(s1,t1) ); vb->setColor(vi+2,color);
            vb->setCoord(vi+3, Vec3(x1,y0,depth) ); vb->setUV(vi+3, Vec2(s1,t0) ); vb->setColor(vi+3,color);
            
            // 0-3
            // | |
            // 1-2
            
            ib->setIndex(ii+0,vi+0);
            ib->setIndex(ii+1,vi+1);
            ib->setIndex(ii+2,vi+2);
            ib->setIndex(ii+3,vi+0);
            ib->setIndex(ii+4,vi+2);            
            ib->setIndex(ii+5,vi+3);
            
            vi+=4;
            ii+=6;
            cur_lb.x += glyph->advance_x;
            max_rt_cache.x = cur_lb.x;
        }
        max_rt_cache.y = (line_num+1)*font->pixel_size;
        ib->setRenderLen(ii);
    }
}


