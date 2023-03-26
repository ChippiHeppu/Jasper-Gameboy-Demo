#include <gb/gb.h>
#include <stdint.h>
//#include <stdio.h>

//#include "tileGFX.c"
#include "testMap.c"

#include "intro.c"
#include "titleScreen.c"
#include "gameOver.c"
#include "game.c"

#include "GFX_Censor.c"

#define GAMESTATE_INTRO 0
#define GAMESTATE_TITLE 1
#define GAMESTATE_GAME 2
#define GAMESTATE_GAMEOVER 3

const uint8_t sine[] = {
	128, 131, 134, 137, 140, 143, 146, 149,
	152, 156, 159, 162, 165, 168, 171, 174,
	176, 179, 182, 185, 188, 191, 193, 196,
	199, 201, 204, 206, 209, 211, 213, 216,
	218, 220, 222, 224, 226, 228, 230, 232,
	234, 236, 237, 239, 240, 242, 243, 245,
	246, 247, 248, 249, 250, 251, 252, 252,
	253, 254, 254, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 254, 254,
	253, 252, 252, 251, 250, 249, 248, 247,
	246, 245, 243, 242, 240, 239, 237, 236,
	234, 232, 230, 228, 226, 224, 222, 220,
	218, 216, 213, 211, 209, 206, 204, 201,
	199, 196, 193, 191, 188, 185, 182, 179,
	176, 174, 171, 168, 165, 162, 159, 156,
	152, 149, 146, 143, 140, 137, 134, 131,
	128, 124, 121, 118, 115, 112, 109, 106,
	103, 99 , 96 , 93 , 90 , 87 , 84 , 81 ,
	79 , 76 , 73 , 70 , 67 , 64 , 62 , 59 ,
	56 , 54 , 51 , 49 , 46 , 44 , 42 , 39 ,
	37 , 35 , 33 , 31 , 29 , 27 , 25 , 23 ,
	21 , 19 , 18 , 16 , 15 , 13 , 12 , 10 ,
	9  , 8  , 7  , 6  , 5  , 4  , 3  , 3  ,
	2  , 1  , 1  , 0  , 0  , 0  , 0  , 0  ,
	0  , 0  , 0  , 0  , 0  , 0  , 1  , 1  ,
	2  , 3  , 3  , 4  , 5  , 6  , 7  , 8  ,
	9  , 10 , 12 , 13 , 15 , 16 , 18 , 19 ,
	21 , 23 , 25 , 27 , 29 , 31 , 33 , 35 ,
	37 , 39 , 42 , 44 , 46 , 49 , 51 , 54 ,
	56 , 59 , 62 , 64 , 67 , 70 , 73 , 76 ,
	78 , 81 , 84 , 87 , 90 , 93 , 96 , 99 ,
	103, 106, 109, 112, 115, 118, 121, 124
};

joypads_t joypads;

BOOLEAN holdStart = FALSE;
BOOLEAN holdSelect = FALSE;
BOOLEAN holdUp = FALSE;
BOOLEAN holdDown = FALSE;
BOOLEAN holdLeft = FALSE;
BOOLEAN holdRight = FALSE;
BOOLEAN holdA = FALSE;
BOOLEAN holdB = FALSE;

uint8_t gameState;
uint8_t nextState;
uint8_t introFlash = 0;
uint8_t t = 0;
uint8_t winY = 128;
uint8_t fade = 30;
BOOLEAN fadeOn = TRUE;
BOOLEAN fadeDir = FALSE;

const uint8_t cheatCode[] = { J_UP, J_UP, J_DOWN, J_DOWN, J_LEFT, J_RIGHT, J_LEFT, J_RIGHT, J_B, J_A };
uint8_t cheatInput = 0;
BOOLEAN cheat = FALSE;

void scanline_isr() {
	
	if (gameState == GAMESTATE_INTRO) {
		if (introFlash >= 63-16 && introFlash < 76) {
			if (LYC_REG == 0) {
				BGP_REG = 0b11100100;
				LYC_REG = introFlash;
			} else if (LYC_REG == introFlash) {
				BGP_REG = 0b10000000;
				LYC_REG = introFlash+16;
			} else if (LYC_REG == introFlash+16) {
				BGP_REG = 0b11100100;
				LYC_REG = 0;
			}
		} else LYC_REG = 0;
		
		
	} else if (gameState == GAMESTATE_TITLE) {
		switch (LYC_REG) {
		case 0:
			SCX_REG = sine[(uint8_t)(t*4)]/16-8;
			SCY_REG = 0;
			LYC_REG = 31;
			break;
		case 31:
			SCX_REG = 0;
			SCY_REG = sine[(uint8_t)(t*8)]/64-2;
			LYC_REG = 114;
			break;
		case 114:
			SCX_REG = -sine[(uint8_t)(t*4)]/16+8;
			SCY_REG = 0;
			LYC_REG = 136;
			break;
		case 136:
			SCX_REG = LYC_REG = 0;
			SCY_REG = LYC_REG = 0;
			break;
		}
	} else if (gameState == GAMESTATE_GAME) {
		/*switch (LYC_REG) {
		case 0:
			SHOW_SPRITES;
			LYC_REG = winY-1;
			break;
		case winY-1:
			HIDE_SPRITES;
			LYC_REG = 0;
			break;
		}*/
		if (LYC_REG == 0) {
			SHOW_SPRITES;
			LYC_REG = winY-1;
		} else if (LYC_REG == winY-1) {
			HIDE_SPRITES;
			LYC_REG = 0;
		}
	} else if (gameState == GAMESTATE_GAMEOVER) {
		switch (LYC_REG) {
		case 0:
			SCX_REG = sine[(uint8_t)(t*2)]/32-4;
			SCY_REG = -sine[(uint8_t)(t*4)]/64;
			LYC_REG = 31;
			break;
		case 31:
			SCX_REG = LYC_REG = 0;
			SCY_REG = LYC_REG = 0;
			break;
		}
	}
}

void setGameState(uint8_t state) {
	fadeOn = TRUE;
	nextState = state;
	/*if (fade == 30) {
		switch (nextState) {
			case GAMESTATE_INTRO:
				// not implemented yet
				break;
			case GAMESTATE_TITLE:
				init_title();
				break;
			case GAMESTATE_GAME:
				init_game(t);
				break;
			case GAMESTATE_GAMEOVER:
				init_gameOver();
				break;
		}
	}*/
}

void soundEffect(uint8_t id) {
	switch(id) {
		case 0: // Collect H
			NR10_REG = 0x64;
			NR11_REG = 0x00;
			NR12_REG = 0xF3;
			NR13_REG = 0x73;
			NR14_REG = 0x86;
			break;
		case 1: // Collect Special
			NR10_REG = 0x65;
			NR11_REG = 0x80;
			NR12_REG = 0xF3;
			NR13_REG = 0x73;
			NR14_REG = 0x86;
			break;
		case 2: // Take Damage
			NR41_REG = 0x3F;
			NR42_REG = 0xF2;
			NR43_REG = 0x37;
			NR44_REG = 0x80;
			break;
		case 3: // Activate Special
			NR10_REG = 0x17;
			NR11_REG = 0x80;
			NR12_REG = 0xF7;
			NR13_REG = 0x1B;
			NR14_REG = 0x84;
			break;
		case 4: // Fart
			NR10_REG = 0x7B;
			NR11_REG = 0x00;
			NR12_REG = 0xF3;
			NR13_REG = 0x1B;
			NR14_REG = 0x84;
			break;
		case 5: // Jump
			NR41_REG = 0x00;
			NR42_REG = 0x0A;
			NR43_REG = 0x07;
			NR44_REG = 0xC0;
			break;
		case 6: // Collect Life
			NR10_REG = 0x77;
			NR11_REG = 0x80;
			NR12_REG = 0xF4;
			NR13_REG = 0x3B;
			NR14_REG = 0x87;
			break;
		case 7: // Death
			NR10_REG = 0x7B;
			NR11_REG = 0x80;
			NR12_REG = 0xF5;
			NR13_REG = 0x1B;
			NR14_REG = 0x84;
			break;
	}
	
	// Channel 1 (Pulse)
	/*NR10_REG = 0b00000000; // Sweep
	NR11_REG = 0b00000000; // Sound length/Wave pattern duty
	NR12_REG = 0b00000000; // Volume Envelope
	NR13_REG = 0b00000000; // Frequency Low
	NR14_REG = 0b00000000; // Frequency High
	
	// Channel 2 (Pulse)
	NR21_REG = 0b00000000; // Tone
	NR22_REG = 0b00000000; // Volume Envelope
	NR23_REG = 0b00000000; // Frequency data Low
	NR24_REG = 0b00000000; // Frequency data High
	
	// Channel 3 (Wave)
	NR30_REG = 0b00000000; // On/Off
	NR31_REG = 0b00000000; // Sound length
	NR32_REG = 0b00000000; // Select output level
	NR33_REG = 0b00000000; // Frequency data Low
	NR34_REG = 0b00000000; // Frequency data High
	
	// Channel 4 (Noise)
	NR41_REG = 0b00000000; // Sound length
	NR42_REG = 0b00000000; // Volume Envelope
	NR43_REG = 0b00000000; // Polynomial Counter
	NR44_REG = 0b00000000; // Counter / Consecutive and Inital
	
	// Control
	NR50_REG = 0b00000000; // Sound Channel control / ON-OFF / Volume 
	NR51_REG = 0b11111111; // Sound Selection of Sound output terminal
	NR52_REG = 0b00001000; // Sound Master on/off*/
}

void cheatKey(uint8_t b) {
	if (b == cheatCode[cheatInput] && !cheat) cheatInput++;
	else cheatInput = 0;
	
	if (cheatInput == 10 && !cheat) {
		soundEffect(6);
		cheat = TRUE;
		set_bkg_data(0x15, 1, GFX_Censor_tiles);
		set_bkg_data(0x1C, 1, GFX_Censor_tiles+16);
		
		set_bkg_tile_xy(0x03, 0x0F, 0x77);
		set_bkg_tile_xy(0x04, 0x0F, 0x66);
		set_bkg_tile_xy(0x05, 0x0F, 0x78);
		set_bkg_tile_xy(0x06, 0x0F, 0x67);
		set_bkg_tile_xy(0x07, 0x0F, 0x6A);
		set_bkg_tile_xy(0x08, 0x0F, 0x65);
		set_bkg_tile_xy(0x09, 0x0F, 0x66);
		set_bkg_tile_xy(0x0A, 0x0F, 0x79);
		set_bkg_tile_xy(0x0B, 0x0F, 0x00);
		set_bkg_tile_xy(0x0C, 0x0F, 0x7A);
		set_bkg_tile_xy(0x0D, 0x0F, 0x6A);
		set_bkg_tile_xy(0x0E, 0x0F, 0x79);
		set_bkg_tile_xy(0x0F, 0x0F, 0x66);
		
		set_bkg_tile_xy(0x08, 0x10, 0x66);
		set_bkg_tile_xy(0x09, 0x10, 0x78);
		set_bkg_tile_xy(0x0A, 0x10, 0x69);
		set_bkg_tile_xy(0x0B, 0x10, 0x7B);
		set_bkg_tile_xy(0x0C, 0x10, 0x6B);
		set_bkg_tile_xy(0x0D, 0x10, 0x66);
		set_bkg_tile_xy(0x0E, 0x10, 0x79);
	}
}

void main(void)
{
	//init_bkg(0);
	//CRITICAL {
		/*STAT_REG = STATF_MODE01 | STATF_MODE00;
		add_LCD(scanline_isr);
	//}
	
	set_interrupts(VBL_IFLAG | LCD_IFLAG);*/
	
	CRITICAL {
		STAT_REG |= STATF_LYC; LYC_REG = 0;
		add_LCD(scanline_isr);
	}
	set_interrupts(VBL_IFLAG | LCD_IFLAG);
	
	joypad_init(1, &joypads);
	
	//set_bkg_data(0, 4, TileGeneric);
	//set_bkg_tiles(0, 0, TestMapWidth, TestMapHeight, TestMap);
	
	SHOW_BKG;		// Enable background
	//DISPLAY_ON; 	// Turn on the display
	
	//uint8_t desiredWinY = 128;
	
	// Init sound channels
	NR52_REG = 0x80; // Sound Master on/off
	NR51_REG = 0xFF; // Sound Selection of Sound output terminal
	NR50_REG = 0x77; // Sound Channel control / ON-OFF / Volume 

	init_intro();
	
	BGP_REG = 0b00000000;	// TODO: Remove these
	OBP0_REG = 0b00000000;
	OBP1_REG = 0b00000000;
	
    // Loop forever
    while(1) {
		joypad_ex(&joypads);
		
		/*if (fadeOn) switch (fade) {
			case 0:
				BGP_REG = 0b11100100;
				OBP0_REG = 0b11100100;
				OBP1_REG = 0b00011011;
				break;
			case 10:
				BGP_REG = 0b10010000;
				OBP0_REG = 0b10010000;
				OBP1_REG = 0b00000110;
				break;
			case 20:
				BGP_REG = 0b01000000;
				OBP0_REG = 0b01000000;
				OBP1_REG = 0b00000001;
				break;
			case 30:
				BGP_REG = 0b00000000;
				OBP0_REG = 0b00000000;
				OBP1_REG = 0b00000000;
				break;
		}*/
		// Game main loop processing goes here
		
		//scroll_bkg(sine[t]/32-128/32, sine[t+(uint8_t)64]/32-128/32);
		//move_bkg(sine[t]/16, sine[(uint8_t)(t+64)]/16);

		// Done processing, yield CPU and wait for start of next frame
        //t++;
		//t++;

		switch(gameState) {
			case GAMESTATE_INTRO:
			process_intro();
				break;
			case GAMESTATE_TITLE:
				
				if (joypads.joy0 & J_START && !holdStart && !fadeOn) {
					//fadeOn = TRUE;
					setGameState(GAMESTATE_GAME);
					soundEffect(3);
				}
				
				if (!fadeOn) {
					if (joypads.joy0 & J_UP && !holdUp) cheatKey(J_UP);
					if (joypads.joy0 & J_DOWN && !holdDown) cheatKey(J_DOWN);
					if (joypads.joy0 & J_LEFT && !holdLeft) cheatKey(J_LEFT);
					if (joypads.joy0 & J_RIGHT && !holdRight) cheatKey(J_RIGHT);
					if (joypads.joy0 & J_B && !holdB) cheatKey(J_B);
					if (joypads.joy0 & J_A && !holdA) cheatKey(J_A);
					if (joypads.joy0 & J_SELECT && !holdSelect) cheatKey(J_SELECT);
				}
				//if (fade == 30) init_game(t);
				
				break;
			case GAMESTATE_GAME:
				process_game();
				break;
			case GAMESTATE_GAMEOVER:
				if (joypads.joy0 & J_START && !holdStart && !fadeOn) {
					/*set_bkg_data(0, 4, TileGeneric);
					set_bkg_tiles(0, 0, Level0Width, Level0Height, Level0);
					set_win_data(0, 44, GFX_Hud);
					set_win_tiles(0, 0, 20, 6, Map_HUD);
					move_win(7, 128);
					SHOW_WIN;*/
					setGameState(GAMESTATE_TITLE);
					//gameState = GAMESTATE_GAME;
				}
				break;
		}

		if (joypads.joy0 & J_START) holdStart = TRUE; else holdStart = FALSE;
		if (joypads.joy0 & J_SELECT) holdSelect = TRUE; else holdSelect = FALSE;
		if (joypads.joy0 & J_UP) holdUp = TRUE; else holdUp = FALSE;
		if (joypads.joy0 & J_DOWN) holdDown = TRUE; else holdDown = FALSE;
		if (joypads.joy0 & J_LEFT) holdLeft = TRUE; else holdLeft = FALSE;
		if (joypads.joy0 & J_RIGHT) holdRight = TRUE; else holdRight = FALSE;
		if (joypads.joy0 & J_A) holdA = TRUE; else holdA = FALSE;
		if (joypads.joy0 & J_B) holdB = TRUE; else holdB = FALSE;
		
		if (fadeOn) {
			if (fadeDir) fade++;
			else fade--;
			if (fade == 0) { fadeOn = FALSE; fadeDir = TRUE;}
			if (fade == 30) {
				
				switch (nextState) {
					case GAMESTATE_INTRO:
						init_intro();
						break;
					case GAMESTATE_TITLE:
						init_title();
						break;
					case GAMESTATE_GAME:
						init_game(t);
						break;
					case GAMESTATE_GAMEOVER:
						init_gameOver();
						break;
				}
				
				fadeDir = FALSE;
			}
			
			switch (fade) {
				case 0:
					BGP_REG = 0b11100100;
					OBP0_REG = 0b11100100;
					OBP1_REG = 0b00011011;
					break;
				case 10:
					BGP_REG = 0b10010000;
					OBP0_REG = 0b10010000;
					OBP1_REG = 0b00000110;
					break;
				case 20:
					BGP_REG = 0b01000000;
					OBP0_REG = 0b01000000;
					OBP1_REG = 0b00000001;
					break;
				case 30:
					BGP_REG = 0b00000000;
					OBP0_REG = 0b00000000;
					OBP1_REG = 0b00000000;
					break;
			}
		}
		
		//scroll_bkg(1, 0);
		t++;
		wait_vbl_done();
    }
}
