#define _CRT_SECURE_NO_WARNINGS
#include "SpyHunter.h"


int initGame(SDL* sdl) {
	sdl->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	sdl->charset = SDL_LoadBMP("./cs8x8.bmp");
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

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	  SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
	                                 &sdl->window, &sdl->renderer);
	// SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdl->window, &sdl->renderer)
	//if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdl->window, &sdl->renderer) != 0) {
	//	SDL_Quit();
	//	printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
	//	return 1;
	//};

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
	// SDL_FillRect(sdl->screen, NULL, SDL_MapRGB(sdl->screen->format, 107, 142, 35));
	int grey = SDL_MapRGB(sdl->screen->format, 105, 105, 105);
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 2 - (int)game->roadWidth / 2, 0, game->roadWidth, SCREEN_HEIGHT, grey, grey);
	int grey_dark = SDL_MapRGB(sdl->screen->format, 40, 40, 40);
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 2 - SCREEN_WIDTH / 40 - (int)game->roadWidth / 2, 0, SCREEN_WIDTH / 40, SCREEN_HEIGHT, grey_dark, grey_dark);
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 2 + (int)game->roadWidth / 2, 0, SCREEN_WIDTH / 40, SCREEN_HEIGHT, grey_dark, grey_dark);
	if (game->player.speed != 0) {
		*roadMarkingPos += (game->player.speed > 0 ? 4 * game->player.speed : (-7 * game->player.speed)) * game->time.delta * 100;
	}
	else {
		*roadMarkingPos += game->time.delta * 2 * CAR_SPEED;
	}
	if (*roadMarkingPos > SCREEN_HEIGHT + SCREEN_HEIGHT / 3) *roadMarkingPos -= SCREEN_HEIGHT / 3;
	// Draw road
	int tmpPos = *roadMarkingPos;
	while (tmpPos > -SCREEN_HEIGHT / 7) {
		DrawRoadRectangle(sdl->screen, tmpPos);
		tmpPos -= SCREEN_HEIGHT / 3;
	}
};


void DrawRoadRectangle(SDL_Surface* screen, int y) {
	int grey_light = SDL_MapRGB(screen->format, 192, 192, 192);
	DrawRectangle(screen, SCREEN_WIDTH / 2, y, SCREEN_WIDTH / 70, SCREEN_HEIGHT / 7, grey_light, grey_light);
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

	DrawCommunicates(screen, game, sdl);
	DrawMenu(sdl);
};


void DrawMenu(SDL sdl) {
	char* text[64] = {
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
		DrawString(sdl.screen, SCREEN_WIDTH - strlen(text[i]) * 9, SCREEN_HEIGHT / 2 + SPACING * i, text[i], sdl.charset);
	}
}


void DrawCommunicates(SDL_Surface* screen, Game game, SDL sdl) {
	char text[128];
	if (game.time.deadMessage > 0) {
		sprintf(text, "You dead! New game..., %.1f", game.time.deadMessage);
		DrawString(sdl.screen, SPACING, SCREEN_HEIGHT / 2, text, sdl.charset);
	}
	else if (game.time.scoreFreeze) {
		sprintf(text, "Score is freezed on %.1f sec", game.time.scoreFreeze);
		DrawString(sdl.screen, SPACING, SCREEN_HEIGHT / 2, text, sdl.charset);
	}
	else if (game.time.killMessage > 0) {
		sprintf(text, "KILL! You get 1000 points!");
		DrawString(sdl.screen, SPACING, SCREEN_HEIGHT / 2, text, sdl.charset);
	}
	if (game.player.lives) {
		if (game.time.total > GOD_MODE_TIME)
			sprintf(text, "You have %d lives!", game.player.lives);
		else sprintf(text, "You have infinite lives until %.1f sec", GOD_MODE_TIME - game.time.total);
		DrawString(sdl.screen, SPACING, SCREEN_HEIGHT / 2 - 20, text, sdl.charset);
	}
	if (game.player.power.time > 0) {
		sprintf(text, "You got weapon until %.1f sec", game.player.power.time);
		DrawString(sdl.screen, SPACING, SCREEN_HEIGHT / 2 - 40, text, sdl.charset);
	}
	if (game.pause) {
		sprintf(text, "Game paused!");
		DrawString(sdl.screen, SPACING, SCREEN_HEIGHT / 2 - 60, text, sdl.charset);
	}
};


int DrawPlayer(Game* game, SDL* sdl) {
	if (game->player.lives < 1) {
		printf("GAME END\n");
		return 1;
	}
	/// 5000 points for getting 1 extra live
	if (game->player.liveGain < (int)game->score / 5000) {
		game->player.lives++;
		game->player.liveGain++;
	}
	if (game->time.total > GOD_MODE_TIME) {
		if (game->totalDistance > 100) {
			game->player.sprite = sdl->playerCars[4];
			game->player.colorIndex = 4;
		}
		else if (game->totalDistance > 80) {
			game->player.sprite = sdl->playerCars[3];
			game->player.colorIndex = 3;
		}
		else if (game->totalDistance > 40) {
			game->player.sprite = sdl->playerCars[2];
			game->player.colorIndex = 2;
		}
		else if (game->totalDistance > 20) {
			game->player.sprite = sdl->playerCars[1];
			game->player.colorIndex = 1;
		}
		else {
			game->player.colorIndex = 0;
		}
	}
	DrawSurface(sdl->screen, game->player.sprite, game->player.coord.x, game->player.coord.y);

	return 0;
}


void DrawBullet(CarInfo* cars, Game* game, SDL* sdl) {
	if (game->bullet.coord.x == 0) return;
	if (game->bullet.coord.y > -game->bullet.sprite->h || game->bullet.coord.y2 != 0 && game->bullet.coord.y2 < SCREEN_HEIGHT + game->bullet.sprite->h) {
		if (game->bullet.coord.y > -game->bullet.sprite->h) game->bullet.coord.y -= game->time.delta * game->bullet.speed;
		if (game->bullet.coord.y2) game->bullet.coord.y2 += game->time.delta * game->bullet.speed * 2;
		DrawSurface(sdl->screen, game->bullet.sprite, game->bullet.coord.x, game->bullet.coord.y);
		if (game->bullet.coord.y2) DrawSurface(sdl->screen, game->bullet.sprite, game->bullet.coord.x, game->bullet.coord.y2);
		int resultUp = carIsKilled(game, cars, sdl, game->bullet.coord.y);
		int resultDown = 0;
		if (game->bullet.coord.y2) resultDown = carIsKilled(game, cars, sdl, game->bullet.coord.y2);
		if (resultUp) {
			game->bullet.coord.y = -200;
			switch (resultUp) {
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
		if (resultDown) {
			game->bullet.coord.y2 = SCREEN_HEIGHT + 200;
			switch (resultDown) {
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
		game->bullet.coord.x = 0;
	}
}


void DrawRandomCar(CarInfo* cars, Game* game, SDL* sdl) {
	for (int i = 0; i < ENEMIES; i++) {
		if (cars[i].coord.x != 0) {
			// ≈сли уничтоженное, то слетает с дороги быстрее
			// ≈сли пауза - умножаем на 0 -> не двигаемс€
			cars[i].speed = (isDestroyed(&cars[i], sdl, game) ? 2 : 1) * (!game->pause) * CAR_SPEED;
			double attackDirection = canAttack(&cars[i], game, cars);
			if (cars[i].isEnemy && attackDirection) {
				// canAttack определ€ет в какую сторону поедет актаковать
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
				cars[i].colorIndex = car_num;
				cars[i].speed = 200;
				if (car_num == 0 || car_num == 3) {
					cars[i].isEnemy = false;
				}
				else {
					cars[i].isEnemy = true;
				}
				cars[i].coord.y = -cars[i].car->h / 2;
				// Ќад экраном на высоту сгенерированного авто
				int iterCounter = 0;
				do {
					cars[i].coord.x = rand() % ((int)game->roadWidth) + LEFT_BORDER;
				} while (!canGo(&cars[i], cars, 1)); // isFreePlace(&car, cars)
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
				printf("Player lives before: %d\n", game->player.lives);
				if (game->time.total > GOD_MODE_TIME) {
					game->player.lives--;
					printf("Player lives after: %d\n", game->player.lives);
				}
				SpawnPlayer(game, cars);
			}
			else {
				DrawSurface(sdl->screen, cars[i].car, cars[i].coord.x, cars[i].coord.y);
			}
		}
	}
}


void DrawRandomPower(CarInfo* cars, Game* game, SDL* sdl) {
	if (game->power.sprite == NULL && int(game->totalDistance * 1000) % 429 == 0) {
		printf("DrawRandomPower if\n");
		game->power.sprite = sdl->powerup[0];
		game->power.coord.y = -game->power.sprite->h / 2;
		int counter = 0;
		do {
			game->power.coord.x = rand() % ((int)game->roadWidth) + LEFT_BORDER;
		} while (++counter < 30 && !canSpawn(game, cars));
	}
	else if (game->power.sprite != NULL && game->player.power.sprite == NULL) {
		// ≈сли бонус заспавнилс€, но у игрока нет бонусов
		game->power.coord.y += game->time.delta * 250 * !game->pause;
		DrawSurface(sdl->screen, game->power.sprite, game->power.coord.x, game->power.coord.y);
		if (game->power.coord.y > SCREEN_HEIGHT + game->power.sprite->h / 2) {
			game->power.sprite = NULL;
			printf("Power sprite = NULL | 336\n");
			return;
		}
		if (inFault(game->power.coord.x, game->player.coord.x, game->player.sprite->w / 2 + game->power.sprite->w / 2) &&
			inFault(game->power.coord.y, game->player.coord.y, game->player.sprite->h / 2 + game->power.sprite->h / 2)) {
			game->player.power.sprite = game->power.sprite;
			game->player.power.time = 5;
			game->power.sprite = NULL;
			printf("Power sprite = NULL | 344\n");
		}
	}
	else if (game->player.power.time <= 0) {
		game->player.power.sprite = NULL;
	}
}


void NewGame(Game* game, CarInfo* cars, SDL* sdl) {
	SpawnPlayer(game, cars);
	game->player.sprite = sdl->playerCars[0];
	game->player.colorIndex = 0;
	game->player.lives = 1;
	game->player.liveGain = 0;
	game->player.power.sprite = NULL;
	game->player.power.time = 0;
	game->time.startGame = SDL_GetTicks();
	game->time.killMessage = 0;
	game->time.scoreFreeze = 0;
	game->time.deadMessage = 0;
	game->time.delta = 0;
	game->time.total = 0;
	game->power.sprite = NULL;
	game->score = 0;
	game->totalDistance = 0;
	game->bullet.speed = 0;
	game->bullet.coord.y = -200;
	game->bullet.coord.y2 = SCREEN_HEIGHT + 200;
	game->pause = false;
	game->roadWidth = SCREEN_WIDTH / 4;
};


void SaveGame(Game* game, CarInfo* cars, SDL* sdl) {
	FILE* out;
	time_t now = time(0);
	tm* time = localtime(&now);

	char* path = new char[30];
	strftime(path, 30, "saves/%d-%m-%Y_%H-%M-%S.dat", time);
	out = fopen(path, "wb");
	if (out == NULL) {
		// если уже была загрузка, курсор уже находитс€ в saves/
		strftime(path, 30, DATE_FORMAT, time);
		out = fopen(path, "wb");
		if (out == NULL) {
			printf("Error while saving game\n");
			return;
		}
	}
	Save save;
	save.game = *game;
	for (int i = 0; i < ENEMIES; i++) {
		save.cars[i] = cars[i];
		printf("save.cars[%d] = %p\n", i, &save.cars[i]);
	}
	if (game->player.power.sprite == NULL) save.game.player.power.sprite = sdl->powerup[0];
	if (game->bullet.sprite == NULL) save.game.bullet.sprite = sdl->bullet;
	if (game->power.sprite == NULL) save.game.power.sprite = sdl->powerup[0];
	for (int i = 0; i < ENEMIES; i++) {
		if (cars[i].car == NULL) save.cars[i].car = sdl->cars[0];
	}

	if (fwrite(&save, sizeof(Save), 1, out) != 1)
	{
		printf("FWRITE ERROR\n");
		return;
	}
	fclose(out);
}


void LoadGame(Game* game, CarInfo* cars, SDL* sdl, char filePath[250]) {
	FILE* in;

	Save save;
	in = fopen(filePath, "rb");
	if (in == NULL) {
		printf("Error while loading game\n");
		return;
	}
	for (int i = 0; i < 250; i++) {
		if (filePath[i] == '\0') {
			if (filePath[i - 4] != '.' || filePath[i - 3] != 'd' || filePath[i - 2] != 'a' || filePath[i - 1] != 't')
				return;
		}
	}
	fread(&save, sizeof(Save), 1, in);
	fclose(in);
	*game = save.game;
	for (int i = 0; i < ENEMIES; i++) {
		cars[i] = save.cars[i];
	}
	game->player.sprite = sdl->playerCars[game->player.colorIndex];
	game->player.power.sprite = sdl->powerup[0];
	game->power.sprite = sdl->powerup[0];
	for (int i = 0; i < ENEMIES; i++) {
		cars[i].car = sdl->cars[cars[i].colorIndex];
	}
}


void ShowSavedGames(Game* game, CarInfo* cars, SDL* sdl) {
	OPENFILENAMEA ofn;
	char szFile[250];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = ".dat";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = "E:/Studying/1 sem/PP/Project2";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn) == TRUE) {
		printf("Selected file: %s\n", szFile);
		//NewGame(game, cars, sdl);
		LoadGame(game, cars, sdl, szFile);
	}
	else {
		printf("No file selected.\n");
	}

}


void SpawnPlayer(Game* game, CarInfo* cars) {
	game->player.coord.x = SCREEN_WIDTH / 2;
	game->player.coord.y = SCREEN_HEIGHT * 2 / 3;
	game->player.speed = 0;
	game->player.turn = 0;
	for (int i = 0; i < ENEMIES; i++) {
		cars[i].coord.x = 0;
	}
};


void AddResult(Game* game, vector_t* resultsList, SDL* sdl) {
	char text[128];
	SDL_FillRect(sdl->screen, NULL, SDL_MapRGB(sdl->screen->format, 107, 142, 35));
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2,
		SDL_MapRGB(sdl->screen->format, 192, 192, 192), SDL_MapRGB(sdl->screen->format, 40, 40, 40));
	sprintf(text, "SPY HUNTER");
	DrawString(sdl->screen, SCREEN_WIDTH / 2 - strlen(text) * 4, SCREEN_HEIGHT / 4 + 30, text, sdl->charset);
	sprintf(text, "Add result to your list of best games?");
	DrawString(sdl->screen, SCREEN_WIDTH / 2 - strlen(text) * 4, SCREEN_HEIGHT / 4 + 60, text, sdl->charset);
	sprintf(text, "Y - Yes");
	DrawString(sdl->screen, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 4 + 80, text, sdl->charset);
	sprintf(text, "N - No");
	DrawString(sdl->screen, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 4 + 80, text, sdl->charset);
	RenderSurfaces(sdl);
	char action = '\0';
	do {
		while (SDL_PollEvent(&sdl->event)) {
			switch (sdl->event.type) {
			case SDL_KEYDOWN:
				if (sdl->event.key.keysym.sym == SDLK_y) action = 'Y';
				else if (sdl->event.key.keysym.sym == SDLK_n) action = 'N';
				break;
			case SDL_KEYUP:
				if (action == 'Y' || action == 'N') break;
			};
		}
	} while (action == '\0');
	if (action == 'N') return;
	Result result{ game->time.total, (int)game->score };
	push_back(resultsList, result);
}


void SaveResults(vector_t* resultsList) {
	FILE* out;
	out = fopen("saves/results.txt", "w");
	if (out == NULL) {
		out = fopen("results.txt", "w");
		if (out == NULL) {
			printf("Error while loading results\n");
			return;
		}
	}
	for (int i = 0; i < resultsList->count; i++) {
		printf("i= %d, time = %.3f, score = %d\n", i, resultsList->ptr[i].time, resultsList->ptr[i].score);
		fprintf(out, "%f %d\n", resultsList->ptr[i].time, resultsList->ptr[i].score);
	}
	fclose(out);
}


void LoadResults(vector_t* resultsList) {
	FILE* in = fopen("saves/results.txt", "r");
	if (in == NULL) {
		in = fopen("results.txt", "r");
		if (in == NULL) {
			printf("Error while loading results\n");
			return;
		}
	}
	Result result = { 0, 0 };
	while (true) {
		if (fscanf(in, "%lf %d\n", &result.time, &result.score) != 2) break;
		push_back(resultsList, result);
	}
	fclose(in);
}


int sortByScore(const void* a, const void* b) {
	struct Result x = *(struct Result*)a;
	struct Result y = *(struct Result*)b;
	if (x.score == y.score) return (y.time - x.time);
	return (y.score - x.score);

}


int sortByTime(const void* a, const void* b) {
	struct Result x = *(struct Result*)a;
	struct Result y = *(struct Result*)b;
	if (x.time == y.time) return (y.score - x.score);
	return 1000 * (y.time - x.time);
}


void topResultsMenu(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars) {
	// fill green color
	static int page = 0;
	char text[128];
	SDL_FillRect(sdl->screen, NULL, SDL_MapRGB(sdl->screen->format, 107, 142, 35));
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2,
		SDL_MapRGB(sdl->screen->format, 192, 192, 192), SDL_MapRGB(sdl->screen->format, 40, 40, 40));
	sprintf(text, "BEST GAMES LIST");
	DrawString(sdl->screen, SCREEN_WIDTH / 2 - strlen(text) * 4, SCREEN_HEIGHT / 4 + SPACING, text, sdl->charset);
	int i;
	for (i = page * RES_PER_PAGE; i < (page + 1) * RES_PER_PAGE && i < resultsList->count; i++) {
		sprintf(text, "%d. Score: %d", i + 1, resultsList->ptr[i].score);
		DrawString(sdl->screen, SCREEN_WIDTH / 3 + 50, SCREEN_HEIGHT / 4 + 2* SPACING + 20 * (i - page * RES_PER_PAGE), text, sdl->charset);
		sprintf(text, "Time: %.1f s", resultsList->ptr[i].time);
		DrawString(sdl->screen, SCREEN_WIDTH / 2 + 50, SCREEN_HEIGHT / 4 + 2* SPACING + 20 * (i - page * RES_PER_PAGE), text, sdl->charset);
	}
	sprintf(text, "\032/\033 - Previous/Next page");
	DrawString(sdl->screen, 2 * SCREEN_WIDTH / 3 - strlen(text) * 8 - SPACING, 3 * SCREEN_HEIGHT / 4 - SPACING, text, sdl->charset);
	sprintf(text, "B - Back");
	DrawString(sdl->screen, SCREEN_WIDTH / 3 + strlen(text) * 8 + SPACING, 3 * SCREEN_HEIGHT / 4 - SPACING, text, sdl->charset);
	RenderSurfaces(sdl);
	getResultsMenuAction(&page, sdl, resultsList, i, game, cars);
}


void welcomeMenu(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars, int* quit) {
	SDL_FillRect(sdl->screen, NULL, SDL_MapRGB(sdl->screen->format, 107, 142, 35));
	DrawRectangle(sdl->screen, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2,
		SDL_MapRGB(sdl->screen->format, 192, 192, 192), SDL_MapRGB(sdl->screen->format, 40, 40, 40));

	char text[][64] = {
		"SPY HUNTER",
		"N - New Game",
		"L - Load Game",
		"ESC - Exit"
	};
	for (int i = 0; i < 4; i++) {
		DrawString(sdl->screen, SCREEN_WIDTH / 2 - strlen(text[i]) * 4, SCREEN_HEIGHT / 4 + SPACING * (i+1), text[i], sdl->charset);
	}
	if (resultsList->count > 0) {
		sprintf(*text, "P - Best results by points");
		DrawString(sdl->screen, SCREEN_WIDTH / 3 + 100, SCREEN_HEIGHT / 4 + SPACING * 5, *text, sdl->charset);
		sprintf(*text, "T - Best results by time");
		DrawString(sdl->screen, SCREEN_WIDTH / 3 + 100, SCREEN_HEIGHT / 4 + SPACING * 6, *text, sdl->charset);
	}
	RenderSurfaces(sdl);
	getWelcomeMenuAction(sdl, resultsList, game, cars, quit);
}


void getWelcomeMenuAction(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars, int* quit) {
	char action = '\0';
	do {
		while (SDL_PollEvent(&sdl->event)) {
			switch (sdl->event.type) {
			case SDL_KEYDOWN:
				if (sdl->event.key.keysym.sym == SDLK_n) action = 'N';
				else if (sdl->event.key.keysym.sym == SDLK_l) action = 'L';
				else if (sdl->event.key.keysym.sym == SDLK_p && resultsList->count) action = 'P';
				else if (sdl->event.key.keysym.sym == SDLK_t && resultsList->count) action = 'T';
				else if (sdl->event.key.keysym.sym == SDLK_ESCAPE) action = 'Q';
				break;
			};
		}
	} while (action == '\0');

	switch (action) {
	case 'N':
		NewGame(game, cars, sdl);
		break;
	case 'L':
		ShowSavedGames(game, cars, sdl);
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
		(*quit)++;
		break;
	}
}


void getResultsMenuAction(int* page, SDL* sdl, vector_t* resultsList, const int place, Game* game, CarInfo* cars) {
	char action = '\0';
	do {
		while (SDL_PollEvent(&sdl->event)) {
			switch (sdl->event.type) {
			case SDL_KEYDOWN:
				if (sdl->event.key.keysym.sym == SDLK_LEFT && *page > 0) action = 'L';
				else if (sdl->event.key.keysym.sym == SDLK_RIGHT && place < resultsList->count) action = 'R';
				else if (sdl->event.key.keysym.sym == SDLK_b) action = 'B';
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


void changeTimers(Game* game) {
	game->time.total += game->time.delta;
	if (game->time.scoreFreeze > 0) game->time.scoreFreeze -= game->time.delta;
	if (game->time.scoreFreeze < 0) game->time.scoreFreeze = 0;
	if (game->time.killMessage > 0) game->time.killMessage -= game->time.delta;
	if (game->time.deadMessage > 0) game->time.deadMessage -= game->time.delta;
	if (game->player.power.time > 0) game->player.power.time -= game->time.delta;
	if ((int)game->time.total % 30 > 17 && (int)game->time.total < 20) game->roadWidth -= game->time.delta * 70;
	else if ((int)game->time.total % 30 > 2 && (int)game->time.total % 30 < 5) game->roadWidth += game->time.delta * 70;
	else if ((int)game->time.total % 30 > 10 && (int)game->time.total % 30 < 4) game->roadWidth += game->time.delta * 30;
	else if ((int)game->time.total % 30 > 25 && (int)game->time.total % 30 < 29) game->roadWidth -= game->time.delta * 30;
}


// check if X and Y coordinates are inside a screen
bool inArray(int x, int y) {
	if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
		return true;
	}
	return false;
}


// Check ifhorizontal position is out of screen and do it in the range.
bool onTheRoad(int* horizontal, Game* game) {
	if (*horizontal < LEFT_BORDER - OUT_ROAD) {
		*horizontal = LEFT_BORDER - OUT_ROAD;
	}
	else if (*horizontal > RIGHT_BORDER + OUT_ROAD) {
		*horizontal = RIGHT_BORDER + OUT_ROAD;
	}
	return (*horizontal < LEFT_BORDER || *horizontal > RIGHT_BORDER) ? false : true;
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


// x,y - координата на периметре машины; x2, y2 - верхн€€ лева€ точка объекта
bool numbersInArray(int x, int y, CarInfo* object) {
	int x2 = object->coord.x - object->car->w / 2;
	int y2 = object->coord.y - object->car->h / 2;
	// warn +1 -1
	if (x >= x2 && y >= y2 && x <= x2 + object->car->w && y <= y2 + object->car->h) {
		return true;
	}
	return false;
}


// ѕроверка дл€ пикселей по периметру машины, находитс€ ли позици€ в другом объекте.
bool touchObject(Game* game, CarInfo* object, const double deltaTime, CarInfo* cars, SDL* sdl) {
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
				if ((object->coord.x < LEFT_BORDER - object->car->w ||
					object->coord.x > RIGHT_BORDER + object->car->w)
					&& object->colorIndex != ENEMIES)
				{
					// Car is destroyed
					object->car = sdl->cars[ENEMIES];
					object->colorIndex = ENEMIES;
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
				// ѕроехать нельз€ из-за преграды в виде авто, значит игрок сдвигаетс€ обратно
				game->player.coord.x -= game->player.turn * 5; // WARN !!!
				// ≈сли могу подвинуть авто, скорость - 200, иначе возвращаюсь на позицию назад где скорость 300
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


bool isDestroyed(CarInfo* car, SDL* sdl, Game* game) {
	if (car->car == sdl->cars[ENEMIES]
		|| car->coord.x < LEFT_BORDER - car->car->w || car->coord.x > RIGHT_BORDER + car->car->w) {
		return true;
	}
	return false;
}


bool isFreePlace(CarInfo* car, CarInfo* cars, int turn) {
	// turn = -1 : упираетс€ справа, 1 : упираетс€ слева
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
		// ≈сли попал на свою же машину
		if (cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x || cars[i].coord.x == 0) continue;
		// провер€ю нижнюю левую и нижнюю правую точки зареспавненной машины
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
	// јтакует если в одной полосе хот€ бы половина машины
	if (inFault(game->player.coord.x, car->coord.x, 31)) {
		// CHANGE | SCREEN_HEIGHT/2
		if (inFault(game->player.coord.y, car->coord.y, SCREEN_HEIGHT) && game->player.coord.y - car->coord.y > car->car->h + 30) {
			// ѕри атаке сверху тормозит за 30 пикселей от мен€
			if (canGo(car, cars, 1)) return 1.8;
		}
		else if (inFault(game->player.coord.y, car->coord.y, SCREEN_HEIGHT) && game->player.coord.y - car->coord.y < -car->car->h - 10) {
			// ѕри атаке тормозит за 10 пикселей от мен€ CHANGE
			if (canGo(car, cars, -1)) return -0.6;
		}
	}
	return 0;
}


// ѕроверка дл€ атакующей машины может ли она ехать
bool canRide(CarInfo* car, CarInfo* cars) {
	// turn = -1 : упираетс€ сверху, 1 : упираетс€ снизу
	int y = car->coord.y;
	for (int i = 0; i < ENEMIES; i++) {
		// ≈сли попал на свою же машину
		if (cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x || cars[i].coord.x == 0) continue;
		// провер€ю нвходитс€ ли в радиусе другое авто
		// WARN -2 -> -1 |  огда машины трутьс€, чтобы могли идти в атаку
		// WARN +2 |  огда игра атакуют 2 машины в р€д, чтобы они не терлись
		if (inFault(cars[i].coord.y, y, car->car->h + 2) && inFault(cars[i].coord.x, car->coord.x, car->car->w - 2))
		{
			return false;
		}
	}
	return true;
}


// проверка с 4 сторон есть ли там машины
bool freeSpace(CarInfo* car, CarInfo* cars) {
	// turn = -1 : упираетс€ сверху, 1 : упираетс€ снизу
	int x = car->coord.x;
	int y = car->coord.y;
	for (int i = 0; i < ENEMIES; i++) {
		// ≈сли попал на свою же машину
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
		// ≈сли попал на свою же машину
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


bool canSpawn(Game* game, CarInfo* cars) {
	int deltaY;
	for (int i = 0; i < ENEMIES; i++) {
		if (cars[i].coord.x == 0) continue;
		deltaY = cars[i].coord.y - game->power.coord.y;
		if (deltaY > 0 && deltaY < cars[i].car->h / 2 + game->power.sprite->h / 2 + 10) {
			return false;
		}
	}
	return true;
}


void addBullet(Game* game, SDL* sdl) {
	if (game->bullet.speed != 0) return;
	game->bullet.sprite = sdl->bullet;
	game->bullet.coord.x = game->player.coord.x;
	game->bullet.coord.y = game->player.coord.y - 30;
	if (game->player.power.time > 0) game->bullet.coord.y2 = game->player.coord.y + 30;
	else game->bullet.coord.y2 = SCREEN_HEIGHT + 200;
	game->bullet.speed = 500;
}


int carIsKilled(Game* game, CarInfo* cars, SDL* sdl, int y) {
	// return 0 - no one is killed, -1 - citizen is killed, 1 - enemy is killed
	for (int i = 0; i < ENEMIES; i++) {
		bool isEnemy = cars[i].isEnemy;
		if (cars[i].coord.x != 0) {
			if ((inFault(y, cars[i].coord.y, cars[i].car->h / 2) &&
				inFault(game->bullet.coord.x, cars[i].coord.x, cars[i].car->w / 2))
				&& cars[i].colorIndex != ENEMIES) {
				// Car is killed
				cars[i].car = sdl->cars[ENEMIES];
				cars[i].colorIndex = ENEMIES;
				cars[i].isEnemy = false;
				// game->bullet.speed = 0;
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