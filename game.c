#include <gb/gb.h>
#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <rand.h>

#include "GFX_HUD.c"
#include "Map_HUD.c"

#include "Sprite_Jasper.c"
//#include "Sprite_Jasper_White.c"
#include "Sprite_Eyes.c"
#include "Sprite_H.c"

#include "GFX_Level0.c"
#include "Map_Level0.c"

extern const uint8_t GFX_Censor_tiles[80];
//#include "GFX_Censor.c"

/*#include "GFX_HUD.c"
#include "Map_HUD.c"*/

#define GAMESTATE_INTRO 0
#define GAMESTATE_TITLE 1
#define GAMESTATE_GAME 2
#define GAMESTATE_GAMEOVER 3

#define TYPE_H 0
#define TYPE_G 1
#define TYPE_LIFE 2
#define TYPE_SUPERH 3
#define TYPE_DISAPPEAR 4

#define PLAYERANIM_SIDE 0
#define PLAYERANIM_FRONT 4
#define PLAYERANIM_BACK 8
#define PLAYERANIM_DEAD 12

extern joypads_t joypads;

extern BOOLEAN holdStart;
extern BOOLEAN holdSelect;
extern BOOLEAN holdUp;
extern BOOLEAN holdDown;
extern BOOLEAN holdLeft;
extern BOOLEAN holdRight;
extern uint8_t gameState;
extern uint8_t winY;
extern uint8_t fade;
extern BOOLEAN fadeOn;

extern BOOLEAN cheat;

extern void setGameState(uint8_t state);
extern void soundEffect(uint8_t id);

//int8_t xVel = 0;
//int8_t yVel = 0;

uint16_t playerX, playerY, playerAccel;
int8_t playerZ;
int16_t playerXVel, playerYVel, playerSpeed;
uint8_t playerAnimSpeed;
uint8_t playerAnimFrame;
uint8_t playerAnimTimer;
uint8_t playerAnimState;
uint16_t playerIFrames;
uint8_t playerIFramesFlashPhase;
BOOLEAN playerFlip;
BOOLEAN playerDead;
uint8_t deathTimer;
BOOLEAN specialAttack;
uint8_t specialAttackTimer;
//uint8_t playerEyeX;
//uint8_t playerEyeY;
uint16_t score;
uint8_t lives, special;
uint16_t camX, camY;

const int8_t specialJump[] = {0, 3, 7, 10, 13, 16, 19, 21, 24, 26, 28, 29, 30, 31, 32, 32, 32, 31, 30, 29, 28, 26, 24, 21, 19, 16, 13, 10, 7, 3};

BOOLEAN pause = FALSE;

BOOLEAN pauseTransition = FALSE;


uint8_t objectSpawnInterval = 60;
uint8_t objectSpawnTimer = 0;

#define OBJECT_MAX 12
#define OBJECT_PROCESS_MAX 4

uint8_t objectProcessCounter = 0;

struct {
	uint16_t x, y;
	int16_t xVel, yVel;
	int16_t maxVel;
	uint8_t w, h;
	uint8_t type;
	uint16_t lifetime;
	BOOLEAN active;
	BOOLEAN homing;
	uint8_t animSpeed;
	uint8_t animFrame;
	uint8_t frameTime;
	uint8_t specialFlash;
} Objects[OBJECT_MAX];

uint8_t RandomRangeU8(uint8_t min, uint8_t max) {
	return (rand() % (max - min + 1)) + min;
}

uint16_t RandomRangeU16(uint16_t min, uint16_t max) {
	return (randw() % (max - min + 1)) + min;
}

int16_t RandomRange16(int16_t min, int16_t max) {
	return (randw() % (max - min + 1)) + min;
}

void UpdateLives(void) {
	if (lives > 4) {
		lives = 4;
	}
	for (uint8_t i = 0; i < 4; i++) {
		uint8_t tile;
		if (i >= lives) tile = 0x00;
		else tile = 0x2C;
		set_win_tile_xy(i, 1, tile);
	}
}

void UpdateScore(void) {
	
	if (score > 999) score = 999;
	
	// TODO: Add support for numbers larger than 999
	
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
	
	set_win_tile_xy(10, 0, hundreds*2+8);
	set_win_tile_xy(10, 1, hundreds*2+8+1);
	set_win_tile_xy(11, 0, tens*2+8);
	set_win_tile_xy(11, 1, tens*2+8+1);
	set_win_tile_xy(12, 0, ones*2+8);
	set_win_tile_xy(12, 1, ones*2+8+1);
	
	if (score < 100) {
		set_win_tile_xy(10, 0, 0);
		set_win_tile_xy(10, 1, 0);
	}
	if (score < 10) {
		set_win_tile_xy(11, 0, 0);
		set_win_tile_xy(11, 1, 0);
	}
}

void UpdateSpecial(void) {
	for (uint8_t i = 0; i < 5; i++) {
		if (i < special) {
			set_win_tile_xy(15+i, 0, 0x1F);
			set_win_tile_xy(15+i, 1, 0x20);
		} else {
			set_win_tile_xy(15+i, 0, 0x1D);
			set_win_tile_xy(15+i, 1, 0x1E);
		}
	}
}

void Objects_init(void) {
	for (uint8_t i = 0; i < (uint8_t)OBJECT_MAX; i++) {
		//Objects[i].x = rand()+128;
		//Objects[i].y = rand()+128;
		//Objects[i].x = 0x0080;
		//Objects[i].y = 0x0080;
		//Objects[i].xVel = (int16_t)(rand()/8);
		//Objects[i].yVel = (int16_t)(rand()/8);
		/*Objects[i].x = RandomRangeU16(0, 191);
		Objects[i].y = RandomRangeU16(0, 191);
		Objects[i].xVel = RandomRange16(-16, 16);
		Objects[i].yVel = RandomRange16(-16, 16);
		Objects[i].type = TYPE_H;
		Objects[i].active = TRUE;
		//Objects[i].animSpeed = rand()/32+5;
		Objects[i].animSpeed = RandomRangeU8(5,20);
		Objects[i].animFrame = 0;
		Objects[i].frameTime = Objects[i].animSpeed;*/
		
		Objects[i].x = 0;
		Objects[i].y = 0;
		Objects[i].xVel = 0;
		Objects[i].yVel = 0;
		Objects[i].maxVel = 0;
		Objects[i].type = TYPE_H;
		Objects[i].lifetime = 600;
		Objects[i].active = FALSE;
		Objects[i].homing = FALSE;
		Objects[i].animSpeed = 1;
		Objects[i].animFrame = 0;
		Objects[i].frameTime = Objects[i].animSpeed;
		Objects[i].specialFlash = 0;
	}
}

void Object_spawn(void) {
	//uint8_t side = RandomRangeU8(0, 3);
	//for (uint8_t i = 0; i < side+1; i++) printf("h");
	//printf(" ");
	for (uint8_t i = 0; i < OBJECT_MAX; i++) {
		if (!Objects[i].active) {
			
			// Decide which side of the screen the object will spawn from
			// 0: Top
			// 1: Bottom
			// 2: Left
			// 3: Right
			
			uint8_t side = RandomRangeU8(0, 3);
			
			switch(side) {
				case 0:
					Objects[i].x = RandomRangeU16(0, 191 << 4);
					Objects[i].y = 0 << 4;
					break;
				case 1:
					Objects[i].x = RandomRangeU16(0, 191 << 4);
					Objects[i].y = 191 << 4;
					break;
				case 2:
					Objects[i].x = 0 << 4;
					Objects[i].y = RandomRangeU16(0, 191 << 4);
					break;
				case 3:
					Objects[i].x = 191 << 4;
					Objects[i].y = RandomRangeU16(0, 191 << 4);
					break;
			}
			
			Objects[i].maxVel = RandomRange16(12, 48);
			Objects[i].xVel = RandomRange16(12, 48);
			Objects[i].yVel = RandomRange16(12, 48);
			Objects[i].homing = FALSE;
			
			if (Objects[i].type == TYPE_SUPERH) { Objects[i].xVel *= 2; Objects[i].yVel *= 2;};
			
			/*if (rand() < 128) {
				Objects[i].type = TYPE_H;
			} else {
				Objects[i].type = TYPE_G;
				if (rand() < 64) Objects[i].homing = TRUE;
			}*/
			
			uint8_t r = rand();
			
			/*if (r < 128) Objects[i].type = TYPE_H;
			else if (r < 192) { Objects[i].type = TYPE_G; if (rand() < 64) Objects[i].homing = TRUE; }
			else if (r < 224) Objects[i].type = TYPE_SUPERH;
			else if (r < 254 && lives < 4) Objects[i].type = TYPE_LIFE;
			else Objects[i].type = TYPE_H;*/
			
			if (r > 128) Objects[i].type = TYPE_H;
			else if (r > 32) { Objects[i].type = TYPE_G; if (rand() < 64) Objects[i].homing = TRUE; }
			else {
				if (lives < 4 && rand() < 32) Objects[i].type = TYPE_LIFE;
				else Objects[i].type = TYPE_SUPERH;
			}
			
			Objects[i].lifetime = 600;
			//Objects[i].type = TYPE_H;
			Objects[i].active = TRUE;
			Objects[i].animSpeed = RandomRangeU8(2,10);
			Objects[i].animFrame = 0;
			Objects[i].frameTime = Objects[i].animSpeed;
			
			break;
		}
	}
}

void Objects_process(void) {
	for (uint8_t i = objectProcessCounter; i < OBJECT_PROCESS_MAX + objectProcessCounter; i++) {
		if (Objects[i].active) {
			
			// Home in on the player if enabled
			
			if (Objects[i].homing) {
				if (Objects[i].x < playerX) Objects[i].xVel += 3;
				else Objects[i].xVel -= 3;
				
				if (Objects[i].y < playerY) Objects[i].yVel += 3;
				else Objects[i].yVel -= 3;
				
				if (Objects[i].xVel > Objects[i].maxVel) Objects[i].xVel = Objects[i].maxVel;
				else if (Objects[i].xVel < -Objects[i].maxVel) Objects[i].xVel = -Objects[i].maxVel;
				if (Objects[i].yVel > Objects[i].maxVel) Objects[i].yVel = Objects[i].maxVel;
				else if (Objects[i].yVel < -Objects[i].maxVel) Objects[i].yVel = -Objects[i].maxVel;
			}
			
			// Apply velocities
			
			Objects[i].x += Objects[i].xVel;
			Objects[i].y += Objects[i].yVel;
			
			// Collision check with the screen boundaries
			
			uint8_t roundX = Objects[i].x >> 4;
			uint8_t roundY = Objects[i].y >> 4;
			
			if (roundX > 224) {
				Objects[i].xVel = -Objects[i].xVel;
				Objects[i].x += Objects[i].xVel;
			}
			else if (roundX >= 192) {
				Objects[i].xVel = -Objects[i].xVel;
				Objects[i].x += Objects[i].xVel;
			}
			
			if (roundY > 224) {
				Objects[i].yVel = -Objects[i].yVel;
				Objects[i].y += Objects[i].yVel;
			}
			else if (roundY >= 192) {
				Objects[i].yVel = -Objects[i].yVel;
				Objects[i].y += Objects[i].yVel;
			}
			
			// Collision check with the player
			
			// Prevent the player's hitbox from wrapping around the top/left sides of the boundary
			uint16_t pX = playerX-(7 << 4); if (pX > 224 << 4) pX = 0;
			uint16_t pY = playerY-(7 << 4); if (pY > 224 << 4) pY = 0;
			
			if (Objects[i].x >= pX && Objects[i].x <= playerX+(8 << 4) && !playerDead) {
				if (Objects[i].y >= pY && Objects[i].y <= playerY+(8 << 4)) {
					
					switch (Objects[i].type) {
						case TYPE_H:
							score++;
							UpdateScore();
							
							Objects[i].active = FALSE;
							move_metasprite(Sprite_H_metasprites[Objects[i].animFrame], 62, 12+i, 0, 0);
							
							soundEffect(0);
							
							break;
						case TYPE_G:
							if (playerIFrames == 0) {
								lives--;
								UpdateLives();
								
								playerXVel += Objects[i].xVel*32;
								playerYVel += Objects[i].yVel*32;
								
								playerIFrames = 120;
								playerIFramesFlashPhase = 3;
								
								Objects[i].active = FALSE;
								move_metasprite(Sprite_H_metasprites[Objects[i].animFrame], 62, 12+i, 0, 0);
								
								soundEffect(2);
							}
							break;
						case TYPE_SUPERH:
							score += 2;
							UpdateScore();
							special++;
							UpdateSpecial();
							
							Objects[i].active = FALSE;
							move_metasprite(Sprite_H_metasprites[Objects[i].animFrame], 62, 12+i, 0, 0);
							
							soundEffect(1);
							
							break;
						case TYPE_LIFE:
							lives++;
							UpdateLives();
							
							Objects[i].active = FALSE;
							move_metasprite(Sprite_H_metasprites[Objects[i].animFrame], 62, 12+i, 0, 0);
							
							soundEffect(6);
							break;
					}
					//move_sprite(MAX_HARDWARE_SPRITES-1-i, 0, 0);
				}
			}
			
			// Animation
			
			Objects[i].frameTime--;
			if (Objects[i].frameTime == 0) {
				Objects[i].animFrame++;
				if (Objects[i].animFrame == 4) Objects[i].animFrame = 0;
				Objects[i].frameTime = Objects[i].animSpeed;
			}
			
			
			//move_metasprite(Sprite_H_metasprites[Objects[i].animFrame], 0, 0, playerX-camX+8, playerY-camY+16);
			
			//set_sprite_data(127-i, 1, Sprite_H_tiles+16*Objects[i].animFrame+(Objects[i].type*8)*4);
			//set_sprite_tile(MAX_HARDWARE_SPRITES-1-i, 127-i);
			
			Objects[i].lifetime--;
			if (Objects[i].lifetime == 0) {
				Objects[i].active = FALSE;
				move_metasprite(Sprite_H_metasprites[Objects[i].animFrame], 62, 12+i, 0, 0);
				//move_sprite(MAX_HARDWARE_SPRITES-1-i, 0, 0);
			}
			
			/*if (i == 0) {
				if (((uint8_t)(Objects[i].x >> 4) >= playerX-11 &&
				(uint8_t)(Objects[i].x >> 4) <= playerX+12) &&
				((uint8_t)(Objects[i].y >> 4) >= playerY-7 &&
				(uint8_t)(Objects[i].y >> 4) <= playerY+8)) {
				
					Objects[i].active = FALSE;
					move_sprite(127-i, 0, 0);
				}
			}*/
		}
	}
	
	objectProcessCounter += OBJECT_PROCESS_MAX;
	if (objectProcessCounter >= OBJECT_MAX) objectProcessCounter = 0;
}

void Objects_draw(void) {
	for (uint8_t i = 0; i < OBJECT_MAX; i++) {
		if (Objects[i].active) {
			int8_t spr = 0;
			
			switch(Objects[i].type) {
				case TYPE_H:
					spr = 0;
					break;
				case TYPE_G:
					if (cheat) spr = 16;
					else spr = 4;
					break;
				case TYPE_LIFE:
					spr = 8;
					break;
				case TYPE_SUPERH:
					spr = 12;
					break;
			}
			
			//move_metasprite(Sprite_H_metasprites[Objects[i].animFrame+spr], 45, 12+i, (uint8_t)(Objects[i].x >> 4)-camX+8, (uint8_t)(Objects[i].y >> 4)-camY+16);
			//move_metasprite(Sprite_H_metasprites[Objects[i].animFrame+spr], 45, 12+i, (uint8_t)((Objects[i].x - (camX+8))>>4), (uint8_t)((Objects[i].y - (camY+16))>>4));
			move_metasprite(Sprite_H_metasprites[Objects[i].animFrame+spr], 62, 12+i, ((Objects[i].x-camX) >> 4) + 8, ((Objects[i].y-camY) >> 4) + 16);
			
			switch(Objects[i].type) {
				case TYPE_H:
					set_sprite_prop(12+i, 0b00010000);
					break;
				case TYPE_G:
					set_sprite_prop(12+i, 0b00000000);
					break;
				case TYPE_LIFE:
					set_sprite_prop(12+i, 0b00010000);
					break;
				case TYPE_SUPERH:
					Objects[i].specialFlash++;
					if (Objects[i].specialFlash == 30) Objects[i].specialFlash = 0;
					if (Objects[i].specialFlash >= 2) set_sprite_prop(12+i, 0b00010000);
					else set_sprite_prop(12+i, 0b00000000);
					break;
			}
			
			//move_sprite(MAX_HARDWARE_SPRITES-1-i, (uint8_t)(Objects[i].x >> 4)-camX+8-4, (uint8_t)(Objects[i].y >> 4)-camY+16-4);
			
		}
	}
}

void init_game(uint8_t seed) {
	DISPLAY_OFF;
	gameState = GAMESTATE_GAME;
    set_bkg_data(0x2D, 12, Level0_Tiles);
	set_bkg_tiles(0, 0, Level0Width, Level0Height, Level0);
	set_win_data(0, 45, GFX_Hud);
	set_win_tiles(0, 0, 20, 6, Map_HUD);
	move_win(7, 128);
	
	if (cheat) {
		set_bkg_data(0x04, 1, GFX_Censor_tiles+16*2);
		set_bkg_data(0x1C, 1, GFX_Censor_tiles+16*4);
	}
	
	SHOW_WIN;
	SHOW_SPRITES;

	set_sprite_data(0, sizeof(Sprite_Jasper_tiles) >> 4, Sprite_Jasper_tiles);
	set_sprite_data(sizeof(Sprite_Jasper_tiles) >> 4, sizeof(Sprite_Eyes_tiles) >> 4, Sprite_Eyes_tiles);
	set_sprite_data((sizeof(Sprite_Jasper_tiles)+sizeof(Sprite_Eyes_tiles)) >> 4, sizeof(Sprite_H_tiles) >> 4, Sprite_H_tiles);
	SPRITES_8x16;
	
	set_sprite_tile(6, 0x3C);
	
	/*#if Sprite_Jasper_TILE_H == 16
		SPRITES_8x16;
	#else
		SPRITES_8x8;
	#endif*/
	
	playerX = (uint16_t)96 << 4;
	playerY = (uint16_t)96 << 4;
	//playerX = 0;
	//playerY = 0;
	playerXVel = 0;
	playerYVel = 0;
	playerAccel = 4;
	playerSpeed = 16;
	
	playerAnimFrame = 0;
	playerAnimSpeed = 8;
	playerAnimTimer = 8;
	playerAnimState = PLAYERANIM_FRONT;
	
	playerIFrames = 0;
	playerIFramesFlashPhase = 0;
	
	playerFlip = FALSE;
	playerDead = FALSE;
	deathTimer = 120;
	
	specialAttack = FALSE;
	specialAttackTimer = 0;
	
	lives = 4;
	score = 0;
	special = 0;
	
	/*camX = 16;
	camY = 32;*/
	
    pause = FALSE;
    pauseTransition = FALSE;
    winY = 128;
	
	initrand(seed);
	Objects_init();
	DISPLAY_ON;
	//for (uint8_t i = 0; i < OBJECT_MAX; i++) Object_spawn();
}

void process_game(void) {
	if (joypads.joy0 & J_START && !pause && !holdStart && !pauseTransition && !playerDead && !fadeOn) {
		pause = TRUE;
		pauseTransition = TRUE;
		// move_win(7, 93);
		// desiredWinY = 93;
	}
	else if (joypads.joy0 & J_START && pause && !holdStart && !pauseTransition && !fadeOn) {
		pause = FALSE;
		pauseTransition = TRUE;
		// move_win(7, 128);
		// desiredWinY = 128;
	}

	if (!pause && !pauseTransition && !specialAttack) {
		move_bkg((uint8_t)(camX >> 4), (uint8_t)(camY >> 4));
		if (joypads.joy0 & J_B) {
			playerSpeed = 32;
			playerAnimSpeed = 4;
		} else {
			playerSpeed = 16;
			playerAnimSpeed = 8;
		}
		
		if (joypads.joy0 & J_A && special >= 5) {
			soundEffect(3);
			special = 0;
			specialAttack = TRUE;
			specialAttackTimer = 120;
			BGP_REG = 0b00011011;
		}
		
		if (joypads.joy0 & J_LEFT && !playerDead) {
			playerXVel -= playerAccel;
		}
		else if (joypads.joy0 & J_RIGHT && !playerDead) {
			playerXVel += playerAccel;
		} else if (playerXVel > 0) playerXVel -= playerAccel; else if (playerXVel < 0) playerXVel += playerAccel;
		
		if (joypads.joy0 & J_UP && !playerDead) {
			playerYVel -= playerAccel;
		}
		else if (joypads.joy0 & J_DOWN && !playerDead) {
			playerYVel += playerAccel;
		} else if (playerYVel > 0) playerYVel -= playerAccel; else if (playerYVel < 0) playerYVel += playerAccel;
		
		if (playerXVel > playerSpeed) playerXVel = playerSpeed;
		else if (playerXVel < -playerSpeed) playerXVel = -playerSpeed;
		
		if (playerYVel > playerSpeed) playerYVel = playerSpeed;
		else if (playerYVel < -playerSpeed) playerYVel = -playerSpeed;
		
		playerX += playerXVel;
		playerY += playerYVel;
		
		if (joypads.joy0 & J_UP) {
			playerAnimState = PLAYERANIM_BACK;
			playerFlip = FALSE;
		}
		else if (joypads.joy0 & J_DOWN) {
			playerAnimState = PLAYERANIM_FRONT;
			playerFlip = FALSE;
		}
		if (joypads.joy0 & J_LEFT) {
			playerAnimState = PLAYERANIM_SIDE;
			playerFlip = TRUE;
		}
		else if (joypads.joy0 & J_RIGHT) {
			playerAnimState = PLAYERANIM_SIDE;
			playerFlip = FALSE;
		}
		
		// Screen boundary collision check
		
		uint8_t roundX = playerX >> 4;
		uint8_t roundY = playerY >> 4;
		
		if (roundX > 224) {
			playerXVel = -playerXVel;
			playerX += playerXVel;
		}
		else if (roundX >= 192) {
			playerXVel = -playerXVel;
			playerX += playerXVel;
		}
		
		if (roundY > 224) {
			playerYVel = -playerYVel;
			playerY += playerYVel;
		}
		else if (roundY >= 192) {
			playerYVel = -playerYVel;
			playerY += playerYVel;
		}
		
		if (playerX >> 4 > 224) {
			playerX = 0;
		} else if (playerX >> 4 >= 192) {
			playerX = 191 << 4;
		}
		
		if (playerY >> 4 > 224) {
			playerY = 0;
		} else if (playerY >> 4 >= 192) {
			playerY = 191 << 4;
		}
		
		camX = playerX - (80 << 4);
		camY = playerY - (72 << 4);
		
		if (camX >> 4 > 224) camX = 0;
		if (camX >> 4 > 32) camX = 32 << 4;
		if (camY >> 4 > 224) camY = 0;
		if (camY >> 4 > 64) camY = 64 << 4;
		
		if (objectSpawnTimer == 0) {
			Object_spawn();
			objectSpawnTimer = objectSpawnInterval;
		} else objectSpawnTimer--;
		
		playerAnimTimer--;
		if (playerAnimTimer == 0) {
			playerAnimTimer = playerAnimSpeed;
			playerAnimFrame++;
		}
		if (playerAnimFrame == 4) playerAnimFrame = 0;
		
		if (playerXVel == 0 && playerYVel == 0) {
			playerAnimFrame = 1;
		}
		if (playerIFrames > 0 && !playerDead) {
			playerIFramesFlashPhase++;
			playerIFrames--;
			if (playerIFramesFlashPhase == 4) playerIFramesFlashPhase = 0;
		}
		
		//move_bkg((uint8_t)(camX >> 4), (uint8_t)(camY >> 4));
		//move_metasprite(Sprite_Jasper_metasprites[playerAnimFrame+playerAnimState], 0, 0, (uint8_t)(playerX >> 4) - (uint8_t)(camX)+8, (uint8_t)(playerY >> 4) - (uint8_t)(camY)+16);
		if ((playerIFramesFlashPhase >= 2 || playerIFrames == 0) && !playerDead) {
			if (playerFlip) {
				//hide_metasprite(Sprite_Jasper_White_metasprites[playerAnimFrame+playerAnimState], 6);
				move_metasprite_vflip(Sprite_Jasper_metasprites[playerAnimFrame+playerAnimState], 0, 0, ((playerX-camX) >> 4) + 8, ((playerY-camY) >> 4) + 16);
				//move_metasprite_vflip(Sprite_Jasper_White_metasprites[playerAnimFrame+playerAnimState], 45, 6, ((playerX-camX) >> 4) + 8, ((playerY-camY) >> 4) + 16);
				//for (uint8_t i = 0; i < 6; i++) set_sprite_prop(6+i, 0b00110000);
				
			} else {
				//hide_metasprite(Sprite_Jasper_White_metasprites[playerAnimFrame+playerAnimState], 6);
				move_metasprite(Sprite_Jasper_metasprites[playerAnimFrame+playerAnimState], 0, 0, ((playerX-camX) >> 4) + 8, ((playerY-camY) >> 4) + 16);
				//move_metasprite(Sprite_Jasper_White_metasprites[playerAnimFrame+playerAnimState], 45, 6, ((playerX-camX) >> 4) + 8, ((playerY-camY) >> 4) + 16);
				//for (uint8_t i = 0; i < 6; i++) set_sprite_prop(6+i, 0b00010000);
			}
			set_sprite_prop(6, 0b00010000);
			switch (playerAnimState) {
				case PLAYERANIM_FRONT:
					if (playerAnimFrame == 1 || playerAnimFrame == 3) move_sprite(6, ((playerX-camX) >> 4) + 8 - 4, ((playerY-camY) >> 4) + 16 - 3);
					else move_sprite(6, ((playerX-camX) >> 4) + 8 - 4, ((playerY-camY) >> 4) + 16 - 2);
					break;
				case PLAYERANIM_BACK:
					move_sprite(6, 0, 0);
					break;
				case PLAYERANIM_SIDE:
					if (playerFlip) {
						set_sprite_prop(6, 0b00110000);
						if (playerAnimFrame == 1 || playerAnimFrame == 3) move_sprite(6, ((playerX-camX) >> 4) + 8 - 11, ((playerY-camY) >> 4) + 16 - 3);
						else move_sprite(6, ((playerX-camX) >> 4) + 8 - 11, ((playerY-camY) >> 4) + 16 - 2);
					} else {
						if (playerAnimFrame == 1 || playerAnimFrame == 3) move_sprite(6, ((playerX-camX) >> 4) + 8 + 3, ((playerY-camY) >> 4) + 16 - 3);
						else move_sprite(6, ((playerX-camX) >> 4) + 8 + 3, ((playerY-camY) >> 4) + 16 - 2);
					}
					break;
			}
		}
		else {
			if (!playerDead) {
				hide_metasprite(Sprite_Jasper_metasprites[playerAnimFrame+playerAnimState], 0);
				move_sprite(6, 0, 0);
			} else {
				//set_sprite_prop(6, 0b01010000);
				move_metasprite(Sprite_Jasper_metasprites[12], 0, 0, ((playerX-camX) >> 4) + 8, ((playerY-camY) >> 4) + 16);
				move_sprite(6, 0, 0);
				//move_metasprite_hflip(Sprite_Jasper_metasprites[0], 0, 0, ((playerX-camX) >> 4) + 8, ((playerY-camY) >> 4) + 16);
				//move_sprite(6, ((playerX-camX) >> 4) + 8 + 3, ((playerY-camY) >> 4) + 16 - 6);
			}
		}
		Objects_process();
		Objects_draw();
		
		if (playerDead) {
			deathTimer--;
			if (deathTimer == 0) setGameState(GAMESTATE_GAMEOVER);
		}
		
		// Collision check
		/*for (uint8_t i = 0; i < OBJECT_MAX; i++) {
			if (Objects[i].active) {
				if (((uint8_t)(Objects[i].x >> 4) >= playerX-11 &&
					(uint8_t)(Objects[i].x >> 4) <= playerX+12) &&
					((uint8_t)(Objects[i].y >> 4) >= playerY-7 &&
					(uint8_t)(Objects[i].y >> 4) <= playerY+8)) {
					
					Objects[i].active = FALSE;
				}
			}
		}*/
		
		//scroll_bkg(xVel, yVel);
	} else if (specialAttack) {
		specialAttackTimer--;
		
		switch (specialAttackTimer) {
			
			case 90:
				soundEffect(5);
				BGP_REG = 0b11100100;
				break;
				
			case 60:
				soundEffect(4);
				for (uint8_t i = 0; i < OBJECT_MAX; i++) {
					if (Objects[i].type == TYPE_G) Objects[i].type = TYPE_H;
					
					if (Objects[i].type == TYPE_H) {
						Objects[i].homing = TRUE;
						Objects[i].xVel = 0;
						Objects[i].yVel = 0;
						Objects[i].maxVel = 64;
						Objects[i].lifetime = 0xFFFF;
					}
				}
				Objects_draw();
				
				BGP_REG = 0b00000000;
				OBP0_REG = 0b00000000;
				OBP1_REG = 0b00000000;
				
				break;
			
			case 50:
				BGP_REG = 0b01000000;
				OBP0_REG = 0b01000000;
				OBP1_REG = 0b00000001;
				
				break;
			
			case 40:
				BGP_REG = 0b10010000;
				OBP0_REG = 0b10010000;
				OBP1_REG = 0b00000110;
				
				break;
			
			case 30:
				BGP_REG = 0b11100100;
				OBP0_REG = 0b11100100;
				OBP1_REG = 0b00011011;
				
				break;
			
			case 0:
				specialAttack = FALSE;
				break;
		}
		
		if (specialAttackTimer <= 90 && specialAttackTimer > 60) {
			switch( specialAttackTimer % 16) {
				case 0:
					playerAnimState = PLAYERANIM_BACK;
					playerFlip = FALSE;
					break;
				case 4:
					playerAnimState = PLAYERANIM_FRONT;
					playerFlip = FALSE;
					break;
				case 8:
					playerAnimState = PLAYERANIM_SIDE;
					playerFlip = TRUE;
					break;
				case 12:
					playerAnimState = PLAYERANIM_SIDE;
					playerFlip = FALSE;
					break;
			}
			/*if (joypads.joy0 & J_UP) {
			playerAnimState = PLAYERANIM_BACK;
			playerFlip = FALSE;
		}
		else if (joypads.joy0 & J_DOWN) {
			playerAnimState = PLAYERANIM_FRONT;
			playerFlip = FALSE;
		}
		if (joypads.joy0 & J_LEFT) {
			playerAnimState = PLAYERANIM_SIDE;
			playerFlip = TRUE;
		}
		else if (joypads.joy0 & J_RIGHT) {
			playerAnimState = PLAYERANIM_SIDE;
			playerFlip = FALSE;
		}*/
			playerZ = specialJump[90-specialAttackTimer];
		}
		
		if (specialAttackTimer == 60) {
			playerZ = 0;
			UpdateSpecial();
		}
		
		if (playerFlip) {
				//hide_metasprite(Sprite_Jasper_White_metasprites[playerAnimFrame+playerAnimState], 6);
				move_metasprite_vflip(Sprite_Jasper_metasprites[playerAnimFrame+playerAnimState], 0, 0, ((playerX-camX) >> 4) + 8, ((playerY-camY) >> 4) + 16 - playerZ);
				//move_metasprite_vflip(Sprite_Jasper_White_metasprites[playerAnimFrame+playerAnimState], 45, 6, ((playerX-camX) >> 4) + 8, ((playerY-camY) >> 4) + 16);
				//for (uint8_t i = 0; i < 6; i++) set_sprite_prop(6+i, 0b00110000);
				
			} else {
				//hide_metasprite(Sprite_Jasper_White_metasprites[playerAnimFrame+playerAnimState], 6);
				move_metasprite(Sprite_Jasper_metasprites[playerAnimFrame+playerAnimState], 0, 0, ((playerX-camX) >> 4) + 8, ((playerY-camY) >> 4) + 16 - playerZ);
				//move_metasprite(Sprite_Jasper_White_metasprites[playerAnimFrame+playerAnimState], 45, 6, ((playerX-camX) >> 4) + 8, ((playerY-camY) >> 4) + 16);
				//for (uint8_t i = 0; i < 6; i++) set_sprite_prop(6+i, 0b00010000);
			}
			set_sprite_prop(6, 0b00010000);
			switch (playerAnimState) {
				case PLAYERANIM_FRONT:
					if (playerAnimFrame == 1 || playerAnimFrame == 3) move_sprite(6, ((playerX-camX) >> 4) + 8 - 4, ((playerY-camY) >> 4) + 16 - 3 - playerZ);
					else move_sprite(6, ((playerX-camX) >> 4) + 8 - 4, ((playerY-camY) >> 4) + 16 - 2 - playerZ);
					break;
				case PLAYERANIM_BACK:
					move_sprite(6, 0, 0);
					break;
				case PLAYERANIM_SIDE:
					if (playerFlip) {
						set_sprite_prop(6, 0b00110000);
						if (playerAnimFrame == 1 || playerAnimFrame == 3) move_sprite(6, ((playerX-camX) >> 4) + 8 - 11, ((playerY-camY) >> 4) + 16 - 3 - playerZ);
						else move_sprite(6, ((playerX-camX) >> 4) + 8 - 11, ((playerY-camY) >> 4) + 16 - 2 - playerZ);
					} else {
						if (playerAnimFrame == 1 || playerAnimFrame == 3) move_sprite(6, ((playerX-camX) >> 4) + 8 + 3, ((playerY-camY) >> 4) + 16 - 3 - playerZ);
						else move_sprite(6, ((playerX-camX) >> 4) + 8 + 3, ((playerY-camY) >> 4) + 16 - 2 - playerZ);
					}
					break;
			}
		
		if (specialAttackTimer == 60) {
			
		}
		
		if (specialAttackTimer == 0) {
			
		}
	}
	
	if (pauseTransition)
	{
		if (pause)
		{
			if (winY > 94)
				winY -= 2;
			else
			{
				winY = 93;
				pauseTransition = FALSE;
			}
		}
		else
		{
			if (winY < 126)
				winY += 2;
			else
			{
				winY = 128;
				pauseTransition = FALSE;
				// pause = FALSE;
			}
		}

		move_win(7, winY);
	}

	if (pause && joypads.joy0 & J_SELECT && !fadeOn) {
		setGameState(GAMESTATE_TITLE);
		//init_title();
		//gameState = GAMESTATE_TITLE;
	}
	
	if (lives == 0 && !playerDead) {
		playerDead = TRUE;
		soundEffect(7);
		//init_gameOver();
	}
}

