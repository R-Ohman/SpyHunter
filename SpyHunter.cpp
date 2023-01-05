#include "SpyHunter.h"


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


void NewGame(struct Game* game, CarInfo * cars) {
	game->time = 0;
	game->score = 0;
	game->distance = 0;
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
						if (!game->freeze) {
							game->score += 1000;
							game->killMesTime = 2;
						}
					}
					else {
						printf("-1000 - %d\n", object->coord.x);
						game->freeze += 3;
					}
				}
			}
			else {
				//game->car.coord.x -= game->car.turn * deltaTime * 200;
				// Проехать нельзя из-за преграды в виде авто, значит игрок сдвигается обратно
				game->car.coord.x -= game->car.turn * 6; // WARN
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
	for (int i = 0; i < 5; i++) {
		// Если попал на свою же машину
		if (cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x || cars[i].coord.x == 0) continue;
		// проверяю нижнюю левую и нижнюю правую точки зареспавненной машины
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


// Проверка для атакующей машины может ли она ехать
bool canRide(struct CarInfo* car, struct CarInfo* cars) {
	// turn = -1 : упирается сверху, 1 : упирается снизу
	int y = car->coord.y;
	for (int i = 0; i < 5; i++) {
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


// can attack if player is above or under the enemy car
// 2 - car is above, 0 - can't attack, -1 - car is under the enemy
int canAttack(struct CarInfo* car, struct Game* game, struct CarInfo* cars) {
	if (inFault(game->car.coord.x, car->coord.x, 31)) {
		if (inFault(game->car.coord.y, car->coord.y, SCREEN_HEIGHT) && game->car.coord.y - car->coord.y > car->car->h + 30) {
			// При атаке сверху тормозит за 30 пикселей от меня
			if (!canRide(car, cars)) {
				// += 1
				car->coord.y -= 3;
				return 0;
			}
			return 2;
		}
		else if (inFault(game->car.coord.y, car->coord.y, SCREEN_HEIGHT) && game->car.coord.y - car->coord.y < -car->car->h - 10) {
			// При атаке тормозит за 10 пикселей от меня
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