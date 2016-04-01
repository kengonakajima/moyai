
#include "common.h"
#include "Keyboard.h"


Keyboard::Keyboard() {
    memset(keys,0,sizeof(keys));
}
void Keyboard::validateKeyCode(int keycode) {
    assertmsg( keycode < MOYAI_KEYCODE_MAX, "keycode out of range:%d", keycode );    
}
int Keyboard::getKey(unsigned int keycode) {
    validateKeyCode(keycode);
    return keys[keycode];
}
void Keyboard::setKey(unsigned int keycode, int val) {
    validateKeyCode(keycode);    
    keys[keycode] = val;
}
void Keyboard::update( int keycode, int action, int modshift, int modctrl, int modalt ) {
    validateKeyCode(keycode);
    keys[keycode] = action;
    mod_shift = modshift;
    mod_ctrl = modctrl;
    mod_alt = modalt;
}
