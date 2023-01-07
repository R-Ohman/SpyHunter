#pragma once
#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include <time.h>
#include<string.h>
#include <stdlib.h>
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"

#define SCREEN_WIDTH	960
#define SCREEN_HEIGHT	720

typedef struct Game {
	struct Car {
		struct Coord {
			int x;
			int y;
		} coord;
		int speed;
		int turn;
	} car;
	struct {
		double startGame;
		double total;
		double delta;
		double scoreFreeze;
		double killMessage;
	} time;
	struct {
		SDL_Surface* bullet;
		struct {
			int x;
			int y;
		} coord;
		int speed;
		double distance;
	} bullet;
	double totalDistance;
	double score;
};


typedef struct CarInfo {
	SDL_Surface* car;
	struct Coord {
		int x;
		int y;
	} coord;
	double speed;
	bool isEnemy;
};

typedef struct SDL {
	SDL_Event event;
	SDL_Surface* screen;
	SDL_Surface* charset;
	SDL_Surface* player;
	SDL_Surface* cars[6];
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
};


int initGame(SDL* sdl);


// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset);


void NewGame(struct Game* game, CarInfo* cars);


// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y);



// check if X and Y coordinates are inside a screen
bool inArray(int x, int y);


// Check if vertical position is out of screen and do it in the range.
void fixCoordY(int* vertical);


// Check ifhorizontal position is out of screen and do it in the range.
void fixCoordX(int* horizontal);


// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color);


// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color);


// x,y - координата на периметре машины; x2, y2 - центр объекта
bool numbersInArray(int x, int y, CarInfo* object);


// Проверка для пикселей по периметру машины, находится ли позиция в другом объекте.
bool touchObject(struct Game* game, CarInfo* object, const double deltaTime, CarInfo *cars, SDL* sdl);


// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor);


void DrawDest(Game* game, SDL* sdl, int* roadMarkingPos);


bool isDestroyed(struct CarInfo* car, SDL* sdl);


void DrawRoadRectangle(SDL_Surface* screen, int y);


// Retern string - path to the BMP image
int randomCar();


bool isFreePlace(struct CarInfo* car, struct CarInfo* cars, int turn);


int canAttack(struct CarInfo* car, struct Game* game, struct CarInfo* cars);


bool canRide(struct CarInfo* car, struct CarInfo* cars);


bool freeSpace(struct CarInfo* car, struct CarInfo* cars);


bool inFault(int num1, int num2, int fault);


void DrawHeader(SDL_Surface* screen, Game game, SDL sdl, double fps);


int modul(int num);


void drawRandomCar(CarInfo* cars, Game* game, SDL* sdl);


void freeSurfaces(SDL sdl);


void renderSurfaces(SDL* sdl);


bool canGo(struct CarInfo* car, struct CarInfo* cars, int direction);


void addBullet(Game* game);


void drawBullet(CarInfo* cars, Game* game, SDL* sdl);


int carIsKilled(Game* game, CarInfo* cars, SDL* sdl);