// world.h

#ifndef WORLD_H
#define WORLD_H

#include <sys/timeb.h>

#include "headers.h"
#include "main.h"
#include "seq.h"
#include "centipede.h"
#include "mushroom.h"
#include "player.h"
#include "dart.h"
#include "worldDefs.h"

#include "flea.h"

class World {
	GLFWwindow* window;

	int  score;
	int  numCols;

	bool playerDied;
	bool goToNextLevel;

	bool  pauseForMessage;
	timeb pauseUntil;

	seq<Centipede*> centipedes;
	seq<Mushroom*>  mushrooms;
	seq<Flea*> fleas;
	Player* player;
	seq<Dart*>      darts;

public:

	int  level;
	int  livesRemaining;
	bool gameOver;

	float l, r, b, t;  // coordinates of window edges

	World(GLFWwindow* w) {
		initWorld(w);
	}

	void initWorld(GLFWwindow* w);

	void initLevel() {
		// Start with one centipede.  With each new level, make the
		// centipede one segment shorter AND create a length-one centipede

		centipedes.clear();
		fleas.clear();

		centipedes.add(new Centipede(MAX_CENTIPEDE_SEGMENTS - level, INIT_CENTIPEDE_POS, INIT_CENTIPEDE_DIR));
		for (int i = 0; i < level; i++)
			centipedes.add(new Centipede(1,
				vec2(WORLD_LEFT_EDGE + (randIn01() * (numCols - 1) + 0.5) * COL_SPACING, INIT_CENTIPEDE_POS.y),
				vec2(randIn01() > 0.5 ? 1 : -1, INIT_CENTIPEDE_DIR.y)));
		// One player

		if (!player)
			player = new Player(INIT_PLAYER_POS);
		else
			player->pos = INIT_PLAYER_POS;

		playerDied = false;
		goToNextLevel = false;
		pauseForMessage = false;

		// Set up the window

		setWindowEdgeCoordinates();
	}

	/// <summary>
	/// Convert mouse coordinates to world coordinates, which should permit you to move the player.
	/// </summary>
	/// <param name="pos">Pass in mouse position</param>
	void playerMove(vec2 pos) {
		int winX, winY;
		glfwGetWindowSize(window, &winX, &winY);

		// [YOUR CODE HERE]
		//
		// Convert the mouse position 'pos' into a world position.  Use
		// the window size 'winX' and 'winY' and use the world coordinates
		// of the window edges (defined above).

		//float worldX = 0.02;
		//float worldY = -0.9;
		float worldX = (2 * pos.x / winX) - 1;
		float worldY = (-2 * pos.y / winY) + 1;

		player->moveTo(vec2(worldX, worldY));
	}

	void playerFire() {
		if (darts.size() < MAX_DARTS_AT_ONCE)
			darts.add(new Dart(player->pos));
	}

	int getScore() {
		return score;
	}

	void draw();
	void updateState(float elapsedTime);
	void setWindowEdgeCoordinates();
	Mushroom* findClosestMushroomAhead(vec2 pos, vec2 dir);
};

#endif
