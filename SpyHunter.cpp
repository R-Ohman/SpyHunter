#include "SpyHunter.h"

int initGame(SDL* sdl) {
	sdl->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	sdl->charset = SDL_LoadBMP("./cs8x8.bmp");
	sdl->playerCars[0] = SDL_LoadBMP("./assets/player_1.bmp");
	sdl->playerCars[1] = SDL_LoadBMP("./assets/player_2.bmp");
	sdl->playerCars[2] = SDL_LoadBMP("./assets/player_3.bmp");
	sdl->playerCars[3] = SDL_LoadBMP("./assets/player_4.bmp");
	sdl->playerCars[4] = SDL_LoadBMP("./assets/player_5.bmp");
	sdl->cars[0] = SDL_LoadBMP("./assets/car_blue.bmp");
	sdl->cars[1] = SDL_LoadBMP("./assets/car_green.bmp");
	sdl->cars[2] = SDL_LoadBMP("./assets/car_lightblue.bmp");
	sdl->cars[3] = SDL_LoadBMP("./assets/car_lilac.bmp");
	sdl->cars[4] = SDL_LoadBMP("./assets/car_red.bmp");
	sdl->cars[5] = SDL_LoadBMP("./assets/car_destroyed.bmp");
	
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	//  SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	// SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdl->window, &sdl->renderer)
	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdl->window, &sdl->renderer) != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(sdl->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(sdl->window, "Spy Hunter | Ruslan Rabadanov 196634");
	sdl->scrtex = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_ARGB8888,
				SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_ShowCursor(SDL_DISABLE);

	if (sdl->charset == NULL || sdl->cars == NULL || sdl->playerCars == NULL) {
		printf("SDL_LoadBMP error: %s\n", SDL_GetError());
		SDL_FreeSurface(sdl->screen);
		SDL_DestroyTexture(sdl->scrtex);
		SDL_DestroyWindow(sdl->window);
		SDL_DestroyRenderer(sdl->renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(sdl->charset, true, 0x000000);
	return 0;
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


void DrawDest(Game* game, SDL* sdl, int* roadMarkingPos) {
	int grey = SDL_MapRGB(sdl->screen->format, 105, 105, 105);
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 3, 0, SCREEN_WIDTH / 3, SCREEN_HEIGHT, grey, grey);
	int grey_dark = SDL_MapRGB(sdl->screen->format, 40, 40, 40);
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 3 - SCREEN_WIDTH / 30, 0, SCREEN_WIDTH / 30, SCREEN_HEIGHT, grey_dark, grey_dark);
	DrawRectangle(sdl->screen, SCREEN_WIDTH * 2 / 3, 0, SCREEN_WIDTH / 30, SCREEN_HEIGHT, grey_dark, grey_dark);
	if (game->player.speed != 0) {
		*roadMarkingPos += (game->player.speed > 0 ? 4 * game->player.speed : (-7 * game->player.speed)) * game->time.delta * 100;
	}
	else {
		*roadMarkingPos += game->time.delta * 500;
	}
	if (*roadMarkingPos > SCREEN_HEIGHT + SCREEN_HEIGHT / 3) *roadMarkingPos -= SCREEN_HEIGHT / 3;
	// Draw road
	//int* tmpPos = (int*)malloc(sizeof(int));
	//*tmpPos = *roadMarkingPos;
	int tmpPos = *roadMarkingPos;
	while (tmpPos > -SCREEN_HEIGHT / 7) {
		DrawRoadRectangle(sdl->screen, tmpPos);
		tmpPos -= SCREEN_HEIGHT / 3;
	}
	//free(tmpPos);
};


void DrawRoadRectangle(SDL_Surface* screen, int y) {
	int grey_light = SDL_MapRGB(screen->format, 192, 192, 192);
	DrawRectangle(screen, SCREEN_WIDTH / 2, y, SCREEN_WIDTH / 50, SCREEN_HEIGHT / 7, grey_light, grey_light);
};


void DrawHeader(SDL_Surface* screen, Game game, SDL sdl, double fps) {
	char text[128];
	int czerwony = SDL_MapRGB(sdl.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(sdl.screen->format, 0x11, 0x11, 0xCC);
	DrawRectangle(sdl.screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	sprintf(text, "Ruslan Rabadanov 196634");
	DrawString(sdl.screen, sdl.screen->w / 2 - strlen(text) * 8 / 2, 10, text, sdl.charset);
	sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s\tScore: %.0f", game.time.total, fps, game.score);
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


void DrawPlayer(Game* game, SDL* sdl) {
	if (game->player.lives < 1) {
		printf("GAME END\n");
		return;
	}
	printf("%f\n", game->totalDistance);
	if (game->time.total > GOD_MODE_TIME) {
		if (game->totalDistance > 100) {
			game->player.player = sdl->playerCars[4];
		}
		else if (game->totalDistance > 80) {
			game->player.player = sdl->playerCars[3];
		}
		else if (game->totalDistance > 40) {
			game->player.player = sdl->playerCars[2];
		}
		else if (game->totalDistance > 20) {
			game->player.player = sdl->playerCars[1];
		}
	}
	DrawSurface(sdl->screen, game->player.player, game->player.coord.x, game->player.coord.y);
}


void DrawBullet(CarInfo* cars, Game* game, SDL* sdl) {
	if (game->bullet.coord.x == 0) return;
	if (game->bullet.coord.y > -game->bullet.bullet->h) {
		game->bullet.coord.y -= game->time.delta * game->bullet.speed;
		DrawSurface(sdl->screen, game->bullet.bullet, game->bullet.coord.x, game->bullet.coord.y);
		int result = carIsKilled(game, cars, sdl);
		if (result) {
			game->bullet.coord.x = 0;
			game->bullet.coord.y = 0;
			switch (result) {
			case -1:
				printf("+3s\n");
				game->time.scoreFreeze += 3;
				break;
			case 1:
				printf("+1000\n");
				game->score += 1000;
				break;
			}
		}
	}
	else {
		game->bullet.speed = 0;
	}
}


void DrawRandomCar(CarInfo* cars, Game* game, SDL* sdl) {
	for (int i = 0; i < ENEMIES; i++) {
		if (cars[i].coord.x != 0) {
			// Если уничтоженное, то слетает с дороги быстрее
			cars[i].speed = CAR_SPEED * isDestroyed(&cars[i], sdl) ? 500 : 250;
			double attackDirection = canAttack(&cars[i], game, cars);
			if (cars[i].isEnemy && attackDirection) {
				// canAttack определяет в какую сторону поедет актаковать
				cars[i].coord.y += attackDirection * cars[i].speed * game->time.delta;
			}
			else {
				cars[i].coord.y += game->time.delta * cars[i].speed;
			}
		}
	}

	// Create new car if needed (every X distance)
	if (int(game->totalDistance * 1000) % 181 == 0) {
		bool flag = true;
		for (int i = 0; i < ENEMIES; i++) {
			if (cars[i].coord.x == 0 && flag) {
				int car_num = rand() % 5;
				cars[i].car = sdl->cars[car_num];
				cars[i].speed = 200;
				if (car_num == 0 || car_num == 3) {
					cars[i].isEnemy = false;
				}
				else {
					cars[i].isEnemy = true;
				}
				cars[i].coord.y = -cars[i].car->h / 2;
				// Над экраном на высоту сгенерированного авто
				cars[i].coord.x = rand() % (SCREEN_WIDTH / 3) + SCREEN_WIDTH / 3;
				int iterCounter = 0;
				do {
					cars[i].coord.x = rand() % (SCREEN_WIDTH / 3) + SCREEN_WIDTH / 3;
				} while (++iterCounter < 30 && (!isFreePlace(&cars[i], cars, -1) || !isFreePlace(&cars[i], cars, 1))); // isFreePlace(&car, cars)
				flag = false;
				break;
			}
			else if (cars[i].coord.y > SCREEN_HEIGHT + cars[i].car->h / 2) {
				cars[i].coord.x = 0;
			}
		}
	}

	for (int i = 0; i < ENEMIES; i++) {
		if (cars[i].coord.x != 0) {
			if (touchObject(game, &cars[i], game->time.delta, cars, sdl)) {
				if (game->time.total > GOD_MODE_TIME) game->player.lives--;
				SpawnPlayer(game, cars);
			}
			else {
				DrawSurface(sdl->screen, cars[i].car, cars[i].coord.x, cars[i].coord.y);
			}
		}
	}
}


void NewGame( Game* game, CarInfo * cars) {
	SpawnPlayer(game, cars);
	game->player.player = SDL_LoadBMP("./assets/player_1.bmp");
	game->player.lives = 2;
	game->time.startGame = SDL_GetTicks();
	game->time.killMessage = 0;
	game->time.scoreFreeze = 0;
	game->time.delta = 0;
	game->time.total = 0;
	game->score = 0;
	game->totalDistance = 0;
	game->bullet.speed = 0;
	game->bullet.coord.y = -20;
};


void SpawnPlayer(Game* game, CarInfo* cars) {
	game->player.coord.x = SCREEN_WIDTH / 2;
	game->player.coord.y = SCREEN_HEIGHT * 2 / 3;
	game->player.speed = 0;
	game->player.turn = 0;
	for (int i = 0; i < ENEMIES; i++) {
		cars[i].coord.x = 0;
	}
};


// check if X and Y coordinates are inside a screen
bool inArray(int x, int y) {
	if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
		return true;
	}
	return false;
}


// Check ifhorizontal position is out of screen and do it in the range.
void fixCoordX(int* horizontal) {
	if (*horizontal < SCREEN_WIDTH / 2 - SCREEN_WIDTH / 6) {
		*horizontal = SCREEN_WIDTH / 2 - SCREEN_WIDTH / 6;
	}
	else if (*horizontal > SCREEN_WIDTH / 2 + SCREEN_WIDTH / 6) {
		*horizontal = SCREEN_WIDTH / 2 + SCREEN_WIDTH / 6;
	}
}


// Check if vertical position is out of screen and do it in the range.
void fixCoordY(int* vertical) {
	if (*vertical < 90) {
		*vertical = 90;
	}
	else if (*vertical > SCREEN_HEIGHT) {
		*vertical = SCREEN_HEIGHT;
	}
}


// x,y - координата на периметре машины; x2, y2 - верхняя левая точка объекта
bool numbersInArray(int x, int y, CarInfo* object) {
	int x2 = object->coord.x - object->car->w / 2;
	int y2 = object->coord.y - object->car->h / 2;
	// warn +1 -1
	if (x >= x2 && y >= y2 && x <= x2 + object->car->w && y <= y2 + object->car->h) {
		return true;
	}
	return false;
}


// Проверка для пикселей по периметру машины, находится ли позиция в другом объекте.
bool touchObject(Game* game, CarInfo* object, const double deltaTime, CarInfo *cars, SDL* sdl) {
	int x1 = game->player.coord.x;
	int y1 = game->player.coord.y;
	int x2 = object->coord.x;
	int y2 = object->coord.y;
	bool flag = true;

	for (int i = -object->car->h / 2 + 1; i < object->car->h / 2; i++) {
		if (numbersInArray(x1 - object->car->w / 2, y1 + i, object) ||
			numbersInArray(x1 + object->car->w / 2, y1 + i, object)) {
			if (isFreePlace(object, cars, game->player.turn)) {
				object->coord.x += game->player.turn * deltaTime * 300;
				if (object->coord.x < SCREEN_WIDTH / 3 - object->car->w ||
					object->coord.x > 2 * SCREEN_WIDTH / 3 + object->car->w)
				{
					// Car is destroyed
					object->car = sdl->cars[ENEMIES];
					if (object->isEnemy) {
						if (!game->time.scoreFreeze) {
							game->score += 1000;
							game->time.killMessage = 2;
						}
					}
					else {
						game->time.scoreFreeze += 3;
					}
				}
			}
			else {
				//game->car.coord.x -= game->car.turn * deltaTime * 200;
				// Проехать нельзя из-за преграды в виде авто, значит игрок сдвигается обратно
				game->player.coord.x -= game->player.turn * 6; // WARN
				// Если могу подвинуть авто, скорость - 200, иначе возвращаюсь на позицию назад где скорость 300
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


bool isDestroyed(CarInfo* car, SDL* sdl) {
	if (car->car == sdl->cars[ENEMIES]
		|| car->coord.x < SCREEN_WIDTH / 3 - car->car->w || car->coord.x > 2 * SCREEN_WIDTH / 3 + car->car->w) {
		return true;
	}
	return false;
}


bool isFreePlace(CarInfo* car, CarInfo* cars, int turn) {
	// turn = -1 : упирается справа, 1 : упирается слева
	int x, y, y2;
	y = car->coord.y + car->car->h / 2;
	y2 = y - car->car->h;
	if (turn == 1) {
		x = car->coord.x + car->car->w / 2;
	}
	else {
		x = car->coord.x - car->car->w / 2;
	}
	for (int i = 0; i < ENEMIES; i++) {
		// Если попал на свою же машину
		if (cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x || cars[i].coord.x == 0) continue;
		// проверяю нижнюю левую и нижнюю правую точки зареспавненной машины
		if (numbersInArray(x, y, &cars[i]) || numbersInArray(x, y2, &cars[i])) {
			// WARN ! 
			car->coord.x -= turn * 2;
			return false;
		}
	}
	return true;
}


// can attack if player is above or under the enemy car
// 2 - car is above, 0 - can't attack, -1 - car is under the enemy
double canAttack(CarInfo* car, Game* game, CarInfo* cars) {
	// Атакует если в одной полосе хотя бы половина машины
	if (inFault(game->player.coord.x, car->coord.x, 31)) {
		// CHANGE | SCREEN_HEIGHT/2
		if (inFault(game->player.coord.y, car->coord.y, SCREEN_HEIGHT) && game->player.coord.y - car->coord.y > car->car->h + 30) {
			// При атаке сверху тормозит за 30 пикселей от меня
			if (canGo(car, cars, 1)) return 1.8;
		}
		else if (inFault(game->player.coord.y, car->coord.y, SCREEN_HEIGHT) && game->player.coord.y - car->coord.y < -car->car->h - 10) {
			// При атаке тормозит за 10 пикселей от меня CHANGE
			if (canGo(car, cars, -1)) return -0.6;
		}
	}
	return 0;
}


// Проверка для атакующей машины может ли она ехать
bool canRide(CarInfo* car, CarInfo* cars) {
	// turn = -1 : упирается сверху, 1 : упирается снизу
	int y = car->coord.y;
	for (int i = 0; i < ENEMIES; i++) {
		// Если попал на свою же машину
		if (cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x || cars[i].coord.x == 0) continue;
		// проверяю нвходится ли в радиусе другое авто
		// WARN -2 -> -1 | Когда машины труться, чтобы могли идти в атаку
		// WARN +2 | Когда игра атакуют 2 машины в ряд, чтобы они не терлись
		if (inFault(cars[i].coord.y, y, car->car->h + 2) && inFault(cars[i].coord.x, car->coord.x, car->car->w - 2))
		{
			return false;
		}
	}
	return true;
}


// проверка с 4 сторон есть ли там машины
bool freeSpace(CarInfo* car, CarInfo* cars) {
	// turn = -1 : упирается сверху, 1 : упирается снизу
	int x = car->coord.x;
	int y = car->coord.y;
	for (int i = 0; i < ENEMIES; i++) {
		// Если попал на свою же машину
		if (cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x) continue;
		
		if (inFault(x, cars[i].coord.x, car->car->w + 5) && inFault(y, cars[i].coord.y, car->car->h + 5))
			return false;
	}
	return true;
}


bool inFault(int num1, int num2, int fault) {
	int num = (num1 - num2 > 0) ? num1 - num2 : num2 - num1;
	return (num < fault) ? true : false;
}


int modul(int num) {
	return num > 0 ? num : -num;
}


bool canGo(CarInfo* car, CarInfo* cars, int direction) {
	// direction: -1 -> вверх, 1 -> вниз
	int deltaY;
	
	for (int i = 0; i < ENEMIES; i++) {
		// Если попал на свою же машину
		if (cars[i].coord.x == 0 || cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x ||
			!inFault(cars[i].coord.x, car->coord.x, car->car->w)) continue;

		// WARN
		if (direction == -1) deltaY = car->coord.y - cars[i].coord.y;
		else deltaY = cars[i].coord.y - car->coord.y;
		
		if (deltaY > 0 && deltaY < car->car->h + 10) {
			return false;
		}
	}
	return true;
}


void addBullet(Game* game) {
	if (game->bullet.speed != 0) return;
	game->bullet.bullet = SDL_LoadBMP("./assets/bullet.bmp");
	game->bullet.coord.x = game->player.coord.x;
	game->bullet.coord.y = game->player.coord.y - 30;
	game->bullet.speed = 500;
}


int carIsKilled(Game* game, CarInfo* cars, SDL* sdl) {
	// return 0 - no one is killed, -1 - citizen is killed, 1 - enemy is killed
	for (int i = 0; i < ENEMIES; i++) {
		bool isEnemy = cars[i].isEnemy;
		if (cars[i].coord.x != 0) {
			if (inFault(game->bullet.coord.y, cars[i].coord.y, cars[i].car->h/2) &&
				inFault(game->bullet.coord.x, cars[i].coord.x, cars[i].car->w / 2)) {
				// Car is killed
				cars[i].car = sdl->cars[ENEMIES];
				cars[i].isEnemy = false;
				game->bullet.speed = 0;
				return isEnemy ? 1 : -1;
			}
		}
	}
	return 0;
}


void RenderSurfaces(SDL* sdl) {
	SDL_UpdateTexture(sdl->scrtex, NULL, sdl->screen->pixels, sdl->screen->pitch);
	SDL_RenderClear(sdl->renderer);
	SDL_RenderCopy(sdl->renderer, sdl->scrtex, NULL, NULL);
	SDL_RenderPresent(sdl->renderer);
}


void FreeSurfaces(SDL sdl) {
	SDL_FreeSurface(sdl.charset);
	SDL_FreeSurface(sdl.screen);
	SDL_DestroyTexture(sdl.scrtex);
	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.window);
	SDL_Quit();
}