#include "client.h"
#include "Pad.h"
#include "Keyboard.h"

void Pad::readKeyboard(Keyboard *kbd) {
	up = kbd->getKey('W');
	left = kbd->getKey('A');
	down = kbd->getKey('S');    
	right = kbd->getKey('D');
}
