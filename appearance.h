#pragma once

#ifndef _APPEARANCE_H_
#define _APPEARANCE_H_

#include "definitions.h"


// Loading all sprites and creating a game window
int initGame(SDL* sdl);


// Draw the road, curb and road markings
void DrawDest(Game* game, SDL* sdl, int* roadMarkingPos);


// Writes out all the necessary text information and draws icons
void DrawInterface(Game game, SDL* sdl);


// Save the state of the game in a file named as the current time
void SaveGame(Game* game, CarInfo* cars, SDL* sdl);

/* Loads the game state from a file.
	Returns 0 if the file can't be opened or the file has an invalid format (not .dat) */
int LoadGame(Game* game, CarInfo* cars, SDL* sdl, char* filePath);


/* Opens a Windows window to select a file.
	In case of loading a file with the correct format, returns 1, otherwise 0 */
int ShowSavedGames(Game* game, CarInfo* cars, SDL* sdl);


// Invites the player to save the result, and if he agrees, it saves
void AddResult(Game* game, vector_t* resultsList, SDL* sdl);


// Writes all data about best results from vector to file
void SaveResults(vector_t* resultsList);


// Loads all data from a file into a vector
void LoadResults(vector_t* resultsList);


// Keep track of the player's actions during the game
void getEvent(Game* game, CarInfo* cars, SDL* sdl, int* quit, int* time);


void topResultsMenu(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars);


void welcomeMenu(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars, int* quit);


void RenderSurfaces(SDL* sdl);


// Freeing all surfaces
void FreeSurfaces(SDL* sdl);

#endif