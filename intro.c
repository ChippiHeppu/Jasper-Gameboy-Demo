#include <gb/gb.h>
#include <stdint.h>

#include "Screen_Intro.c"
#include "GBDK_2020_logo.c"
//#include "GFX_HUD.c"
//#include "Map_HUD.c"
//#include "Screen_GameOver.c"

extern uint8_t gameState;
extern uint8_t introFlash;
extern void soundEffect(uint8_t id);

#define GAMESTATE_INTRO 0
#define GAMESTATE_TITLE 1
#define GAMESTATE_GAME 2
#define GAMESTATE_GAMEOVER 3

extern void setGameState(uint8_t state);

/*const uint8_t scrollAnim[9] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 2, 2, 2, 2, 1, 1, 1, 1};*/
uint8_t animTime;

void init_intro(void) {
	DISPLAY_OFF;
    init_bkg(0);
    set_bkg_data(0, 44, Screen_Intro_tiles);
	set_bkg_tiles(0, 0, 20, 18, Screen_Intro_map);
    HIDE_WIN;
	SHOW_SPRITES;
    gameState = GAMESTATE_INTRO;
	animTime = 0;
	//LYC_REG = 67;
	
	/*for (uint8_t i = 0; i < MAX_HARDWARE_SPRITES; i++) {
		move_sprite(i, 0, 0);
	}*/
	
	/*for (uint8_t i = 0; i < 13; i++) {
		set_sprite_tile(i, i);
		set_sprite_tile(i+13, i+13);
		move_sprite(i, i*8+8, 16);
		move_sprite(i+13, i*8+8, 24);
	}*/
	
	//set_bkg_data(0, 128, Screen_GameOver_tiles);
	//set_bkg_tiles(0, 0, 20, 18, Screen_GameOver_map);
	DISPLAY_ON;
    //printf("Jasper: The Game\n\nTitle screen goes\nhere.\n\nPress Start");
}

void process_intro(void) {
	if (animTime < 30) {
		move_bkg(0, -29+animTime);
	} else if (animTime < 80) {
		introFlash = (45-16)+(animTime-30)*2;
	}
	animTime++;
	
	if (animTime == 40) soundEffect(1);
	
	//if (animTime == 90) setGameState(GAMESTATE_TITLE);
	if (animTime == 90) setGameState(255);
	
	if (animTime >= 90 && animTime < 120) {
		move_bkg(0, (animTime-90));
	}
	
	if (animTime == 120) {
		move_bkg(0, 0);
		set_bkg_data(0, 39, GBDK_2020_logo_tiles);
		set_bkg_tiles(0, 0, 20, 18, GBDK_2020_logo_map);
	}
	
	if (animTime == 180) setGameState(GAMESTATE_TITLE);
	//introFlash++;
}