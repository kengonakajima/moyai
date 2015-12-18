#ifdef USE_OPENGL

#include "TextBox_OGL.h"



void TextBox_OGL::render(Camera *cam ) {
    if(!str) return;

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    

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
        VertexFormat *vf = getVertexFormat();
        vb->setFormat(vf);
        vb->reserve(quad_num*4);
        IndexBuffer *ib = new IndexBuffer();
        ib->reserve(quad_num*4);
        mesh->setVertexBuffer(vb);
        mesh->setIndexBuffer(ib);
        mesh->setPrimType(GL_QUADS);
        

        Vec2 start_lb(0, line_num * font->pixel_size ); // render starts from bottom line and go up to the first line
        Vec2 cur_lb = start_lb;


        float y_margin = ( (float)font->pixel_size / 7.0f ); // suitable for ascii/japanese(CJK) mixed text

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
            //            print("%f %f %f %f  :  %d %d", x0,y0,x1,y1, glyph->width, glyph->height);

            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;
            float depth = 10;
            int vi = i * 4;
            vb->setCoord(vi+0, Vec3(x0,y0,depth) ); vb->setUV(vi+0, Vec2(s0,t0) ); vb->setColor(vi+0,color);
            vb->setCoord(vi+1, Vec3(x0,y1,depth) ); vb->setUV(vi+1, Vec2(s0,t1) ); vb->setColor(vi+1,color);
            vb->setCoord(vi+2, Vec3(x1,y1,depth) ); vb->setUV(vi+2, Vec2(s1,t1) ); vb->setColor(vi+2,color);
            vb->setCoord(vi+3, Vec3(x1,y0,depth) ); vb->setUV(vi+3, Vec2(s1,t0) ); vb->setColor(vi+3,color);
            ib->setIndex(vi+0,vi+0);
            ib->setIndex(vi+1,vi+1);
            ib->setIndex(vi+2,vi+2);
            ib->setIndex(vi+3,vi+3);            
            //            glTexCoord2f(s0,t0); glVertex3i( x0,y0, depth );
            //            glTexCoord2f(s0,t1); glVertex3i( x0,y1, depth );
            //            glTexCoord2f(s1,t1); glVertex3i( x1,y1, depth );
            //            glTexCoord2f(s1,t0); glVertex3i( x1,y0, depth );

            cur_lb.x += glyph->advance_x;
        }
        
    }
    Vec2 camloc;
    if(cam) {
        camloc.x = cam->loc.x;
        camloc.y = cam->loc.y;
    }        
    drawMesh( mesh, font->atlas->id, camloc );
}

#endif
