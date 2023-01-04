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