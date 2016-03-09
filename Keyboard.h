#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#define MOYAI_KEYCODE_MAX 512  // GLFW_KEY_LAST 348 in version 3.1.2    

class Keyboard {
public:
    int keys[MOYAI_KEYCODE_MAX];
    int mod_shift;
    int mod_ctrl;
    int mod_alt;
    
    Keyboard();
    void validateKeyCode(int keycode);
    int getKey(unsigned int keycode);
    void setKey(unsigned int keycode, int val);
    void update( int keycode, int action, int mod_shift, int mod_ctrl, int mod_alt );
};
    
#endif
