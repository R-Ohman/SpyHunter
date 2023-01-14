#define _CRT_SECURE_NO_WARNINGS
#include "SpyHunter.h"



int DrawPlayer(Game* game, SDL* sdl) {
	if (game->player.lives < 1) {
		printf("GAME END\n");
		return 1;
	}
	if (game->player.liveGain < (int)game->score / 5000) {
		game->player.lives++;
		game->player.liveGain++;
	}
	if (game->time.total > GOD_MODE_TIME) {
		for (int i = 0; i < 5; i++) {
			if (game->totalDistance > 100 * (i + 1)) {
				game->player.sprite = sdl->playerCars[i];
				game->player.colorIndex = i;
			}
		}
	}
	DrawSurface(sdl->screen, game->player.sprite, game->player.coord.x, game->player.coord.y);
	return 0;
}


// Returns "true" if one of the bullets is in the visible area of the screen
bool bulletExists(Game* game) {
	return (game->bullet.coord.y > -game->bullet.sprite->h || game->bullet.coord.y2 != 0 && game->bullet.coord.y2 < SCREEN_HEIGHT + game->bullet.sprite->h) ? true : false;
}


void DrawBullet(CarInfo* cars, Game* game, SDL* sdl) {
	if (game->bullet.coord.x == 0) return;
	if (bulletExists(game)) {
		int result[2] = { 0 };
		/* If the bullet flying up is in the visible area of the screen, move it.
		* Draw bullet. 'result': 1 - enemy is killed; -1 - citizen is kille; 0 - no one is killed
		*/
		if (game->bullet.coord.y > -game->bullet.sprite->h) {
			game->bullet.coord.y -= game->time.delta * game->bullet.speed;
			DrawSurface(sdl->screen, game->bullet.sprite, game->bullet.coord.x, game->bullet.coord.y);
			result[0] = carIsKilled(game, cars, sdl, game->bullet.coord.y);
		}
		if (game->bullet.coord.y2) {
			game->bullet.coord.y2 += game->time.delta * game->bullet.speed * 2;
			DrawSurface(sdl->screen, game->bullet.sprite, game->bullet.coord.x, game->bullet.coord.y2);
			result[1] = carIsKilled(game, cars, sdl, game->bullet.coord.y2);
		}
		// If the bullet killed another car, take it out of the screen
		for (int i = 0; i <= 1; i++) {
			if (result[i]) {
				if (i == 0) game->bullet.coord.x = -200;
				else game->bullet.coord.y2 = SCREEN_HEIGHT + 200;
				switch (result[i]) {
				case -1:
					game->time.scoreFreeze += 3;
					break;
				case 1:
					game->score += 1000;
					break;
				}
			}
		}
	}
	else {
		// If the bullets are out of the screen, stop moving it
		game->bullet.speed = 0;
		game->bullet.coord.x = 0;
	}
}


/*If the car is not spawned (x coordinate = 0), give it a random skin
	and a random position on the road (on the x axis)	*/
void spawnRandomCar(CarInfo* cars, Game* game, SDL* sdl) {
	if (int(game->totalDistance * 1000) % 181 == 0) {
		for (int i = 0; i < ENEMIES; i++) {
			if (cars[i].coord.x == 0) {
				int car_num = rand() % 5;
				cars[i].car = sdl->cars[car_num];
				cars[i].colorIndex = car_num;
				// If car is blue ir lilac, it's citizen
				if (car_num == 0 || car_num == 3)
					cars[i].isEnemy = false;
				else
					cars[i].isEnemy = true;
				// Spawn the car above the screen
				cars[i].coord.y = -sdl->cars[car_num]->h / 2;
				// Над экраном на высоту сгенерированного авто
				int iterCounter = 0;
				do {
					cars[i].coord.x = rand() % ((int)game->roadWidth) + LEFT_BORDER;
					if (++iterCounter > 30) {
						// Do not spawn a car if there is no free space
						cars[i].coord.x = 0;
						return;
					}
				} while (!canGo(&cars[i], cars, 1)); // Generates the X position until there is an empty space
				break;
			}
			else if (cars[i].coord.y > SCREEN_HEIGHT + sdl->cars[0]->h / 2) {
				// If the car went off the screen, remove it untill it will be spawned again
				cars[i].coord.x = 0;
			}
		}
	}
}

// Moves cars along the Y axis
void moveRandomCar(CarInfo* cars, Game* game, SDL* sdl) {
	for (int i = 0; i < ENEMIES; i++) {
		if (cars[i].coord.x != 0) {
			/* If a vehicle is destroyed, its downward speed increases.
				If there is a pause, the cars do not move.*/
			cars[i].speed = (isDestroyed(&cars[i]) ? 2 : 1) * (!game->pause) * CAR_SPEED;
			int attackDirection = canAttack(&cars[i], game, cars);
			// 1 if attacking down, -1 if attacking up,0 if not attacking
			if (cars[i].isEnemy && attackDirection != 0) {
				double coef = attackDirection;
				if (attackDirection > 0)
					// Goes down faster if the player's speed is increased by a power-up
					coef *= game->player.powerTime[1] > 0 ? 2.5 : 1.5;
				else
					// Goes up more slowly if the player's speed is increased by a power-up
					coef *= game->player.powerTime[1] > 0 ? 0.1 : 0.5;
				cars[i].coord.y += coef * cars[i].speed * game->time.delta;
			}
			else {
				// Goes down faster if the player's speed is increased by a power-up
				double coef = cars[i].speed * (game->player.powerTime[1] > 0 ? 2 : 1);
				cars[i].coord.y += game->time.delta * coef;
			}
		}
	}
}


void DrawRandomCar(CarInfo* cars, Game* game, SDL* sdl) {
	moveRandomCar(cars, game, sdl);
	spawnRandomCar(cars, game, sdl);

	for (int i = 0; i < ENEMIES; i++) {
		if (cars[i].coord.x != 0) {
			// Checks if the given player's car is pushing, if not, draws the car
			if (touchObject(game, &cars[i], game->time.delta, cars, sdl)) {
				if (game->time.total > GOD_MODE_TIME) {
					game->player.lives--;
				}
				SpawnPlayer(game, cars);
			}
			else {
				DrawSurface(sdl->screen, cars[i].car, cars[i].coord.x, cars[i].coord.y);
			}
		}
	}
}


// Returns true if the power-up is not spawned and the player does not have this power-up
bool powerNotSpawned(Game* game, int index) {
	return (game->powerCoord[index].x == 0 && game->player.powerTime[index] == 0) ? true : false;
}


// Spawns a power-up if the player doesn't have it
void spawnRandomPower(CarInfo* cars, Game* game, SDL* sdl) {
	if ((powerNotSpawned(game, 0) || powerNotSpawned(game, 1)) && int(game->totalDistance * 1000) % 429 == 0) {
		int powerSpawnIndex;
		// If one power up is not spawned, choose randomly, otherwise choose the one that does not exist
		if (powerNotSpawned(game, 0) && powerNotSpawned(game, 1))
			powerSpawnIndex = rand() % 2;
		else if (powerNotSpawned(game, 0))
			powerSpawnIndex = 0;
		else
			powerSpawnIndex = 1;
		// P_ut it above the screen and choose a random position on the x-axis
		game->powerCoord[powerSpawnIndex].y = -sdl->powerup[powerSpawnIndex]->h / 2;
		int counter = 0;
		do {
			game->powerCoord[powerSpawnIndex].x = rand() % ((int)game->roadWidth) + LEFT_BORDER;
			if (++counter > 30) {
				// If there is no free space, do not spawn it
				game->powerCoord[powerSpawnIndex].x = 0;
				return;
			}
		} while (!canSpawn(game, cars, sdl, powerSpawnIndex));
	}
}


void DrawRandomPower(CarInfo* cars, Game* game, SDL* sdl) {
	spawnRandomPower(cars, game, sdl);
	for (int i = 0; i <= 1; i++) {
		// If the power-up is spawned, but the player does not have it
		if (game->powerCoord[i].x != 0 && game->player.powerTime[i] == 0) {
			game->powerCoord[i].y += game->time.delta * CAR_SPEED * !game->pause * (game->player.powerTime[1] > 0 ? 2 : 1);
			DrawSurface(sdl->screen, sdl->powerup[i], game->powerCoord[i].x, game->powerCoord[i].y);
			if (game->powerCoord[i].y > SCREEN_HEIGHT + sdl->powerup[i]->h / 2) {
				// If the power-up went to the bottom of the screen
				game->powerCoord[i].x = 0;
				return;
			}
			/* A power-up touches a player if the y-distance between the player
			and the car = half their height and the x-distance = half their width*/
			if (inFault(game->powerCoord[i].x, game->player.coord.x, game->player.sprite->w / 2 + sdl->powerup[i]->w / 2) &&
				inFault(game->powerCoord[i].y, game->player.coord.y, game->player.sprite->h / 2 + sdl->powerup[i]->h / 2)) {
				game->player.powerTime[i] = POWER_TIME;
				game->powerCoord[i].x = 0;
			}
		}
	}
}


bool onTheRoad(double* horizontal, Game* game) {
	if (*horizontal < LEFT_BORDER - OUT_ROAD) {
		*horizontal = LEFT_BORDER - OUT_ROAD;
	}
	else if (*horizontal > RIGHT_BORDER + OUT_ROAD) {
		*horizontal = RIGHT_BORDER + OUT_ROAD;
	}
	return (*horizontal < LEFT_BORDER || *horizontal > RIGHT_BORDER) ? false : true;
}


void fixCoordY(double* vertical) {
	if (*vertical < 90) {
		*vertical = 90;
	}
	else if (*vertical > SCREEN_HEIGHT) {
		*vertical = SCREEN_HEIGHT;
	}
}


bool numbersInArray(int x, int y, CarInfo* car) {
	int leftPoint = car->coord.x - car->car->w / 2;
	int topPoint = car->coord.y - car->car->h / 2;
	return (x >= leftPoint && y >= topPoint && x <= leftPoint + car->car->w && y <= topPoint + car->car->h) ? true : false;
}


double modul(int a, int b) {
	return (a - b > 0) ? a - b : b - a;
}


bool touchObject(Game* game, CarInfo* object, const double deltaTime, CarInfo* cars, SDL* sdl) {
	// If the player touches the car with the left or right side
	if (modul(game->player.coord.x, object->coord.x) == object->car->w &&
		inFault(game->player.coord.y, object->coord.y, object->car->h)) {
		if (isFreePlace(object, cars, game->player.turn)) {
			// If the player is pushing a car and there is nothing in the direction, move the car the same distance as the player is moving
			object->coord.x += game->player.turn * deltaTime * CAR_SPEED * (game->player.powerTime[1] > 0 ? 1.8 : 1.2);
			// If the car is pushed out of the way, change the skin
			if ((object->coord.x < LEFT_BORDER - object->car->w ||
				object->coord.x > RIGHT_BORDER + object->car->w)
				&& object->colorIndex != ENEMIES)
			{
				object->car = sdl->cars[ENEMIES];
				object->colorIndex = ENEMIES;
				if (object->isEnemy) {
					if (!game->time.scoreFreeze) {
						game->score += 1000;
						game->time.killMessage = 2;
						object->isEnemy = false;
					}
				}
				else {
					game->time.scoreFreeze += 3;
				}
			}
		}
		else {
			// There is already a car in the direction, so player can’t push the current one
			game->player.coord.x -= game->player.turn * deltaTime * CAR_SPEED * (game->player.powerTime[1] > 0 ? 1.8 : 1.2);

		}
		return false;
	}
	// If the player touches the car from above or below
	if (modul(game->player.coord.y, object->coord.y) == object->car->h &&
		inFault(game->player.coord.x, object->coord.x, object->car->w))
		return true;
	
	return false;
}


bool isDestroyed(CarInfo* car) {
	return car->colorIndex == ENEMIES ? true : false;
}


bool isFreePlace(CarInfo* car, CarInfo* cars, int turn) {
	for (int i = 0; i < ENEMIES; i++) {
		// If I check the same car or got on an unspawned car
		if (cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x || cars[i].coord.x == 0) continue;
		// If the car touches a vehicle with the left/right side
		if (turn * (cars[i].coord.x - car->coord.x) == car->car->w &&
			inFault(car->coord.y, cars[i].coord.y, car->car->h))
		{	
			car->coord.x -= turn * 2; // WARN auto bounces a couple of pixels back
			return false;
		}
	}
	return true;
}


double canAttack(CarInfo* car, Game* game, CarInfo* cars) {
	// Attacks if at least half of the car is in one lane
	if (inFault(game->player.coord.x, car->coord.x, car->car->w/2)) {
		// Attacks only if the distance is not more than 2/3 of the screen
		if (inFault(game->player.coord.y, car->coord.y, 2 * SCREEN_HEIGHT / 3)) {
			if (canGo(car, cars, 1)) return 1;
			if (canGo(car, cars, -1)) return -1;
		}
	}
	return 0;
}


bool inFault(int num1, int num2, int fault) {
	return (modul(num1, num2) < fault) ? true : false;
}


bool canGo(CarInfo* car, CarInfo* cars, int direction) {
	// direction: -1 -> top, 1 -> bottom
	for (int i = 0; i < ENEMIES; i++) {
		// If I check the same car or got on an unspawned car
		if (cars[i].coord.y == car->coord.y && cars[i].coord.x == car->coord.x || cars[i].coord.x == 0) continue;
		// If the distance from the car to another car is less than 10 pixels
		if (direction * (cars[i].coord.y - car->coord.y) <= car->car->h + 10 &&
			inFault(car->coord.x, cars[i].coord.x, car->car->w))
			return false;
	}
	return true;
}


bool canSpawn(Game* game, CarInfo* cars, SDL* sdl, const int powerIndex) {
	for (int i = 0; i < ENEMIES; i++) {
		// Skip unspawned cars
		if (cars[i].coord.x == 0) continue;
		int height = sdl->powerup[powerIndex]->h / 2 + cars[i].car->h / 2;
		int width = sdl->powerup[powerIndex]->w / 2 + cars[i].car->w / 2;
		// If the turn-up touches another car (nearly)
		if (cars[i].coord.y - game->powerCoord[powerIndex].y <= height + 5 &&
			inFault(game->powerCoord[powerIndex].x, cars[i].coord.x, width + 5))
			return false;
	}
	return true;
}


int carIsKilled(Game* game, CarInfo* cars, SDL* sdl, int y) {
	// Return 0 - no one is killed, -1 - citizen is killed, 1 - enemy is killed
	for (int i = 0; i < ENEMIES; i++) {
		bool isEnemy = cars[i].isEnemy;
		if (cars[i].coord.x != 0) {
			// If a bullet touches an undestroyed car
			if ((inFault(y, cars[i].coord.y, cars[i].car->h / 2) &&
				inFault(game->bullet.coord.x, cars[i].coord.x, cars[i].car->w / 2))
				&& cars[i].colorIndex != ENEMIES) {
				cars[i].car = sdl->cars[ENEMIES];
				cars[i].colorIndex = ENEMIES;
				cars[i].isEnemy = false;
				return isEnemy ? 1 : -1;
			}
		}
	}
	return 0;
}