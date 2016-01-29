#pragma once

// GLFW3 doesn't support ios.
// This is an emulator of glfw3 that almost does nothing.

typedef struct _GLFWwindow {
} GLFWwindow;
typedef struct _GLFWmonitor {
} GLFWmonitor;



void glfwSwapBuffers( GLFWwindow *w );
bool glfwInit();
void glfwSetErrorCallback( void (*f)(int code, const char *desc) );
GLFWwindow* glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
void glfwTerminate();
void glfwMakeContextCurrent( GLFWwindow *w );
void glfwSetWindowCloseCallback( GLFWwindow *w, void (* GLFWwindowclosefun)(GLFWwindow*) );
#define GLFW_STICKY_KEYS            0x00033002                                                                                         
void glfwSetInputMode( GLFWwindow *w, int mode, int value );
void glfwSwapInterval( int intvl );
bool glfwWindowShouldClose( GLFWwindow *w );
void glfwGetFramebufferSize(GLFWwindow* window, int* width, int* height);
int glfwGetKey(GLFWwindow* window, int key);
void glfwPollEvents();


