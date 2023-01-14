#pragma once

#ifndef _APPEARANCE_H_
#define _APPEARANCE_H_

#include "definitions.h"


int initGame(SDL* sdl);


// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color);


// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color);


// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor);


// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset);


void DrawDest(Game* game, SDL* sdl, int* roadMarkingPos);


void DrawMenu(SDL sdl);


void DrawRoadRectangle(SDL_Surface* screen, int y);


void DrawHeader(SDL_Surface* screen, Game game, SDL sdl, double fps);


void DrawCommunicates(SDL_Surface* screen, Game game, SDL sdl);


void DrawMenu(SDL sdl);


void SaveGame(Game* game, CarInfo* cars, SDL* sdl);


void LoadGame(Game* game, CarInfo* cars, SDL* sdl, char filePath[250]);


void ShowSavedGames(Game* game, CarInfo* cars, SDL* sdl);


void AddResult(Game* game, vector_t* resultsList, SDL* sdl);


void SaveResults(vector_t* resultsList);


void LoadResults(vector_t* resultsList);


void getEvent(Game* game, CarInfo* cars, SDL* sdl, int* quit, int* time);


void topResultsMenu(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars);


void welcomeMenu(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars, int* quit);


void getResultsMenuAction(int* page, SDL* sdl, vector_t* resultsList, const int place, Game* game, CarInfo* cars);


void getWelcomeMenuAction(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars, int* quit);


void RenderSurfaces(SDL* sdl);


void FreeSurfaces(SDL sdl);


// check if X and Y coordinates are inside a screen
bool inArray(int x, int y);

#endif