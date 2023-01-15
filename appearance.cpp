#define _CRT_SECURE_NO_WARNINGS
#include "appearance.h"

#define BUTTON sdl->event.key.keysym.sym

void loadSprites(SDL* sdl) {
	// Loads sprites from the folder
	sdl->bullet = SDL_LoadBMP("./assets/bullet.bmp");
	sdl->powerup[0] = SDL_LoadBMP("./assets/powerup_1.bmp");
	sdl->powerup[1] = SDL_LoadBMP("./assets/powerup_2.bmp");
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
	sdl->liveIcon = SDL_LoadBMP("./assets/live.bmp");
	sdl->infinityIcon = SDL_LoadBMP("./assets/infinity.bmp");
	sdl->fireIcon = SDL_LoadBMP("./assets/fireSpeed.bmp");
}

 
int initGame(SDL* sdl) {
	sdl->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	sdl->charset = SDL_LoadBMP("./cs8x8.bmp");
	
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	//SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
	//	&sdl->window, &sdl->renderer);
	//SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdl->window, &sdl->renderer);
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

	if (sdl->charset == NULL) {
		printf("SDL_LoadBMP error: %s\n", SDL_GetError());
		FreeSurfaces(sdl);
		return 1;
	};
	SDL_SetColorKey(sdl->charset, true, 0x000000);
	// Loading sprites if there were no errors
	loadSprites(sdl);
	return 0;
}


// check if X and Y coordinates are inside a screen
bool inArray(int x, int y) {
	if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
		return true;
	}
	return false;
}


// Draw one pixel at the point (x, y)
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	if (inArray(x, y)) {
		int bpp = surface->format->BytesPerPixel;
		Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
		*(Uint32*)p = color;
	}
};


// Draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// Draw and paint a rectangle of size l by k
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


/* Draw a text 'txt' on surface 'screen', starting from the point (x, y)
	charset is a 128x128 bitmap containing character images */
void DrawString(SDL* sdl, int x, int y, const char* text) {
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
		SDL_BlitSurface(sdl->charset, &s, sdl->screen, &d);
		x += 8;
		text++;
	};
};


// Draw window at the center of the screen with green background
void drawDialogWindow(SDL* sdl) {
	SDL_FillRect(sdl->screen, NULL, SDL_MapRGB(sdl->screen->format, 107, 142, 35));
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2,
		SDL_MapRGB(sdl->screen->format, 192, 192, 192), SDL_MapRGB(sdl->screen->format, 40, 40, 40));
}


// Draws a light rectangle in the middle of the screen (road markings)
void DrawRoadRectangle(SDL_Surface* screen, int y) {
	int grey_light = SDL_MapRGB(screen->format, 192, 192, 192);
	DrawRectangle(screen, SCREEN_WIDTH / 2 - SCREEN_WIDTH / 80, y, SCREEN_WIDTH / 70, SCREEN_HEIGHT / 7, grey_light, grey_light);
};


void DrawDest(Game* game, SDL* sdl, int* roadMarkingPos) {
	// SDL_FillRect(sdl->screen, NULL, SDL_MapRGB(sdl->screen->format, 107, 142, 35));
	int grey = SDL_MapRGB(sdl->screen->format, 105, 105, 105);
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 2 - (int)game->roadWidth / 2, 0, game->roadWidth, SCREEN_HEIGHT, grey, grey);
	int grey_dark = SDL_MapRGB(sdl->screen->format, 40, 40, 40);
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 2 - SCREEN_WIDTH / 40 - (int)game->roadWidth / 2, 0, SCREEN_WIDTH / 40, SCREEN_HEIGHT, grey_dark, grey_dark);
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 2 + (int)game->roadWidth / 2, 0, SCREEN_WIDTH / 40, SCREEN_HEIGHT, grey_dark, grey_dark);

	int roadSpeed = 2 * CAR_SPEED;
	if (game->player.speed == -1)
		roadSpeed *= 1.5;
	else if (game->player.powerTime[1] > 0)
		roadSpeed *= 2;
	*roadMarkingPos += roadSpeed * game->time.delta * !game->pause;

	if (*roadMarkingPos > SCREEN_HEIGHT + SCREEN_HEIGHT / 3) *roadMarkingPos -= SCREEN_HEIGHT / 3;
	int tmpPos = *roadMarkingPos;
	// Until the road marking goes beyond the top of the screen, draw it every 1/3 of the screen
	while (tmpPos > -SCREEN_HEIGHT / 7) {
		DrawRoadRectangle(sdl->screen, tmpPos);
		tmpPos -= SCREEN_HEIGHT / 3;
	}
};


// In the upper left part of the screen writes the author and information about the current game
void DrawHeader(Game game, SDL* sdl, double fps) {
	char* text = (char*)malloc(128 * sizeof(char));
	sprintf(text, "Ruslan Rabadanov 196634");
	DrawString(sdl, SPACING, 10, text);
	sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s Score: %.0f", game.time.total, fps, game.score);
	DrawString(sdl, SPACING, SPACING, text);
	free(text);
	text = NULL;
}


// In the lower right part of the screen writes out the functionality of the game
void DrawMenu(SDL* sdl) {
	char* text[48] = {
		"N - New Game",
		"ESC - Exit",
		"\030/\031 - acceleration / deceleration",
		"\032/\033 - left / right",
		"Space - Shoot",
		"F - End Game",
		"P - Pause",
		"S - Save Game",
		"L - Load Game"
	};
	for (int i = 0; i < 9; i++) {
		DrawString(sdl, SCREEN_WIDTH - strlen(text[i]) * 9, SCREEN_HEIGHT / 2 + SPACING * i, text[i]);
	}
}


// Writes messages related to game events in the lower left of the screen
void DrawCommunicates(Game game, SDL* sdl) {
	char* text = (char*)malloc(128 * sizeof(char));
	if (game.time.scoreFreeze) {
		sprintf(text, "Score is freezed on %.1f sec", game.time.scoreFreeze);
		DrawString(sdl, SCREEN_WIDTH / 2 - strlen(text) * 4, SPACING * 1.5, text);
	}
	else if (game.time.killMessage > 0) {
		sprintf(text, "KILL! You get 1000 points!");
		DrawString(sdl, SPACING, SCREEN_HEIGHT - 3 * SPACING, text);
	}
	if (game.time.total < GOD_MODE_TIME) {
		sprintf(text, "You have infinite lives until %.1f sec", GOD_MODE_TIME - game.time.total);
		DrawString(sdl, SPACING, SCREEN_HEIGHT - SPACING, text);
		sprintf(text, "%.1fs", GOD_MODE_TIME - game.time.total);
		// Writes out the time until the end of immortality under the infinity icon and under the player
		DrawString(sdl, SCREEN_WIDTH / 2 - strlen(text) * 4, SPACING, text);
		DrawString(sdl, game.player.coord.x - strlen(text) * 4, game.player.coord.y + sdl->playerCars[0]->h / 2, text);
	}
	if (game.player.powerTime[0] > 0) {
		sprintf(text, "You got weapon until %.1f sec", game.player.powerTime[0]);
		DrawString(sdl, SPACING, SCREEN_HEIGHT - 3 * SPACING, text);
	}
	if (game.player.powerTime[1] > 0) {
		sprintf(text, "You have extra speed until %.1f sec", game.player.powerTime[1]);
		DrawString(sdl, SPACING, SCREEN_HEIGHT - 2 * SPACING, text);
	}
	if (game.pause) {
		// Gray line in the middle of the screen
		DrawRectangle(sdl->screen, 0, SCREEN_HEIGHT/2 - SPACING, SCREEN_WIDTH, SPACING,
			SDL_MapRGB(sdl->screen->format, 192, 192, 192), SDL_MapRGB(sdl->screen->format, 40, 40, 40));
		sprintf(text, "Game paused!");
		DrawString(sdl, SCREEN_WIDTH/2 - strlen(text)*4, SCREEN_HEIGHT/2 - SPACING/2, text);
	}
	free(text);
	text = NULL;
};


void DrawInterface(Game game, SDL* sdl, double fps) {
	DrawHeader(game, sdl, fps);
	// Hearts icons (infinity icon) are always displayed at the center of the screen
	for (int i = 0; i < game.player.lives; i++) {
		int pos_x = (SCREEN_WIDTH / 2 - (game.player.lives - 1) * (game.time.total > GOD_MODE_TIME ? sdl->liveIcon->w : sdl->infinityIcon->w) / 2) + SPACING * i;
		DrawSurface(sdl->screen, game.time.total > GOD_MODE_TIME ? sdl->liveIcon : sdl->infinityIcon, pos_x, SPACING / 2);
	}
	DrawCommunicates(game, sdl);
	DrawMenu(sdl);
};


void SaveGame(Game* game, CarInfo* cars, SDL* sdl) {
	FILE* out;
	time_t now = time(0);
	tm* time = localtime(&now);
	char* path = (char*)malloc(32 * sizeof(char));
	if (path == NULL) {
		printf("Memory allocation failed\n");
		return;
	}
	strftime(path, 30, "saves/%d-%m-%Y_%H-%M-%S.dat", time);
	out = fopen(path, "wb");
	if (out == NULL) {
		// If the "saved" folder doesn't exist
		strftime(path, 30, DATE_FORMAT, time);
		out = fopen(path, "wb");
		if (out == NULL) {
			printf("Error while saving game\n");
			free(path);
			path = NULL;
			return;
		}
	}
	free(path); 
	path = NULL;
	Save save;
	save.game = *game;
	for (int i = 0; i < ENEMIES; i++)
		save.cars[i] = cars[i];

	if (fwrite(&save, sizeof(Save), 1, out) != 1)
	{
		printf("Error while writing file.\n");
		return;
	}
	fclose(out);
}


// Returns true if path has '.dat' at the end
bool correctFormat(char* path) {
	for (int i = 0; i < 256; i++)
		if (path[i] == '\0')
			if (path[i - 4] != '.' || path[i - 3] != 'd' || path[i - 2] != 'a' || path[i - 1] != 't')
				return false;
	return true;
}


int LoadGame(Game* game, CarInfo* cars, SDL* sdl, char* filePath) {
	FILE* in = fopen(filePath, "rb");
	if (in == NULL) {
		printf("Error while opening the file\n");
		return 0;
	}
	if (!correctFormat(filePath))
		return 0;
	
	Save save;
	fread(&save, sizeof(Save), 1, in);
	fclose(in);
	*game = save.game;
	for (int i = 0; i < ENEMIES; i++) {
		cars[i] = save.cars[i];
	}
	return 1;
}


int ShowSavedGames(Game* game, CarInfo* cars, SDL* sdl) {
	OPENFILENAMEA ofn;
	char* szFile = (char*)malloc(256 * sizeof(char));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 256;
	ofn.lpstrFilter = ".dat";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	// Check if file is selected
	if (GetOpenFileNameA(&ofn) == TRUE) {
		if (!LoadGame(game, cars, sdl, szFile)) return 0;
		free(szFile);
		szFile = NULL;
		return 1;
	}
	free(szFile);
	szFile = NULL;
	return 0;
}


// Draws a dialog box in the center of the screen prompting you to save the game result
void addResultMenu(SDL* sdl) {
	char* text = (char*)malloc(128 * sizeof(char));
	if (text == NULL) {
		printf("Memory allocation error.\n");
		return;
	}
	SDL_FillRect(sdl->screen, NULL, SDL_MapRGB(sdl->screen->format, 107, 142, 35));
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2,
		SDL_MapRGB(sdl->screen->format, 192, 192, 192), SDL_MapRGB(sdl->screen->format, 40, 40, 40));
	sprintf(text, "SPY HUNTER");
	DrawString(sdl, SCREEN_WIDTH / 2 - strlen(text) * 4, SCREEN_HEIGHT / 4 + 30, text);
	sprintf(text, "Add result to your list of best games?");
	DrawString(sdl, SCREEN_WIDTH / 2 - strlen(text) * 4, SCREEN_HEIGHT / 4 + 60, text);
	sprintf(text, "Y - Yes");
	DrawString(sdl, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 4 + 80, text);
	sprintf(text, "N - No");
	DrawString(sdl, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 4 + 80, text);
	RenderSurfaces(sdl);
	free(text);
	text = NULL;
}


void AddResult(Game* game, vector_t* resultsList, SDL* sdl) {
	addResultMenu(sdl);
	char action = '\0';
	do {
		while (SDL_PollEvent(&sdl->event)) {
			switch (sdl->event.type) {
			case SDL_KEYDOWN:
				if (sdl->event.key.keysym.sym == SDLK_y) action = 'Y';
				else if (sdl->event.key.keysym.sym == SDLK_n) action = 'N';
				break;
			};
		}
	} while (action == '\0');
	if (action == 'N') return;
	// Add time and count to the vector
	Result result{ game->time.total, (int)game->score };
	push_back(resultsList, result);
}


void SaveResults(vector_t* resultsList) {
	FILE* out;
	out = fopen("saves/results.txt", "w");
	if (out == NULL) {
		out = fopen("results.txt", "w");
		if (out == NULL) {
			// If the file 'results.txt' is not found
			printf("Error while saving results\n");
			return;
		}
	}
	for (int i = 0; i < resultsList->count; i++)
		fprintf(out, "%f %d\n", resultsList->ptr[i].time, resultsList->ptr[i].score);
	fclose(out);
}


void LoadResults(vector_t* resultsList) {
	FILE* in = fopen("saves/results.txt", "r");
	if (in == NULL) {
		in = fopen("results.txt", "r");
		if (in == NULL) {
			// If the file 'results.txt' is not found
			printf("Error while loading results\n");
			return;
		}
	}
	Result result = { 0, 0 };
	// FIX ME
	while (fscanf(in, "%lf %d\n", &result.time, &result.score) == 2)
		push_back(resultsList, result);
	fclose(in);
}


// Function for QuickSort
int sortByScore(const void* a, const void* b) {
	struct Result x = *(struct Result*)a;
	struct Result y = *(struct Result*)b;
	if (x.score == y.score) return (y.time - x.time);
	return (y.score - x.score);

}


// Function for QuickSort
int sortByTime(const void* a, const void* b) {
	struct Result x = *(struct Result*)a;
	struct Result y = *(struct Result*)b;
	if (x.time == y.time) return (y.score - x.score);
	return 1000 * (y.time - x.time);
}


void getEvent(Game* game, CarInfo* cars, SDL* sdl, int* quit, int* time) {
	while (SDL_PollEvent(&sdl->event)) {
		switch (sdl->event.type) {
		case SDL_KEYDOWN:
			if (BUTTON == SDLK_ESCAPE) *quit = 1;
			else if (BUTTON == SDLK_n) NewGame(game, cars, sdl);
			else if (BUTTON == SDLK_f) game->player.lives = 0;
			else if (BUTTON == SDLK_p) game->pause = !game->pause;
			else if (BUTTON == SDLK_s)  SaveGame(game, cars, sdl);
			else if (BUTTON == SDLK_l) ShowSavedGames(game, cars, sdl);
			else if (BUTTON == SDLK_UP) game->player.speed = -1;
			else if (BUTTON == SDLK_DOWN) game->player.speed = 1;
			else if (BUTTON == SDLK_LEFT) game->player.turn = -1;
			else if (BUTTON == SDLK_RIGHT) game->player.turn = 1;
			else if (BUTTON == SDLK_SPACE && !game->pause) addBullet(game, sdl);
			break;
		case SDL_KEYUP:
			if (BUTTON == SDLK_UP || BUTTON == SDLK_DOWN)
				game->player.speed = 0;
			else if (BUTTON == SDLK_LEFT || BUTTON == SDLK_RIGHT)
				game->player.turn = 0;
			break;
		case SDL_QUIT:
			*quit = 1;
			break;
		};
	};
}


void getResultsMenuAction(int* page, SDL* sdl, vector_t* resultsList, const int place, Game* game, CarInfo* cars) {
	char action = '\0';
	do {
		while (SDL_PollEvent(&sdl->event)) {
			switch (sdl->event.type) {
			case SDL_KEYDOWN:
				if (BUTTON == SDLK_LEFT && *page > 0) action = 'L';
				else if (BUTTON == SDLK_RIGHT && place < resultsList->count) action = 'R';
				else if (BUTTON == SDLK_b) action = 'B';
				break;
			};
		}
	} while (action == '\0');

	switch (action) {
	case 'B':
		welcomeMenu(sdl, resultsList, game, cars, 0);
		break;
	case 'L':
		(*page)--;
		topResultsMenu(sdl, resultsList, game, cars);
		break;
	case 'R':
		(*page)++;
		topResultsMenu(sdl, resultsList, game, cars);
		break;
	}
}


void topResultsMenu(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars) {
	static int page = 0;
	char* text = (char*)malloc(128 * sizeof(char));
	if (text == NULL) {
		printf("Memory allocation error.\n");
		return;
	}
	drawDialogWindow(sdl);
	
	sprintf(text, "BEST GAMES LIST");
	DrawString(sdl, SCREEN_WIDTH / 2 - strlen(text) * 4, SCREEN_HEIGHT / 4 + SPACING, text);
	int place;
	for (place = page * RES_PER_PAGE; place < (page + 1) * RES_PER_PAGE && place < resultsList->count; place++) {
		sprintf(text, "%d. Score: %d", place + 1, resultsList->ptr[place].score);
		DrawString(sdl, SCREEN_WIDTH / 3 + 50, SCREEN_HEIGHT / 4 + 2 * SPACING + 20 * (place - page * RES_PER_PAGE), text);
		sprintf(text, "Time: %.1f s", resultsList->ptr[place].time);
		DrawString(sdl, SCREEN_WIDTH / 2 + 50, SCREEN_HEIGHT / 4 + 2 * SPACING + 20 * (place - page * RES_PER_PAGE), text);
	}
	// Write function keys at the bottom of the rating window
	sprintf(text, "\032/\033 - Previous/Next page");
	DrawString(sdl, 2 * SCREEN_WIDTH / 3 - strlen(text) * 8 - SPACING, 3 * SCREEN_HEIGHT / 4 - SPACING, text);
	sprintf(text, "B - Back");
	DrawString(sdl, SCREEN_WIDTH / 3 + strlen(text) * 8 + SPACING, 3 * SCREEN_HEIGHT / 4 - SPACING, text);
	
	free(text);
	text = NULL;
	RenderSurfaces(sdl);
	getResultsMenuAction(&page, sdl, resultsList, place, game, cars);
}


void getWelcomeMenuAction(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars, int* quit) {
	char action = '\0';
	do {
		while (SDL_PollEvent(&sdl->event)) {
			switch (sdl->event.type) {
			case SDL_KEYDOWN:
				if (BUTTON == SDLK_n) action = 'N';
				else if (BUTTON == SDLK_l) action = 'L';
				else if (BUTTON == SDLK_p && resultsList->count) action = 'P';
				else if (BUTTON == SDLK_t && resultsList->count) action = 'T';
				else if (BUTTON == SDLK_ESCAPE) action = 'Q';
				break;
			};
		}
	} while (action == '\0');

	switch (action) {
	case 'N':
		NewGame(game, cars, sdl);
		break;
	case 'L':
		if (!ShowSavedGames(game, cars, sdl)) // If the user has not selected a file to upload
			welcomeMenu(sdl, resultsList, game, cars, quit);
		break;
	case 'P':
		qsort(resultsList->ptr, resultsList->count, sizeof(struct Result), sortByScore);
		topResultsMenu(sdl, resultsList, game, cars);
		break;
	case 'T':
		qsort(resultsList->ptr, resultsList->count, sizeof(struct Result), sortByTime);
		topResultsMenu(sdl, resultsList, game, cars);
		break;
	case 'Q':
		// Skip game loop
		(*quit)++;
		break;
	}
}


void welcomeMenu(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars, int* quit) {
	drawDialogWindow(sdl);
	char text[][32] = {
		"SPY HUNTER",
		"N - New Game",
		"L - Load Game",
		"ESC - Exit"
	};
	for (int i = 0; i < 4; i++)
		DrawString(sdl, SCREEN_WIDTH / 2 - strlen(text[i]) * 4, SCREEN_HEIGHT / 4 + SPACING * (i + 1), text[i]);
	if (resultsList->count > 0) {
		sprintf(*text, "P - Best results by points");
		DrawString(sdl, SCREEN_WIDTH / 3 + 100, SCREEN_HEIGHT / 4 + SPACING * 5, *text);
		sprintf(*text, "T - Best results by time");
		DrawString(sdl, SCREEN_WIDTH / 3 + 100, SCREEN_HEIGHT / 4 + SPACING * 6, *text);
	}
	RenderSurfaces(sdl);
	getWelcomeMenuAction(sdl, resultsList, game, cars, quit);
}


void RenderSurfaces(SDL* sdl) {
	SDL_UpdateTexture(sdl->scrtex, NULL, sdl->screen->pixels, sdl->screen->pitch);
	SDL_RenderClear(sdl->renderer);
	SDL_RenderCopy(sdl->renderer, sdl->scrtex, NULL, NULL);
	SDL_RenderPresent(sdl->renderer);
}


void FreeSurfaces(SDL* sdl) {
	SDL_FreeSurface(sdl->charset);
	SDL_FreeSurface(sdl->screen);
	SDL_DestroyTexture(sdl->scrtex);
	SDL_DestroyRenderer(sdl->renderer);
	SDL_DestroyWindow(sdl->window);
	SDL_Quit();
}