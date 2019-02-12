#include "client.h"

#include "CharGrid.h"

void CharGrid::printf( int x, int y, Color c, const char *fmt, ...) {
	va_list argptr;
	char s[1024];
	va_start(argptr, fmt);
	vsnprintf( s, sizeof(s), fmt, argptr );
	va_end(argptr);
    int dx=0, dy=0;
	int l = strlen(s);
	for(int i=0;i<l;i++){
        char ch=s[i];
        if(ch=='\n') {
            dx=0;
            dy--;
            continue;
        }
		int ind = ascii_offset + s[i];
		if(x+dx>=width)continue;
		set(x+dx,y+dy,ind);
		setColor(x+dx,y+dy,c);
        dx++;
	}    
}
