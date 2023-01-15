#pragma once

#ifndef _SPYHUNTER_H_
#define _SPYHUNTER_H_

#include "definitions.h"


/* When necessary, changes the player's skin.
	In case of accumulation of 5000 points, adds life.
	In case of 0 lives, returns 1. */
int DrawPlayer(Game* game, SDL* sdl);

// Draws a bullet and checks if it killed another car
void DrawBullet(CarInfo* cars, Game* game, SDL* sdl);


// Draws cars. Spawns a car if necessary. Tests the sticking with the player
void DrawRandomCar(CarInfo* cars, Game* game, SDL* sdl);


// Spawn power-up if necessary. Draws it and checks if the player picked it up
void DrawRandomPower(CarInfo* cars, Game* game, SDL* sdl);


void SpawnPlayer(Game* game, CarInfo* cars);


// check if X and Y coordinates are inside a screen
bool inArray(int x, int y);


// Prevents the player from going above 90px and below the screen
void fixCoordY(double* vertical);


/* Checks if the player is touching the car.
  If it touches from the side - it tries to move the car,
  and if it touches from above or below - the player loses a life */
bool touchObject(Game* game, CarInfo* object, const double deltaTime, CarInfo* cars, SDL* sdl);


// Return true if car has a destroyed car sprite
bool isDestroyed(CarInfo* car);


// Checks if there is another vehicle in the direction of the car
bool isFreePlace(CarInfo* car, CarInfo* cars, SDL* sdl, int turn);


/* Returns 1 if the auto attacks down, -1 if it attacks up,
   0 if it doesn't attack (is not in the same lane along the X axis as the player) */
double canAttack(CarInfo* car, Game* game, CarInfo* cars, SDL* sdl);


// Returns true if modulo difference is less than 'fault'
bool inFault(int num1, int num2, int fault);


// Checks if there are other vehicles on the top/bottom of the car
bool canGo(CarInfo* car, CarInfo* cars, SDL* sdl, int direction);


// Returns false if there is a car within a radius of 5 pixels
bool canSpawn(Game* game, CarInfo* cars, SDL* sdl, const int powerIndex);


// Checks if the bullet killed the car. If yes, changes the sprite of the car
int carIsKilled(Game* game, CarInfo* cars, SDL* sdl, int y);


// Updates the timers. Changes the position of the player. Adds points if needed.
void movePlayerCar(Game* game, CarInfo* cars);

#endif