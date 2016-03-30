#include "client.h"

#include "MoyaiClient.h"

int MoyaiClient::render(){
    batch_list.clear();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
    SorterEntry tosort[128];
    int sort_n = 0;
    for(int i=0;i<elementof(groups);i++) {
        Group *g = groups[i];
		if( g && g->to_render ) {
            Layer *l = (Layer*)g;
            tosort[sort_n].val = l->priority;
            tosort[sort_n].ptr = l;
            sort_n++;
        }
    }
    quickSortF( tosort, 0, sort_n-1 );
        
	int render_n=0;    
	for(int i=0;i<sort_n;i++){
		Layer *l = (Layer*) tosort[i].ptr;
        render_n += l->renderAllProps(&batch_list);
	}

    last_draw_call_count = batch_list.drawAll();    
	glfwSwapBuffers(window);
	glFlush();
	return render_n;
}

void MoyaiClient::capture( Image *img ) {
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

int MoyaiClient::poll( double dt ) {
    int cnt = Moyai::poll(dt);
    if( remote_head ) {
         remote_head->heartbeat();
    }    
    return cnt;    
}
int MoyaiClient::getHighestPriority() {
	int prio = 0;
    for(int i=0;i<MAXGROUPS;i++) {
        if( groups[i] ) {
            Layer *l = (Layer*)groups[i];
            if(l->priority > prio ) prio = l->priority;
        }
    }
    return prio;
}
