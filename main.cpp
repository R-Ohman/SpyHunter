#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include <time.h>
#include<string.h>
#include <stdlib.h>

/*
ПЛАН НА 4.01
- структуризировать программу, разбить на функции, убрать все ненужное
- поменять период спавна автомобилей, сделать проверку налазит ли авто на другое авто
- сделать возможность сбивать автомобиль с дороги
*/

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	960
#define SCREEN_HEIGHT	720


// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};

struct Game {
	struct Car {
		struct Coord {
			int x = SCREEN_WIDTH / 2;
			int y = SCREEN_HEIGHT * 2 / 3;
		} coord;
		int speed = 0;
		int turn = 0;
	} car;
	double time = 0;
	double distance = 0;
	double score = distance * distance * time;
};


struct CarInfo {
	SDL_Surface* car;
	struct Coord {
		int x;
		int y;
	} coord;
	bool isEnemy;
};


void NewGame(struct Game* game) {
	game->time = 0;
	game->score = 0;
	game->distance = 0;
	game->car.coord.x = SCREEN_WIDTH / 2;
	game->car.coord.y = SCREEN_HEIGHT * 2 / 3;
	game->car.speed = 0;
	game->car.turn = 0;
};


// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};



// check if X and Y coordinates are inside a screen
bool inArray(int x, int y) {
	if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
		return true;
	}
	return false;
}


// Check if vertical position is out of screen and do it in the range.
void inArrayDeltaY(int* vertical) {
	if (*vertical < 90) {
		*vertical = 90;
	}
	else if (*vertical > SCREEN_HEIGHT) {
		*vertical = SCREEN_HEIGHT;
	}
}


// Check ifhorizontal position is out of screen and do it in the range.
void inArrayDeltaX(int* horizontal) {
	if (*horizontal < SCREEN_WIDTH / 2 - SCREEN_WIDTH / 6) {
		*horizontal = SCREEN_WIDTH / 2 - SCREEN_WIDTH / 6;
	}
	else if (*horizontal > SCREEN_WIDTH / 2 + SCREEN_WIDTH / 6) {
		*horizontal = SCREEN_WIDTH / 2 + SCREEN_WIDTH / 6;
	}
}


// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	if (inArray(x, y)) {
		int bpp = surface->format->BytesPerPixel;
		Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
		*(Uint32*)p = color;
	}
};


// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// x,y - координата на периметре машины; x2, y2 - центр объекта
bool numbersInArray(int x, int y, CarInfo* object) {
	int x2 = object->coord.x;
	int y2 = object->coord.y;
	x2 = x2 - object->car->w / 2;
	y2 = y2 - object->car->h / 2;
	if (x >= x2 && y >= y2 && x <= x2 + object->car->w && y <= y2 + object->car->h) {
		return true;
	}
	return false;
}


// Проверка для пикселей по периметру машины, находится ли позиция в другом объекте.
bool touchObject(struct Game* game, CarInfo* object) {
	int x1 = game->car.coord.x;
	int y1 = game->car.coord.y;
	int x2 = object->coord.x;
	int y2 = object->coord.y;

	for (int i = -object->car->h / 2; i < object->car->h / 2; i++) {
		if (numbersInArray(x1 - object->car->w / 2, y1 + i, object) ||
			numbersInArray(x1 + object->car->w / 2, y1 + i, object)) return true;
	}
	for (int i = -object->car->w / 2; i < object->car->w / 2; i++) {
		if (numbersInArray(x1 + i, y1 - object->car->h / 2, object) ||
			numbersInArray(x1 + i, y1 + object->car->h / 2, object)) return true;
	}
	return false;
}


// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};


void DrawDest(SDL_Surface* screen) {
	int green = SDL_MapRGB(screen->format, 107, 142, 35);
	DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, green, green);
	int grey = SDL_MapRGB(screen->format, 105, 105, 105);
	DrawRectangle(screen, SCREEN_WIDTH / 3, 0, SCREEN_WIDTH / 3, SCREEN_HEIGHT, grey, grey);
	int grey_dark = SDL_MapRGB(screen->format, 40, 40, 40);
	DrawRectangle(screen, SCREEN_WIDTH / 3 - SCREEN_WIDTH / 30, 0, SCREEN_WIDTH / 30, SCREEN_HEIGHT, grey_dark, grey_dark);
	DrawRectangle(screen, SCREEN_WIDTH * 2 / 3, 0, SCREEN_WIDTH / 30, SCREEN_HEIGHT, grey_dark, grey_dark);
};


void DrawRoadRectangle(SDL_Surface* screen, int y) {
	int grey_light = SDL_MapRGB(screen->format, 192, 192, 192);
	DrawRectangle(screen, SCREEN_WIDTH / 2, y, SCREEN_WIDTH / 50, SCREEN_HEIGHT / 7, grey_light, grey_light);
};


// Retern string - path to the BMP image
char* randomCar() {
	int number = rand() % 5 + 1;
	switch (number) {
	case 1:
		return "./assets/car_blue.bmp";
	case 2:
		return "./assets/car_green.bmp";
	case 3:
		return "./assets/car_lightblue.bmp";
	case 4:
		return "./assets/car_lilac.bmp";
	case 5:
		return "./assets/car_red.bmp";
	}
}


#ifdef __cplusplus
extern "C"
#endif

int main(int argc, char** argv) {
	int t1, t2, quit, frames, rc;
	double delta, fpsTimer, fps;
	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Surface* player;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	struct Game game;
	struct CarInfo cars[5] = { NULL };

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	// fullscreen mode
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Spy Hunter | Ruslan Rabadanov 196634");

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);


	// wyі№czenie widocznoњci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		// SDL_DestroyTexture(playerTex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);

	player = SDL_LoadBMP("./assets/car.bmp");

	if (player == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		// SDL_DestroyTexture(playerTex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	int roadPos = SCREEN_HEIGHT - 100;

	while (!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		int speed_coef = 1;
		game.time += delta;
		if (game.car.speed != 0) {
			speed_coef = game.car.speed > 0 ? game.car.speed : (-3 * game.car.speed);
			game.distance += speed_coef * delta;
			game.score += speed_coef * game.distance * delta / 2;
		}
		else {
			game.distance += delta;
			game.score += game.distance * delta / 2;
		}

		if (game.car.speed * delta > 0) {
			game.car.coord.y += game.car.speed * delta * 200;
		}
		else if (game.car.speed * delta < 0) {
			game.car.coord.y += game.car.speed * delta * 200;
		}

		game.car.coord.x += game.car.turn * delta * 200;
		inArrayDeltaX(&game.car.coord.x);
		inArrayDeltaY(&game.car.coord.y);

		for (auto& car : cars) {
			if (car.coord.x != 0) {
				car.coord.y += speed_coef * delta * 100;
			}
		}


		SDL_FillRect(screen, NULL, czarny);
		DrawDest(screen);

		if (game.car.speed != 0) {
			roadPos += (game.car.speed > 0 ? 4 * game.car.speed : (-7 * game.car.speed)) * delta * 100;
		}
		else {
			roadPos += delta * 500;
		}

		int tmpPos = roadPos;
		// Draw road
		while (tmpPos > -SCREEN_HEIGHT / 7) {
			DrawRoadRectangle(screen, tmpPos);
			tmpPos -= SCREEN_HEIGHT / 3;
		}

		// Create new car if needed (every 3 distance)
		if (int(game.distance * 10000) % 181 == 0) {
			bool flag = true;
			printf("\ngame.distance*100 = %f\n", game.distance * 100);
			for (auto& car : cars) {
				if (car.coord.x == 0 && flag) {
					char* car_path = randomCar();
					car.car = SDL_LoadBMP(car_path);
					// Над экраном на высоту сгенерированного авто
					car.coord.y = -car.car->h;
					car.coord.x = rand() % (SCREEN_WIDTH / 3) + SCREEN_WIDTH / 3;
					if (car_path == "./assets/car_blue.bmp" || car_path == "./assets/car_lilac.bmp") {
						car.isEnemy = false;
					}
					else {
						car.isEnemy = true;
					}
					flag = false;
					printf("car_path = %s\ncar.coord.x = %d\tcar.coord.y = %d\ncar.isEnemy = %d\n", car_path, car.coord.x, car.coord.y, car.isEnemy);
					break;
				}
				else if (car.coord.y > SCREEN_HEIGHT + car.car->h / 2) {
					car.coord.x = 0;
				}
			}
		}


		for (auto& car : cars) {
			if (car.coord.x != 0) {
				if (touchObject(&game, &car)) {
					NewGame(&game);
				}
				else {
					DrawSurface(screen, car.car, car.coord.x, car.coord.y);
				}
			}
		}
		DrawSurface(screen, player, game.car.coord.x, game.car.coord.y);

		fpsTimer += delta;
		if (fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};
		// info text
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		sprintf(text, "Ruslan Rabadanov 196634");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s\tScore: %.0f", game.time, fps, game.score);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

		sprintf(text, "Esc - wyjscie");
		DrawString(screen, screen->w - strlen(text) * 9, 8 * SCREEN_HEIGHT / 10 - 30, text, charset);

		sprintf(text, "N - nowa gra");
		DrawString(screen, screen->w - strlen(text) * 9, 8 * SCREEN_HEIGHT / 10, text, charset);
		sprintf(text, "\030/\031 - przyspieszenie/zwolnienie");
		DrawString(screen, screen->w - strlen(text) * 9, 8 * SCREEN_HEIGHT / 10 - 60, text, charset);
		
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// handling of events (if there were any)
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
				else if (event.key.keysym.sym == SDLK_n) NewGame(&game);
				else if (event.key.keysym.sym == SDLK_UP) game.car.speed = -1;
				else if (event.key.keysym.sym == SDLK_DOWN) game.car.speed = 1;
				else if (event.key.keysym.sym == SDLK_LEFT) game.car.turn = -1;
				else if (event.key.keysym.sym == SDLK_RIGHT) game.car.turn = 1;
				break;
			case SDL_KEYUP:
				game.car.speed = 0;
				game.car.turn = 0;
				break;
			case SDL_QUIT:
				quit = 1;
				break;
			};
		};
		frames++;
	};

	// freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
};
