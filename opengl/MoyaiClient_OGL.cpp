#ifdef USE_OPENGL

#include "MoyaiClient_OGL.h"

int MoyaiClient_OGL::render(){
    batch_list.clear();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
	int cnt=0;
	for(int i=0;i<elementof(groups);i++){
		Group *g = groups[i];
		if( g && g->to_render ) {
			Layer *l = (Layer*) g;
			cnt += l->renderAllProps(&batch_list);
		}
	}
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
    last_draw_call_count = batch_list.drawAll();    
	glfwSwapBuffers(window);
	glFlush();
	return cnt;
}

void MoyaiClient_OGL::capture( Image *img ) {
	float *buf = (float*)MALLOC( img->width * img->height * 3 * sizeof(float) );
	glReadPixels( 0, 0, img->width, img->height, GL_RGB, GL_FLOAT, buf );
	for(int y=0;y<img->height;y++){
		for(int x=0;x<img->width;x++){
			int ind = (x + y * img->width)*3;
			float r = buf[ind+0], g = buf[ind+1], b = buf[ind+2];
			Color c( r,g,b,1);
			img->setPixel(x,img->height-1-y,c);
		}
	}    
	FREE(buf);
}


#endif
