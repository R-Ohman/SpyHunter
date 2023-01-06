#include "SpyHunter.h"


int initGame(SDL sdl) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdl.window, &sdl.renderer) != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(sdl.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(sdl.renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(sdl.window, "Spy Hunter | Ruslan Rabadanov 196634");
	sdl.scrtex = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_ShowCursor(SDL_DISABLE);

	if (sdl.charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(sdl.screen);
		SDL_DestroyTexture(sdl.scrtex);
		SDL_DestroyWindow(sdl.window);
		SDL_DestroyRenderer(sdl.renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(sdl.charset, true, 0x000000);

	if (sdl.player == NULL) {
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
	return 0;
}


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


void NewGame( Game* game, CarInfo * cars) {
	game->time.startGame = SDL_GetTicks();
	game->time.killMessage = 0;
	game->time.scoreFreeze = 0;
	game->time.total = 0;
	game->score = 0;
	game->totalDistance = 0;
	game->car.coord.x = SCREEN_WIDTH / 2;
	game->car.coord.y = SCREEN_HEIGHT * 2 / 3;
	game->car.speed = 0;
	game->car.turn = 0;
	for (int i = 0; i < 5; i++) {
		cars[i].coord.x = 0;
	}
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


int modul(int num) {
	return num > 0 ? num : -num;
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


// x,y - ���������� �� ��������� ������; x2, y2 - ������� ����� ����� �������
bool numbersInArray(int x, int y, CarInfo* object) {
	int x2 = object->coord.x - object->car->w / 2;
	int y2 = object->coord.y - object->car->h / 2;
	// warn +1 -1
	if (x >= x2 && y >= y2 && x <= x2 + object->car->w && y <= y2 + object->car->h) {
		return true;
	}
	return false;
}


// �������� ��� �������� �� ��������� ������, ��������� �� ������� � ������ �������.
bool touchObject(struct Game* game, CarInfo* object, const double deltaTime, struct CarInfo *cars) {
	int x1 = game->car.coord.x;
	int y1 = game->car.coord.y;
	int x2 = object->coord.x;
	int y2 = object->coord.y;
	bool flag = true;

	for (int i = -object->car->h / 2 + 1; i < object->car->h / 2; i++) {
		if (numbersInArray(x1 - object->car->w / 2, y1 + i, object) ||
			numbersInArray(x1 + object->car->w / 2, y1 + i, object)) {
			if (isFreePlace(object, cars, game->car.turn)) {
				object->coord.x += game->car.turn * deltaTime * 300;
				printf("Delta: %f\n", game->car.turn * deltaTime * 300);
				if (object->coord.x < SCREEN_WIDTH / 3 - object->car->w ||
					object->coord.x > 2 * SCREEN_WIDTH / 3 + object->car->w)
				{
					object->car = SDL_LoadBMP("./assets/car_destroyed.bmp");
					if (object->isEnemy) {
						if (!game->time.scoreFreeze) {
							game->score += 1000;
							game->time.killMessage = 2;
						}
					}
					else {
						printf("-1000 - %d\n", object->coord.x);
						game->time.scoreFreeze += 3;
					}
				}
			}
			else {
				//game->car.coord.x -= game->car.turn * deltaTime * 200;
				// �������� ������ ��-�� �������� � ���� ����, ������ ����� ���������� �������
				game->car.coord.x -= game->car.turn * 6; // WARN
				// ���� ���� ��������� ����, �������� - 200, ����� ����������� �� ������� ����� ��� �������� 300
				flag = false;
				break;
			}
		}
	}
	if (flag) {
		for (int i = -object->car->w / 2 + 3; i < object->car->w / 2 - 2; i++) {
			if (numbersInArray(x1 + i, y1 - object->car->h / 2, object) ||
				numbersInArray(x1 + i, y1 + object->car->h / 2, object)) return true;
		}
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


void DrawDest(SDL_Surface* screen, int roadMarkingPos) {
	int grey = SDL_MapRGB(screen->format, 105, 105, 105);
	DrawRectangle(screen, SCREEN_WIDTH / 3, 0, SCREEN_WIDTH / 3, SCREEN_HEIGHT, grey, grey);
	int grey_dark = SDL_MapRGB(screen->format, 40, 40, 40);
	DrawRectangle(screen, SCREEN_WIDTH / 3 - SCREEN_WIDTH / 30, 0, SCREEN_WIDTH / 30, SCREEN_HEIGHT, grey_dark, grey_dark);
	DrawRectangle(screen, SCREEN_WIDTH * 2 / 3, 0, SCREEN_WIDTH / 30, SCREEN_HEIGHT, grey_dark, grey_dark);

	if (game.car.speed != 0) {
		roadMarkingPos += (game.car.speed > 0 ? 4 * game.car.speed : (-7 * game.car.speed)) * game.time.delta * 100;
	}
	else {
		roadMarkingPos += game.time.delta * 500;
	}

	int tmpPos = roadMarkingPos;
	// Draw road
	while (tmpPos > -SCREEN_HEIGHT / 7) {
		DrawRoadRectangle(sdl.screen, tmpPos);
		tmpPos -= SCREEN_HEIGHT / 3;
	}
};


void DrawHeader(SDL_Surface* screen, Game game, SDL sdl, double fps) {
	char text[128];
	int czerwony = SDL_MapRGB(sdl.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(sdl.screen->format, 0x11, 0x11, 0xCC);
	DrawRectangle(sdl.screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	sprintf(text, "Ruslan Rabadanov 196634");
	DrawString(sdl.screen, sdl.screen->w / 2 - strlen(text) * 8 / 2, 10, text, sdl.charset);
	sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s\tScore: %.0f", game.time, fps, game.score);
	DrawString(sdl.screen, sdl.screen->w / 2 - strlen(text) * 8 / 2, 26, text, sdl.charset);

	if (game.time.scoreFreeze) {
		sprintf(text, "Score is freezed on %.1f sec", game.time.scoreFreeze);
		DrawString(sdl.screen, sdl.screen->w - strlen(text) * 8, SCREEN_HEIGHT / 2, text, sdl.charset);
	}
	else if (game.time.killMessage > 0) {
		sprintf(text, "KILL! You get 1000 points!");
		DrawString(sdl.screen, sdl.screen->w - strlen(text) * 8, SCREEN_HEIGHT / 2, text, sdl.charset);
	}
	sprintf(text, "Esc - wyjscie");
	DrawString(sdl.screen, sdl.screen->w - strlen(text) * 9, 8 * SCREEN_HEIGHT / 10 - 30, text, sdl.charset);

	sprintf(text, "N - nowa gra");
	DrawString(sdl.screen, sdl.screen->w - strlen(text) * 9, 8 * SCREEN_HEIGHT / 10, text, sdl.charset);
	sprintf(text, "\030/\031 - przyspieszenie/zwolnienie");
	DrawString(sdl.screen, sdl.screen->w - strlen(text) * 9, 8 * SCREEN_HEIGHT / 10 - 60, text, sdl.charset);
};


void DrawRoadRectangle(SDL_Surface* screen, int y) {
	int grey_light = SDL_MapRGB(screen->format, 192, 192, 192);
	DrawRectangle(screen, SCREEN_WIDTH / 2, y, SCREEN_WIDTH / 50, SCREEN_HEIGHT / 7, grey_light, grey_light);
};


bool isDestroyed(struct CarInfo* car) {
	if (car->car == SDL_LoadBMP("./assets/car_destroyed.bmp")
		|| car->coord.x < SCREEN_WIDTH / 3 - car->car->w || car->coord.x > 2 * SCREEN_WIDTH / 3 + car->car->w) {
		return true;
	}
	return false;
}


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


bool isFreePlace(struct CarInfo* car, struct CarInfo* cars, int turn) {
	// turn = -1 : ��������� ������, 1 : ��������� �����
	int x, y, y2;
	y = car->coord.y + car->car->h / 2;
	y2 = y - car->car->h;
	if (turn == 1) {
		x = car->coord.x + car->car->w / 2;
	}
	else {
		x = car->coord.x - car->car->w / 2;
	}
	for (int i = 0; i < 5; i++) {
		// ���� ����� �� ���� �� ������
		if (cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x || cars[i].coord.x == 0) continue;
		// �������� ������ ����� � ������ ������ ����� �������������� ������
		if (numbersInArray(x, y, &cars[i]) || numbersInArray(x, y2, &cars[i])) {
			// WARN
			car->coord.x -= turn;
			return false;
		}
	}
	return true;
}


bool inFault(int num1, int num2, int fault) {
	int num = (num1 - num2 > 0) ? num1 - num2 : num2 - num1;
	return (num < fault) ? true : false;
}


// �������� ��� ��������� ������ ����� �� ��� �����
bool canRide(struct CarInfo* car, struct CarInfo* cars) {
	// turn = -1 : ��������� ������, 1 : ��������� �����
	int y = car->coord.y;
	for (int i = 0; i < 5; i++) {
		// ���� ����� �� ���� �� ������
		if (cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x || cars[i].coord.x == 0) continue;
		// �������� ��������� �� � ������� ������ ����
		// WARN -2 -> -1 | ����� ������ �������, ����� ����� ���� � �����
		// WARN +2 | ����� ���� ������� 2 ������ � ���, ����� ��� �� �������
		if (inFault(cars[i].coord.y, y, car->car->h + 2) && inFault(cars[i].coord.x, car->coord.x, car->car->w - 2))
		{
			return false;
		}
	}
	return true;
}


// can attack if player is above or under the enemy car
// 2 - car is above, 0 - can't attack, -1 - car is under the enemy
int canAttack(struct CarInfo* car, struct Game* game, struct CarInfo* cars) {
	if (inFault(game->car.coord.x, car->coord.x, 31)) {
		if (inFault(game->car.coord.y, car->coord.y, SCREEN_HEIGHT) && game->car.coord.y - car->coord.y > car->car->h + 30) {
			// ��� ����� ������ �������� �� 30 �������� �� ����
			if (!canRide(car, cars)) {
				// += 1
				car->coord.y -= 3;
				return 0;
			}
			return 2;
		}
		else if (inFault(game->car.coord.y, car->coord.y, SCREEN_HEIGHT) && game->car.coord.y - car->coord.y < -car->car->h - 10) {
			// ��� ����� �������� �� 10 �������� �� ����
			if (!canRide(car, cars)) {
				// -= 2
				car->coord.y += 3;
				return 0;
			}
			return -1;
		}
		//return  (car->coord.y < game->car.coord.y) ? 2 : -1;
	}
	
	return 100;
}


void drawRandomCar(CarInfo* cars, Game game, SDL sdl) {
	for (int i = 0; i < 5; i++) {
		if (cars[i].coord.x != 0) {
			// ���� ������������, �� ������� � ������ �������
			cars[i].speed = isDestroyed(&cars[i]) ? 500 : 100;
			if (cars[i].isEnemy && canAttack(&cars[i], &game, cars) != 100) {
				// canAttack ���������� � ����� ������� ������ ����������
				cars[i].coord.y += game.time.delta * cars[i].speed * 1.3 * canAttack(&cars[i], &game, cars);
			}
			else {
				cars[i].coord.y += game.time.delta * cars[i].speed;
			}
		}
	}

	// Create new car if needed (every X distance)
	if (int(game.totalDistance * 10000) % 181 == 0) {
		bool flag = true;
		for (int i = 0; i < 5; i++) {
			if (cars[i].coord.x == 0 && flag) {
				char* car_path = randomCar();
				cars[i].car = SDL_LoadBMP(car_path);
				if (car_path == "./assets/car_blue.bmp" || car_path == "./assets/car_lilac.bmp") {
					cars[i].isEnemy = false;
				}
				else {
					cars[i].isEnemy = true;
				}
				cars[i].coord.y = -cars[i].car->h / 2;
				// ��� ������� �� ������ ���������������� ����
				cars[i].coord.x = rand() % (SCREEN_WIDTH / 3) + SCREEN_WIDTH / 3;
				do {
					cars[i].coord.x = rand() % (SCREEN_WIDTH / 3) + SCREEN_WIDTH / 3;
				} while (!isFreePlace(&cars[i], cars, -1) || !isFreePlace(&cars[i], cars, 1)); // isFreePlace(&car, cars)
				flag = false;
				break;
			}
			else if (cars[i].coord.y > SCREEN_HEIGHT + cars[i].car->h / 2) {
				cars[i].coord.x = 0;
			}
		}
	}

	for (int i = 0; i < 5; i++) {
		if (cars[i].coord.x != 0) {
			if (touchObject(&game, &cars[i], game.time.delta, cars)) {
				NewGame(&game, cars);
			}
			else {
				DrawSurface(sdl.screen, cars[i].car, cars[i].coord.x, cars[i].coord.y);
			}
		}
	}
}


void freeSurfaces(SDL sdl) {
	SDL_FreeSurface(sdl.charset);
	SDL_FreeSurface(sdl.screen);
	SDL_DestroyTexture(sdl.scrtex);
	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.window);
	SDL_Quit();
}


void renderSurfaces(SDL sdl) {
	SDL_UpdateTexture(sdl.scrtex, NULL, sdl.screen->pixels, sdl.screen->pitch);
	SDL_RenderClear(sdl.renderer);
	SDL_RenderCopy(sdl.renderer, sdl.scrtex, NULL, NULL);
	SDL_RenderPresent(sdl.renderer);
}