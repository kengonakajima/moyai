#include "CharGrid.h"

void CharGrid::printf( int x, int y, Color c, const char *fmt, ...) {
	va_list argptr;
	char dest[1024];
	va_start(argptr, fmt);
	vsnprintf( dest, sizeof(dest), fmt, argptr );
	va_end(argptr);

	int l = strlen(dest);
	for(int i=0;i<l;i++){
		int ind = ascii_offset + dest[i];
		if(x+i>=width)break;
		set(x+i,y,ind);
		setColor(x+i,y,c);
	}    
}