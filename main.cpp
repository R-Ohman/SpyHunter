#define _USE_MATH_DEFINES
#include "SpyHunter.h"

/*
ПЛАН НА 4.01
- структуризировать программу, разбить на функции, убрать все ненужное
- поменять период спавна автомобилей, сделать проверку налазит ли авто на другое авто
- сделать возможность сбивать автомобиль с дороги
*/


int main(int argc, char** argv) {
	int t1, t2, quit, frames, rc;
	double delta, fpsTimer, fps;
	
	struct {
		SDL_Event event;
		SDL_Surface* screen, * charset;
		SDL_Texture* scrtex;
		SDL_Window* window;
		SDL_Renderer* renderer;
	} sdl;
	
	SDL_Surface* player;
	struct Game game;
	struct CarInfo cars[5] = { NULL };

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	// fullscreen mode
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&sdl.window, &sdl.renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(sdl.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(sdl.renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(sdl.window, "Spy Hunter | Ruslan Rabadanov 196634");

	sdl.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	sdl.scrtex = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);


	// wyі№czenie widocznoњci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	sdl.charset = SDL_LoadBMP("./cs8x8.bmp");
	if (sdl.charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(sdl.screen);
		SDL_DestroyTexture(sdl.scrtex);
		// SDL_DestroyTexture(playerTex);
		SDL_DestroyWindow(sdl.window);
		SDL_DestroyRenderer(sdl.renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(sdl.charset, true, 0x000000);

	player = SDL_LoadBMP("./assets/car.bmp");

	if (player == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(sdl.charset);
		SDL_FreeSurface(sdl.screen);
		SDL_DestroyTexture(sdl.scrtex);
		// SDL_DestroyTexture(playerTex);
		SDL_DestroyWindow(sdl.window);
		SDL_DestroyRenderer(sdl.renderer);
		SDL_Quit();
		return 1;
	};

	char text[128];
	int czarny = SDL_MapRGB(sdl.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(sdl.screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(sdl.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(sdl.screen->format, 0x11, 0x11, 0xCC);

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


		SDL_FillRect(sdl.screen, NULL, czarny);
		DrawDest(sdl.screen);

		if (game.car.speed != 0) {
			roadPos += (game.car.speed > 0 ? 4 * game.car.speed : (-7 * game.car.speed)) * delta * 100;
		}
		else {
			roadPos += delta * 500;
		}

		int tmpPos = roadPos;
		// Draw road
		while (tmpPos > -SCREEN_HEIGHT / 7) {
			DrawRoadRectangle(sdl.screen, tmpPos);
			tmpPos -= SCREEN_HEIGHT / 3;
		}

		// Create new car if needed (every X distance)
		if (int(game.distance * 10000) % 181 == 0) {
			bool flag = true;
			printf("\ngame.distance*100 = %f\n", game.distance * 100);
			for (auto& car : cars) {
				if (car.coord.x == 0 && flag) {
					char* car_path = randomCar();
					car.car = SDL_LoadBMP(car_path);
					if (car_path == "./assets/car_blue.bmp" || car_path == "./assets/car_lilac.bmp") {
						car.isEnemy = false;
					}
					else {
						car.isEnemy = true;
					}
					car.coord.y = -car.car->h/2;
					// Над экраном на высоту сгенерированного авто
					car.coord.x = rand() % (SCREEN_WIDTH / 3) + SCREEN_WIDTH / 3;
					do {
						car.coord.x = rand() % (SCREEN_WIDTH / 3) + SCREEN_WIDTH / 3;
					} while (!isFreePlace(&car, cars)); // isFreePlace(&car, cars)
					printf("car_path = %s\ncar.coord.x = %d\tcar.coord.y = %d\ncar.isEnemy = %d\n", car_path, car.coord.x, car.coord.y, car.isEnemy);
					flag = false;
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
					DrawSurface(sdl.screen, car.car, car.coord.x, car.coord.y);
				}
			}
		}
		DrawSurface(sdl.screen, player, game.car.coord.x, game.car.coord.y);

		fpsTimer += delta;
		if (fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};
		// info text
		DrawRectangle(sdl.screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		sprintf(text, "Ruslan Rabadanov 196634");
		DrawString(sdl.screen, sdl.screen->w / 2 - strlen(text) * 8 / 2, 10, text, sdl.charset);
		sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s\tScore: %.0f", game.time, fps, game.score);
		DrawString(sdl.screen, sdl.screen->w / 2 - strlen(text) * 8 / 2, 26, text, sdl.charset);

		sprintf(text, "Esc - wyjscie");
		DrawString(sdl.screen, sdl.screen->w - strlen(text) * 9, 8 * SCREEN_HEIGHT / 10 - 30, text, sdl.charset);

		sprintf(text, "N - nowa gra");
		DrawString(sdl.screen, sdl.screen->w - strlen(text) * 9, 8 * SCREEN_HEIGHT / 10, text, sdl.charset);
		sprintf(text, "\030/\031 - przyspieszenie/zwolnienie");
		DrawString(sdl.screen, sdl.screen->w - strlen(text) * 9, 8 * SCREEN_HEIGHT / 10 - 60, text, sdl.charset);
		
		SDL_UpdateTexture(sdl.scrtex, NULL, sdl.screen->pixels, sdl.screen->pitch);
		SDL_RenderClear(sdl.renderer);
		SDL_RenderCopy(sdl.renderer, sdl.scrtex, NULL, NULL);
		SDL_RenderPresent(sdl.renderer);

		// handling of events (if there were any)
		while (SDL_PollEvent(&sdl.event)) {
			switch (sdl.event.type) {
			case SDL_KEYDOWN:
				if (sdl.event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
				else if (sdl.event.key.keysym.sym == SDLK_n) NewGame(&game);
				else if (sdl.event.key.keysym.sym == SDLK_UP) game.car.speed = -1;
				else if (sdl.event.key.keysym.sym == SDLK_DOWN) game.car.speed = 1;
				else if (sdl.event.key.keysym.sym == SDLK_LEFT) game.car.turn = -1;
				else if (sdl.event.key.keysym.sym == SDLK_RIGHT) game.car.turn = 1;
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
	SDL_FreeSurface(sdl.charset);
	SDL_FreeSurface(sdl.screen);
	SDL_DestroyTexture(sdl.scrtex);
	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.window);
	SDL_Quit();
	return 0;
};
