#include <gb/gb.h>
#include <stdint.h>

//#include "GFX_HUD.c"
//#include "Map_HUD.c"
#include "Screen_Title.c"

extern const uint8_t GFX_Censor_tiles[80];
//#include "GFX_Censor.c"

extern uint8_t gameState;

extern BOOLEAN cheat;

#define GAMESTATE_INTRO 0
#define GAMESTATE_TITLE 1
#define GAMESTATE_GAME 2
#define GAMESTATE_GAMEOVER 3

void init_title(void) {
	DISPLAY_OFF;
    init_bkg(0);
    //set_bkg_data(0, 44, );
	//set_bkg_tiles(0, 0, Map_HUDWidth, Map_HUDHeight, Map_HUD);
    move_bkg(0, 0);
    HIDE_WIN;
    gameState = GAMESTATE_TITLE;
	
	for (uint8_t i = 0; i < MAX_HARDWARE_SPRITES; i++) {
		move_sprite(i, 0, 0);
	}
	
	set_bkg_data(0, 128, Screen_Title_tiles);
	set_bkg_tiles(0, 0, 20, 18, Screen_Title_map);
	
	if (cheat) {
		set_bkg_data(0x15, 1, GFX_Censor_tiles);
		set_bkg_data(0x1C, 1, GFX_Censor_tiles+16);
	}
	
	DISPLAY_ON;
    //printf("Jasper: The Game\n\nTitle screen goes\nhere.\n\nPress Start");
}