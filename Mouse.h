#ifndef _MOUSE_H_
#define _MOUSE_H_

class RemoteHead;

class Mouse {
public:
    Vec2 cursor_pos;
    static const int BUTTON_MAX = 8;
    int buttons[BUTTON_MAX];
    int mod_shift;
    int mod_ctrl;
    int mod_alt;
    RemoteHead *remote_head;
    
    Mouse() : remote_head(0) {
        memset(buttons,0,sizeof(buttons));
    }
    void validateButtonIndex( unsigned int bti ) {
        assert(bti < BUTTON_MAX );
    }
    void updateButton( unsigned int button, int action, int modshift, int modctrl, int modalt ) {
        validateButtonIndex(button);
        buttons[button] = action;
        mod_shift = modshift;
        mod_ctrl = modctrl;
        mod_alt = modalt;
    }
    int getButton( unsigned int button ) {
        validateButtonIndex(button);
        return buttons[button];
    }
    void updateCursorPosition( float x, float y ) {
        cursor_pos.x = x;
        cursor_pos.y = y;
    }
    Vec2 getCursorPos() {
        return cursor_pos;
    }
    void setRemoteHead( RemoteHead *rh ) { remote_head = rh; }
};

#endif
