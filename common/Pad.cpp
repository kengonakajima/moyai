
#ifdef USE_OPENGL
	#include "GLFW/glfw3.h"
#endif

#include "Pad.h"

void Pad::readGLFW(GLFWwindow *w) 
{
#ifdef USE_OPENGL
	up = glfwGetKey(w,'W');
	left = glfwGetKey(w,'A');
	down = glfwGetKey(w,'S');    
	right = glfwGetKey(w,'D');
#endif
}
