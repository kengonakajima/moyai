#pragma once

#include "Context_D3D.h"

// Our own implementation of GLFW using Direct3D. 
// Used to make sure game code does not have to change at all.

namespace glfw_d3d
{
	/* glfwOpenWindow modes */
	#define GLFW_WINDOW               0x00010001
	#define GLFW_FULLSCREEN           0x00010002

	/* glfwEnable/glfwDisable tokens */
	#define GLFW_STICKY_KEYS          0x00030002

	/* GLFW initialization, termination and version querying */
	int glfwInit( void );
	void glfwTerminate( void );

	/* Enable/disable functions */
	void glfwEnable( int token );

	/* Window handling */
	int glfwOpenWindow( int width, int height, int redbits, int greenbits, int bluebits, int alphabits, int depthbits, int stencilbits, int mode );
	void glfwSetWindowTitle( const char *title );
	void glfwGetWindowSize( int *width, int *height );
	void glfwSwapInterval( int interval );
	void glfwSwapBuffers( void );

	/* Threading */
	void glfwSleep( double time );

	#define GLFWCALL

	/* glfwGetJoystickParam tokens */
	#define GLFW_PRESENT              0x00050001
	#define GLFW_AXES                 0x00050002
	#define GLFW_BUTTONS              0x00050003


    // glfwGetWindowParam options
    #define GLFW_ICONIFIED            0x00020003				
    int glfwGetWindowParam( int flag );
    
	/*************************************************************************
	 * Input handling definitions
	 *************************************************************************/

	/* Key and button state/action definitions */
	#define GLFW_RELEASE            0
	#define GLFW_PRESS              1

	/* Keyboard key definitions: 8-bit ISO-8859-1 (Latin 1) encoding is used
	 * for printable keys (such as A-Z, 0-9 etc), and values above 256
	 * represent special (non-printable) keys (e.g. F1, Page Up etc).
	 */
	#define GLFW_KEY_UNKNOWN      -1
	#define GLFW_KEY_SPACE        32
	#define GLFW_KEY_SPECIAL      256
	#define GLFW_KEY_ESC          (GLFW_KEY_SPECIAL+1)
	#define GLFW_KEY_F1           (GLFW_KEY_SPECIAL+2)
	#define GLFW_KEY_F2           (GLFW_KEY_SPECIAL+3)
	#define GLFW_KEY_F3           (GLFW_KEY_SPECIAL+4)
	#define GLFW_KEY_F4           (GLFW_KEY_SPECIAL+5)
	#define GLFW_KEY_F5           (GLFW_KEY_SPECIAL+6)
	#define GLFW_KEY_F6           (GLFW_KEY_SPECIAL+7)
	#define GLFW_KEY_F7           (GLFW_KEY_SPECIAL+8)
	#define GLFW_KEY_F8           (GLFW_KEY_SPECIAL+9)
	#define GLFW_KEY_F9           (GLFW_KEY_SPECIAL+10)
	#define GLFW_KEY_F10          (GLFW_KEY_SPECIAL+11)
	#define GLFW_KEY_F11          (GLFW_KEY_SPECIAL+12)
	#define GLFW_KEY_F12          (GLFW_KEY_SPECIAL+13)
	#define GLFW_KEY_F13          (GLFW_KEY_SPECIAL+14)
	#define GLFW_KEY_F14          (GLFW_KEY_SPECIAL+15)
	#define GLFW_KEY_F15          (GLFW_KEY_SPECIAL+16)
	#define GLFW_KEY_F16          (GLFW_KEY_SPECIAL+17)
	#define GLFW_KEY_F17          (GLFW_KEY_SPECIAL+18)
	#define GLFW_KEY_F18          (GLFW_KEY_SPECIAL+19)
	#define GLFW_KEY_F19          (GLFW_KEY_SPECIAL+20)
	#define GLFW_KEY_F20          (GLFW_KEY_SPECIAL+21)
	#define GLFW_KEY_F21          (GLFW_KEY_SPECIAL+22)
	#define GLFW_KEY_F22          (GLFW_KEY_SPECIAL+23)
	#define GLFW_KEY_F23          (GLFW_KEY_SPECIAL+24)
	#define GLFW_KEY_F24          (GLFW_KEY_SPECIAL+25)
	#define GLFW_KEY_F25          (GLFW_KEY_SPECIAL+26)
	#define GLFW_KEY_UP           (GLFW_KEY_SPECIAL+27)
	#define GLFW_KEY_DOWN         (GLFW_KEY_SPECIAL+28)
	#define GLFW_KEY_LEFT         (GLFW_KEY_SPECIAL+29)
	#define GLFW_KEY_RIGHT        (GLFW_KEY_SPECIAL+30)
	#define GLFW_KEY_LSHIFT       (GLFW_KEY_SPECIAL+31)
	#define GLFW_KEY_RSHIFT       (GLFW_KEY_SPECIAL+32)
	#define GLFW_KEY_LCTRL        (GLFW_KEY_SPECIAL+33)
	#define GLFW_KEY_RCTRL        (GLFW_KEY_SPECIAL+34)
	#define GLFW_KEY_LALT         (GLFW_KEY_SPECIAL+35)
	#define GLFW_KEY_RALT         (GLFW_KEY_SPECIAL+36)
	#define GLFW_KEY_TAB          (GLFW_KEY_SPECIAL+37)
	#define GLFW_KEY_ENTER        (GLFW_KEY_SPECIAL+38)
	#define GLFW_KEY_BACKSPACE    (GLFW_KEY_SPECIAL+39)
	#define GLFW_KEY_INSERT       (GLFW_KEY_SPECIAL+40)
	#define GLFW_KEY_DEL          (GLFW_KEY_SPECIAL+41)
	#define GLFW_KEY_PAGEUP       (GLFW_KEY_SPECIAL+42)
	#define GLFW_KEY_PAGEDOWN     (GLFW_KEY_SPECIAL+43)
	#define GLFW_KEY_HOME         (GLFW_KEY_SPECIAL+44)
	#define GLFW_KEY_END          (GLFW_KEY_SPECIAL+45)
	#define GLFW_KEY_KP_0         (GLFW_KEY_SPECIAL+46)
	#define GLFW_KEY_KP_1         (GLFW_KEY_SPECIAL+47)
	#define GLFW_KEY_KP_2         (GLFW_KEY_SPECIAL+48)
	#define GLFW_KEY_KP_3         (GLFW_KEY_SPECIAL+49)
	#define GLFW_KEY_KP_4         (GLFW_KEY_SPECIAL+50)
	#define GLFW_KEY_KP_5         (GLFW_KEY_SPECIAL+51)
	#define GLFW_KEY_KP_6         (GLFW_KEY_SPECIAL+52)
	#define GLFW_KEY_KP_7         (GLFW_KEY_SPECIAL+53)
	#define GLFW_KEY_KP_8         (GLFW_KEY_SPECIAL+54)
	#define GLFW_KEY_KP_9         (GLFW_KEY_SPECIAL+55)
	#define GLFW_KEY_KP_DIVIDE    (GLFW_KEY_SPECIAL+56)
	#define GLFW_KEY_KP_MULTIPLY  (GLFW_KEY_SPECIAL+57)
	#define GLFW_KEY_KP_SUBTRACT  (GLFW_KEY_SPECIAL+58)
	#define GLFW_KEY_KP_ADD       (GLFW_KEY_SPECIAL+59)
	#define GLFW_KEY_KP_DECIMAL   (GLFW_KEY_SPECIAL+60)
	#define GLFW_KEY_KP_EQUAL     (GLFW_KEY_SPECIAL+61)
	#define GLFW_KEY_KP_ENTER     (GLFW_KEY_SPECIAL+62)
	#define GLFW_KEY_KP_NUM_LOCK  (GLFW_KEY_SPECIAL+63)
	#define GLFW_KEY_CAPS_LOCK    (GLFW_KEY_SPECIAL+64)
	#define GLFW_KEY_SCROLL_LOCK  (GLFW_KEY_SPECIAL+65)
	#define GLFW_KEY_PAUSE        (GLFW_KEY_SPECIAL+66)
	#define GLFW_KEY_LSUPER       (GLFW_KEY_SPECIAL+67)
	#define GLFW_KEY_RSUPER       (GLFW_KEY_SPECIAL+68)
	#define GLFW_KEY_MENU         (GLFW_KEY_SPECIAL+69)
	#define GLFW_KEY_LAST         GLFW_KEY_MENU

	/* Mouse button definitions */
	#define GLFW_MOUSE_BUTTON_1      0
	#define GLFW_MOUSE_BUTTON_2      1
	#define GLFW_MOUSE_BUTTON_3      2
	#define GLFW_MOUSE_BUTTON_4      3
	#define GLFW_MOUSE_BUTTON_5      4
	#define GLFW_MOUSE_BUTTON_6      5
	#define GLFW_MOUSE_BUTTON_7      6
	#define GLFW_MOUSE_BUTTON_8      7
	#define GLFW_MOUSE_BUTTON_LAST   GLFW_MOUSE_BUTTON_8

	/* Mouse button aliases */
	#define GLFW_MOUSE_BUTTON_LEFT   GLFW_MOUSE_BUTTON_1
	#define GLFW_MOUSE_BUTTON_RIGHT  GLFW_MOUSE_BUTTON_2
	#define GLFW_MOUSE_BUTTON_MIDDLE GLFW_MOUSE_BUTTON_3


	/* Joystick identifiers */
	#define GLFW_JOYSTICK_1          0
	#define GLFW_JOYSTICK_2          1
	#define GLFW_JOYSTICK_3          2
	#define GLFW_JOYSTICK_4          3
	#define GLFW_JOYSTICK_5          4
	#define GLFW_JOYSTICK_6          5
	#define GLFW_JOYSTICK_7          6
	#define GLFW_JOYSTICK_8          7
	#define GLFW_JOYSTICK_9          8
	#define GLFW_JOYSTICK_10         9
	#define GLFW_JOYSTICK_11         10
	#define GLFW_JOYSTICK_12         11
	#define GLFW_JOYSTICK_13         12
	#define GLFW_JOYSTICK_14         13
	#define GLFW_JOYSTICK_15         14
	#define GLFW_JOYSTICK_16         15
	#define GLFW_JOYSTICK_LAST       GLFW_JOYSTICK_16

	/* Input handling */
	void glfwSetKeyCallback( GLFWkeyfun cbfun );
    void glfwSetWindowCloseCallback( GLFWwindowclosefun cbfun );
	int glfwGetKey( int key );
	int glfwGetJoystickPos( int joy, float *pos, int numaxes );
	int glfwGetJoystickParam( int joy, int param );
	int glfwGetJoystickButtons( int joy, unsigned char *buttons, int numbuttons );
	int glfwGetMouseWheel( void );

	// OpenGL functions
	void glClearColor(float r, float g, float b, float a);
	void glewInit();
}
