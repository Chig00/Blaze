#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL.h>

const char* const TITLE = "Blaze by Chigozie Agomo";
const char* const SONG = "tlozalttpow.wav";
const int SONG_LEN = 138;
const int WINDOW_W = 640;
const int WINDOW_H = 640;
const double PI = acos(-1);
const int QUIT = SDL_SCANCODE_ESCAPE;
const int RESET = SDL_SCANCODE_R;
const int PAUSE = SDL_SCANCODE_P;
const int UP = SDL_SCANCODE_UP;
const int LEFT = SDL_SCANCODE_LEFT;
const int DOWN = SDL_SCANCODE_DOWN;
const int RIGHT = SDL_SCANCODE_RIGHT;
const int BGR = 120;
const int BGG = 20;
const int BGB = 0;
const int PW = 64;
const int PH = 64;
const double P_SPD = 0.75;
const int P_PHASES = 2;
const int P_PHASE_L = 50;
const char* const PS[] = {
	"u0.bmp",
	"u1.bmp",
	"l0.bmp",
	"l1.bmp",
	"d0.bmp",
	"d1.bmp",
	"r0.bmp",
	"r1.bmp"
};
const int FBW = 16;
const int FBH = 16;
const double FB_SPD = 0.25;
const int FB_PHASES = 4;
const int FB_PHASE_L = 100;
const char* const FBS[] = {
	"fb0.bmp",
	"fb1.bmp",
	"fb2.bmp",
	"fb3.bmp"
};
const int DELAY_DECAY = 50;
const double DELAY_BASE = 0.5;

typedef struct {
	SDL_Surface* surfs[4][2];
	SDL_Rect rect;
	double pos[2];
	int dir;
	int phase;
} Player;

typedef struct {
	SDL_Surface* surfs[4];
	SDL_Rect rect;
	double pos[2];
	double vel[2];
	int phase;
} Fireball;

typedef struct {
	Fireball* fballs;
	int count;
} Fireballs;

void reset(Player* player) {
	player->pos[0] = WINDOW_W / 2;
	player->pos[1] = WINDOW_H / 2;
	player->rect.x = player->pos[0] - PW / 2;
	player->rect.y = player->pos[1] - PH / 2;
	player->dir = 2;
	player->phase = 0;
}

double get_time() {
	return (double) clock() / CLOCKS_PER_SEC;
}

double random(double min, double max) {
	return min + (max - min) * rand() / RAND_MAX;
}

double fball_delay(int score) {
	return DELAY_DECAY / (score + DELAY_DECAY / DELAY_BASE);
}

void new_fball(Fireballs* fballs, SDL_Surface* fb_surfs[]) {
	fballs->fballs = realloc(fballs->fballs, sizeof(Fireball) * (fballs->count + 1));
	fballs->fballs[fballs->count].surfs[0] = fb_surfs[0];
	fballs->fballs[fballs->count].surfs[1] = fb_surfs[1];
	fballs->fballs[fballs->count].surfs[2] = fb_surfs[2];
	fballs->fballs[fballs->count].surfs[3] = fb_surfs[3];
	fballs->fballs[fballs->count].rect.w = FBW;
	fballs->fballs[fballs->count].rect.h = FBH;
	fballs->fballs[fballs->count].phase = 0;
	
	if (rand() % 2) {
		fballs->fballs[fballs->count].pos[0] = random(-FBW / 2, WINDOW_W + FBW / 2);
		
		if (rand() % 2) {
			fballs->fballs[fballs->count].pos[1] = -FBH / 2;
		}
		
		else {
			fballs->fballs[fballs->count].pos[1] = WINDOW_H + FBH / 2;
		}
	}
	
	else {
		fballs->fballs[fballs->count].pos[1] = random(-FBH / 2, WINDOW_H + FBH / 2);
		
		if (rand() % 2) {
			fballs->fballs[fballs->count].pos[0] = -FBW / 2;
		}
		
		else {
			fballs->fballs[fballs->count].pos[0] = WINDOW_W + FBW / 2;
		}
	}
	
	double angle =
		atan2(WINDOW_H / 2 - fballs->fballs[fballs->count].pos[1], WINDOW_W / 2 - fballs->fballs[fballs->count].pos[0])
		+ random(-PI / 4, PI / 4);
	;
	fballs->fballs[fballs->count].vel[0] = FB_SPD * cos(angle);
	fballs->fballs[fballs->count].vel[1] = FB_SPD * sin(angle);
	fballs->fballs[fballs->count].rect.x = fballs->fballs[fballs->count].pos[0] - FBW / 2;
	fballs->fballs[fballs->count].rect.y = fballs->fballs[fballs->count].pos[1] - FBH / 2;
	fballs->count++;
}

void update_player(Player* player, SDL_Surface* display) {
	if (SDL_GetKeyboardState(NULL)[UP] && player->dir == 0) {
		player->pos[1] -= P_SPD;
		player->phase = (player->phase + 1) % (P_PHASES * P_PHASE_L);
	}
	
	else if (SDL_GetKeyboardState(NULL)[LEFT] && player->dir == 1) {
		player->pos[0] -= P_SPD;
		player->phase = (player->phase + 1) % (P_PHASES * P_PHASE_L);
	}
	
	else if (SDL_GetKeyboardState(NULL)[DOWN] && player->dir == 2) {
		player->pos[1] += P_SPD;
		player->phase = (player->phase + 1) % (P_PHASES * P_PHASE_L);
	}
	
	else if (SDL_GetKeyboardState(NULL)[RIGHT] && player->dir == 3) {
		player->pos[0] += P_SPD;
		player->phase = (player->phase + 1) % (P_PHASES * P_PHASE_L);
	}
	
	else if (SDL_GetKeyboardState(NULL)[UP]) {
		player->dir = 0;
		player->phase = 0;
	}
	
	else if (SDL_GetKeyboardState(NULL)[LEFT]) {
		player->dir = 1;
		player->phase = 0;
	}
	
	else if (SDL_GetKeyboardState(NULL)[DOWN]) {
		player->dir = 2;
		player->phase = 0;
	}
	
	else if (SDL_GetKeyboardState(NULL)[RIGHT]) {
		player->dir = 3;
		player->phase = 0;
	}
	
	if (player->pos[0] < PW / 2) {
		player->pos[0] = PW / 2;
	}
	
	else if (player->pos[0] > WINDOW_W - PW / 2) {
		player->pos[0] = WINDOW_W - PW / 2;
	}
	
	else if (player->pos[1] < PH / 2) {
		player->pos[1] = PH / 2;
	}
	
	else if (player->pos[1] > WINDOW_H - PH / 2) {
		player->pos[1] = WINDOW_H - PH / 2;
	}
	
	player->rect.x = player->pos[0] - PW / 2;
	player->rect.y = player->pos[1] - PH / 2;
	SDL_BlitSurface(player->surfs[player->dir][player->phase / P_PHASE_L], NULL, display, &player->rect);
}

bool update_fballs(Fireballs* fballs, Player player, int* score, SDL_Surface* display, SDL_Window* window) {
	for (int i = 0; i < fballs->count; i++) {
		fballs->fballs[i].pos[0] += fballs->fballs[i].vel[0];
		fballs->fballs[i].pos[1] += fballs->fballs[i].vel[1];
		fballs->fballs[i].rect.x = fballs->fballs[i].pos[0] - FBW / 2;
		fballs->fballs[i].rect.y = fballs->fballs[i].pos[1] - FBH / 2;
		fballs->fballs[i].phase = (fballs->fballs[i].phase + 1) % (FB_PHASES * FB_PHASE_L);
		SDL_BlitSurface(fballs->fballs[i].surfs[fballs->fballs[i].phase / FB_PHASE_L], NULL, display, &fballs->fballs[i].rect);
		
		if (SDL_HasIntersection(&fballs->fballs[i].rect, &player.rect)) {
			SDL_UpdateWindowSurface(window);
			return true;
		}
		
		if (
			fballs->fballs[i].pos[0] < -FBW / 2 || fballs->fballs[i].pos[0] > WINDOW_W + FBW / 2
			|| fballs->fballs[i].pos[1] < -FBH / 2 || fballs->fballs[i].pos[1] > WINDOW_H + FBH / 2
		) {
			for (int j = i; j < fballs->count - 1; j++) {
				fballs->fballs[j] = fballs->fballs[j + 1];
			}
			
			fballs->fballs = realloc(fballs->fballs, sizeof(Fireball) * --fballs->count);
			++*score;
		}
	}
	
	return false;
}

int main(int argc, char* argv[]) {
	srand(time(NULL));
	SDL_Window* window = SDL_CreateWindow(
		TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN
	);
	SDL_Surface* display = SDL_GetWindowSurface(window);
	SDL_Surface* fb_surfs[4] = {
		SDL_LoadBMP(FBS[0]),
		SDL_LoadBMP(FBS[1]),
		SDL_LoadBMP(FBS[2]),
		SDL_LoadBMP(FBS[3])
	};
	Player player = {
		.surfs = {
			{SDL_LoadBMP(PS[0]), SDL_LoadBMP(PS[1])},
			{SDL_LoadBMP(PS[2]), SDL_LoadBMP(PS[3])},
			{SDL_LoadBMP(PS[4]), SDL_LoadBMP(PS[5])},
			{SDL_LoadBMP(PS[6]), SDL_LoadBMP(PS[7])}
		},
		.rect = {
			.w = PW,
			.h = PH,
		}
	};
	SDL_AudioSpec song;
	Uint8* buf;
	Uint32 len;
	SDL_LoadWAV(SONG, &song, &buf, &len);
	SDL_OpenAudio(&song, NULL);
	SDL_PauseAudio(false);
	SDL_QueueAudio(1, buf, len);
	double last_queue = get_time();
	bool quit = false;
	
	while (!quit) {
		bool end = false;
		int score = 0;
		reset(&player);
		Fireballs fballs = {
			.fballs = NULL,
			.count = 0
		};
		double last_fball = get_time();
		
		while (!(quit || end)) {
			SDL_FillRect(display, NULL, SDL_MapRGB(display->format, BGR, BGG, BGB));
			
			if (get_time() >= last_queue + SONG_LEN) {
				SDL_QueueAudio(1, buf, len);
				last_queue = get_time();
			}
			
			if (get_time() >= last_fball + fball_delay(score)) {
				new_fball(&fballs, fb_surfs);
				last_fball = get_time();
			}
			
			update_player(&player, display);
			
			if (update_fballs(&fballs, player, &score, display, window)) {
				break;
			}
			
			SDL_UpdateWindowSurface(window);
			SDL_PumpEvents();
			quit = SDL_GetKeyboardState(NULL)[QUIT];
			end = SDL_GetKeyboardState(NULL)[RESET];
			
			if (SDL_GetKeyboardState(NULL)[PAUSE]) {
				while (SDL_GetKeyboardState(NULL)[PAUSE]) {
					SDL_PumpEvents();
				}
				
				while (!SDL_GetKeyboardState(NULL)[PAUSE]) {
					SDL_PumpEvents();
				}
				
				while (SDL_GetKeyboardState(NULL)[PAUSE]) {
					SDL_PumpEvents();
				}
			}
		}
		
		printf("Score: %d\n", score);
		
		while (!(SDL_GetKeyboardState(NULL)[RESET] || quit)) {
			quit = SDL_GetKeyboardState(NULL)[QUIT];
			SDL_PumpEvents();
		}
		
		while (SDL_GetKeyboardState(NULL)[RESET]) {
			SDL_PumpEvents();
		}
		
		free(fballs.fballs);
	}
}