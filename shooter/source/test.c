#include <gba.h>
#include <stdio.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////example
#include "./include/ship0.h"
#include "./include/testProjectile.h"
#include "./include/brick.h"
#include <string.h>

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;

typedef uint32 Tile[16];
typedef Tile   TileBlock[256];

#define VIDEOMODE_0    0x0000
#define ENABLE_OBJECTS 0x1000
#define MAPPINGMODE_1D 0x0040

#define REG_VCOUNT              (*(volatile uint16*) 0x04000006)
#define REG_DISPLAYCONTROL      (*(volatile uint16*) 0x04000000)

#define MEM_VRAM      ((volatile uint16*)0x6000000)
#define MEM_TILE      ((TileBlock*)0x6000000 )
#define MEM_PALETTE   ((uint16*)(0x05000200))
#define SCREEN_W      240
#define SCREEN_H      160

typedef struct ObjectAttributes {
    uint16 attr0;
    uint16 attr1;
    uint16 attr2;
    uint16 pad;
} __attribute__((packed, aligned(4))) ObjectAttributes;

#define MEM_OAM       ((volatile ObjectAttributes *)0x07000000)

/////////////////////////////////////////////////////////////////////////////////////

// keeps sprites on the screen
inline void constrainPos(short *x, short *y, int xOffset, int yOffset) {
    if(*x < 0) {
        *x = 0;
    }
    if(*x > SCREEN_W - xOffset) {
        *x = SCREEN_W - xOffset;
    }
    if(*y < 0) {
        *y = 0;
    }
    if(*y > SCREEN_H - yOffset) {
        *y = SCREEN_H - yOffset;
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

    // consoleDemoInit();

	u16 keysHeldVar;
    u16 keysUpVar;
    u16 keysDownVar;
    u16 keysDownRepeatVar;

    short shipPosX = SCREEN_W / 2 - 16;
    short shipPosY = SCREEN_H; // this works fine because we are constraining the position
    short projectilePosX = 0;
    short projectilePosY = 0;
    short brickX = 0;
    short brickDirection = 1; // 1 for right-moving, -1 for left

    short moveSpeed = 2;
    short isMosaic = 0;
    short projectileIsInvisible = 0;
    short projectileIsFired = 0;
    short brickIsInvisible = 0;

    setRepeat(10, 3);

    // copy palette and tile info to VRAM

    // ship
    memcpy(MEM_PALETTE, ship0Pal,  ship0PalLen);
    memcpy(&MEM_TILE[4][1], ship0Tiles, ship0TilesLen);
    // projectile
    memcpy(MEM_PALETTE, testProjectilePal, testProjectilePalLen);
    memcpy(&MEM_TILE[4][5], testProjectileTiles, testProjectileTilesLen);
    // brick
    memcpy(MEM_PALETTE, brickPal, brickPalLen);
    memcpy(&MEM_TILE[4][6], brickTiles, brickTilesLen);

    // instatiate sprite 'objects' in OAM

    volatile ObjectAttributes *shipAttribs = &MEM_OAM[0];

    shipAttribs->attr0 = 0x2000; // 8bpp tiles, SQUARE shape
    shipAttribs->attr1 = 0x4000; // 16x16 size when using the SQUARE shape
    shipAttribs->attr2 = 2;      // Start at the first tile in tile

    volatile ObjectAttributes *projectileAttribs = &MEM_OAM[1];

    projectileAttribs->attr0 = 0x2000;
    projectileAttribs->attr1 = 0x0000; 
    projectileAttribs->attr2 = 10;

    volatile ObjectAttributes *brickAttribs = &MEM_OAM[2];

    brickAttribs->attr0 = 0x6000; // 8bpp tiles, Wide shape
    brickAttribs->attr1 = 0x8000; // 16x32
    brickAttribs->attr2 = 12;

    REG_DISPLAYCONTROL =  VIDEOMODE_0 | ENABLE_OBJECTS | MAPPINGMODE_1D;

    //set mosaic object stretch to 3x3
    REG_MOSAIC = 0x1200;
    //LOOP///////////////////////////////////////////////////////////////////////
	while (1) {

        scanKeys();
        keysHeldVar = keysHeld();
        keysUpVar = keysUp();
        keysDownVar = keysDown();
        keysDownRepeatVar = keysDownRepeat();

        if(keysDownVar & KEY_A) {
            projectileIsFired = 1;
        }
        if(keysDownVar & KEY_B) {
            projectileIsFired = 0;
            brickIsInvisible = 0;
        }
		
        if(keysHeldVar & KEY_A){
        } else {
        }
        if(keysHeldVar & KEY_B){
        }
        if(keysHeldVar & KEY_L){
            isMosaic = 1;
        } else {
            isMosaic = 0;
        }
        if(keysHeldVar & KEY_R){
        }
        if(keysHeldVar & KEY_START){
        }
        if(keysHeldVar & KEY_SELECT){
        }
        if(keysHeldVar & KEY_UP){
            shipPosY -= moveSpeed ;
        }
        if(keysHeldVar & KEY_DOWN){
            shipPosY += moveSpeed;
        }
        if(keysHeldVar & KEY_LEFT){
            shipPosX -= moveSpeed;
        }
        if(keysHeldVar & KEY_RIGHT){
            shipPosX += moveSpeed;
        }
        
        // if(keysDownRepeatVar & KEY_UP){
        //     shipPosY -= moveSpeed ;
        // }
        // if(keysDownRepeatVar & KEY_DOWN){
        //     shipPosY += moveSpeed;
        // }
        // if(keysDownRepeatVar & KEY_LEFT){
        //     shipPosX -= moveSpeed;
        // }
        // if(keysDownRepeatVar & KEY_RIGHT){
        //     shipPosX += moveSpeed;
        // }

        constrainPos(&shipPosX, &shipPosY, 16, 16);

        if(!projectileIsFired) {
            // keep projectile with the ship and hidden behind it while not fired
            // probably don't need to do this, just set position upon firing
            projectilePosX = shipPosX + 4;
            projectilePosY = shipPosY;
        } else {
            // keep projectile visible and move it up the screen
            projectileIsInvisible = 0;
            projectilePosY -= 3;
        }

        if(projectilePosY == 0) {
            // when it hits the top, hide it again
            projectileIsFired = 0;
            projectileIsInvisible = 1;
        }

        //move brick back and forth across top of the screen
        if(brickDirection == 1 && brickX < SCREEN_W - 32) {
            brickX++;
        } else if(brickDirection == 1 && brickX >= SCREEN_W - 32) {
            brickDirection = -1;
            brickX = SCREEN_W - 32;
        } else if(brickDirection == -1 && brickX > 0) {
            brickX--;
        } else if(brickDirection == -1 && brickX <= 0) {
            brickDirection = 1;
            brickX = 0;
        }

        if(projectilePosX + 6 >= brickX && projectilePosX <= brickX + 30) {
            //potential collision
            if(projectilePosY <= 12) {
                projectileIsInvisible = 1;
                brickIsInvisible = 1;
                projectileIsFired = 0;
            }
        }

        // TO DO: Find definitions for attributes and use those instead of raw hex
        // attributes define the visible properties of objects (sprites stored in OAM)
        shipAttribs->attr1 = 0x4000 | shipPosX;
        shipAttribs->attr0 = 0x2000 | ((isMosaic ? 0x1000 : 0) + shipPosY);

        projectileAttribs->attr1 = 0x0000 | projectilePosX;
        projectileAttribs->attr0 = 0x2000 | ((projectileIsInvisible ? 0x200 : 0) + projectilePosY);

        brickAttribs->attr1 = 0x8000 | brickX;
        brickAttribs->attr0 = 0x6000 | (brickIsInvisible ? 0x200 : 0);

        VBlankIntrWait();

	}
}