#include <gb/gb.h>
#include <stdint.h>

//#include "GFX_HUD.c"
//#include "Map_HUD.c"
#include "Screen_GameOver.c"

extern const uint8_t GFX_Censor_tiles[80];
//#include "GFX_Censor.c"

extern uint8_t gameState;
extern uint16_t score;

extern BOOLEAN cheat;

#define GAMESTATE_INTRO 0
#define GAMESTATE_TITLE 1
#define GAMESTATE_GAME 2
#define GAMESTATE_GAMEOVER 3

void init_gameOver(void) {
	DISPLAY_OFF;
    init_bkg(0);
    //set_bkg_data(0, 44, );
	//set_bkg_tiles(0, 0, Map_HUDWidth, Map_HUDHeight, Map_HUD);
    move_bkg(0, 0);
    HIDE_WIN;
    gameState = GAMESTATE_GAMEOVER;
	
	for (uint8_t i = 0; i < MAX_HARDWARE_SPRITES; i++) {
		move_sprite(i, 0, 0);
	}
	
	set_bkg_data(0, 128, Screen_GameOver_tiles);
	set_bkg_tiles(0, 0, 20, 18, Screen_GameOver_map);
	
	if (cheat) set_bkg_data(0x11, 1, GFX_Censor_tiles+16*3);
	
	uint16_t number = score;
	uint8_t ones = 0;
	uint8_t tens = 0;
	uint8_t hundreds = 0;
	/*uint8_t thousands = 0;
	
	while (number >= 1000) {
		thousands++;
		number -= 1000;
	}*/
	
	while (number >= 100) {
		hundreds++;
		number -= 100;
	}
	
	while (number >= 10) {
		tens++;
		number -= 10;
	}
	
	while (number >= 1) {
		ones++;
		number -= 1;
	}
	
	uint8_t sX = 0x02;
	uint8_t sY = 0x0C;
	
	set_bkg_tile_xy(sX, sY, hundreds+0x6A);
	set_bkg_tile_xy(sX+1, sY, tens+0x6A);
	set_bkg_tile_xy(sX+2, sY, ones+0x6A);
	
	if (score < 100) {
		set_bkg_tile_xy(sX, sY, 0);
	}
	if (score < 10) {
		set_bkg_tile_xy(sX+1, sY, 0);
	}
	
	DISPLAY_ON;
    //printf("Jasper: The Game\n\nTitle screen goes\nhere.\n\nPress Start");
}