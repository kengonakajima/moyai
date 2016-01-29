
#include "GLFWiosemu.h"


void glfwSwapBuffers( GLFWwindow *w ) {
}
bool glfwInit() {
    return true;
}
void glfwSetErrorCallback( void (*f)(int code, const char *desc) ) {
}
GLFWwindow* glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) {
    return 0;
}
void glfwTerminate() {
}

void glfwMakeContextCurrent( GLFWwindow *w ) {
}
void glfwSetWindowCloseCallback( GLFWwindow *w, void (* GLFWwindowclosefun)(GLFWwindow*) ) {
}

#define GLFW_STICKY_KEYS            0x00033002                                                                                         
void glfwSetInputMode( GLFWwindow *w, int mode, int value ) {
}
void glfwSwapInterval( int intvl ) {
}

bool glfwWindowShouldClose( GLFWwindow *w ) {
    return false;
}
void glfwGetFramebufferSize(GLFWwindow* window, int* width, int* height) {
}

int glfwGetKey(GLFWwindow* window, int key) {
    return false;
}
void glfwPollEvents() {
}


