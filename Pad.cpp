#include "GLFW/glfw3.h"
#include "Pad.h"

void Pad::readGLFW(GLFWwindow *w) 
{
	up = glfwGetKey(w,'W');
	left = glfwGetKey(w,'A');
	down = glfwGetKey(w,'S');    
	right = glfwGetKey(w,'D');
}
