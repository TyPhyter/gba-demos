#include <gba.h>
#include <stdio.h>
#include <stdlib.h>

// prevents sprite from receiving a position outside of the screen
inline void constrainPos(short *x, short *y) {
    if(*x < 0) {
        *x = 0;
    }
    if(*x > 29) {
        *x = 29;
    }
    if(*y < 0) {
        *y = 0;
    }
    if(*y > 20) {
        *y = 20;
    }
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);

	consoleDemoInit();

	// ansi escape sequence to set print co-ordinates
	// /x1b[line;columnH

    // some tests of writing to the screen
	// iprintf("\x1b[10;10HHello World!\n");
	// iprintf("\x1b[0;10H%d!\n", KEY_A);

	u16 keysHeldVar;
    u16 keysUpVar;
    u16 keysDownVar;
    u16 keysDownRepeatVar;
    char* symbol = malloc(6*sizeof(char));
    char* outStr = malloc(25*sizeof(char));
    short posX = 0;
    short posY = 0;
    setRepeat(10, 3);
	while (1) {
        //clear screen
        iprintf("\x1b[2J");
        symbol = "$";

        scanKeys();
        keysHeldVar = keysHeld();
        keysUpVar = keysUp();
        keysDownVar = keysDown();
        keysDownRepeatVar = keysDownRepeat();
		
        if(keysHeldVar & KEY_A){
            symbol = "A";
        }
        if(keysHeldVar & KEY_B){
            symbol = "B";
        }
        if(keysHeldVar & KEY_L){
            symbol = "L";
        }
        if(keysHeldVar & KEY_R){
            symbol = "R";
        }
        if(keysHeldVar & KEY_START){
            symbol = "START";
        }
        if(keysHeldVar & KEY_SELECT){
            symbol = "SELECT";
        }
        if(keysDownRepeatVar & KEY_UP){
            posY--;
        }
        if(keysDownRepeatVar & KEY_DOWN){
            posY++;
        }
        if(keysDownRepeatVar & KEY_LEFT){
            posX--;
        }
        if(keysDownRepeatVar & KEY_RIGHT){
            posX++;
        }

        constrainPos(&posX, &posY);

        // outStr = "\x1b[" + posX + ";" + posY + "H%d!\n";
        iprintf("\x1b[%d;%dH%s\n", posY, posX, symbol);

		VBlankIntrWait();
	}
}