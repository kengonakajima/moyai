#include "Pad.h"

// TODO: REMOVE THIS!
#ifdef USE_OPENGL
	#include "GL/glfw.h"
#endif

void Pad::readGLFW() 
{
#ifdef USE_OPENGL
	up = glfwGetKey('W');
	left = glfwGetKey('A');
	down = glfwGetKey('S');    
	right = glfwGetKey('D');
#endif
}