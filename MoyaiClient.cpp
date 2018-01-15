#include "client.h"

#include "MoyaiClient.h"

int MoyaiClient::render(){
#if defined(__linux__)
    return 0;
#else    
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
        render_n += l->render(&batch_list);
	}

    last_draw_call_count = batch_list.drawAll();    
	glfwSwapBuffers(window);
	glFlush();
	return render_n;
#endif    
}

void MoyaiClient::capture( Image *img ) {
#if !defined(__linux__)    
    glReadBuffer(GL_FRONT);
    unsigned char *buf = (unsigned char*)MALLOC( img->width * img->height * 4 );
	glReadPixels( 0, 0, img->width, img->height, GL_RGBA, GL_UNSIGNED_BYTE, buf );
	for(int y=0;y<img->height;y++){ // captured data is upside down!
        unsigned char *src = buf + (img->height-1-y) * img->width * 4;
        unsigned char *dst = img->buffer + y * img->width * 4;
        memcpy( dst, src, img->width*4);
    }
    FREE(buf);
#endif    
}

int MoyaiClient::poll( double dt ) {
    int cnt = Moyai::poll(dt);
    if( remote_head ) {
         remote_head->heartbeat(dt);
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
