// world.cpp

#include "world.h"
#include "gpuProgram.h"
#include "main.h"
#include "strokefont.h"

#include <sstream>
#include <iomanip>

extern Mushroom* closestMush;

#define TEXT_SIZE 0.06  // as a fraction of centre-to-top distance

// Initialize the world state.  This is called before each new level.

void World::initWorld(GLFWwindow* w)

{
	window = w;
	score = 0;
	level = 0;
	gameOver = false;

	player = new Player(vec2(0, 0));
	livesRemaining = INIT_LIVES_REMAINING;

	// Random mushrooms

	srand(1574);

	numCols = (WORLD_RIGHT_EDGE - WORLD_LEFT_EDGE) / COL_SPACING - 1;

	mushrooms.clear();

	for (int i = 0; i < INIT_NUM_MUSHROOMS; i++) {
		// Generate random row/col

		int r = floor(randIn01() * NUM_ROWS);
		if (r == NUM_ROWS)
			r = NUM_ROWS - 1;

		int c = floor(randIn01() * numCols);
		if (c == numCols)
			c = numCols - 1;

		// Convert to world coordinates

		vec2 worldPos(WORLD_LEFT_EDGE + (c + 1) * COL_SPACING, WORLD_TOP_ROW - r * ROW_SPACING);

		// Check that it doesn't already exist

		bool exists = false;
		for (int j = 0; j < mushrooms.size(); j++)
			if (mushrooms[j]->pos == worldPos) {
				exists = true;
				break;
			}

		if (!exists) {
			//cout << "Pos:" << worldPos << endl;
			mushrooms.add(new Mushroom(worldPos));
		}
	}

	initLevel();
}

// Update the state of the world after 'elapsedTime' seconds have passed

void World::updateState(float elapsedTime)

{
	//cout << "time:" << elapsedTime << endl;
	// Don't do anything if we're pausing while a message is being displayed

	if (pauseForMessage || gameOver)
		return;

	// Move centipedes.

	for (int i = 0; i < centipedes.size(); i++)
		centipedes[i]->updatePose(elapsedTime);

	//For Flea Generation
	if (fleas.size() == 0) {
		int randCol = min(int(randIn01() * numCols) + 1, numCols - 1);
		float xPos = WORLD_LEFT_EDGE + randCol * COL_SPACING;
		int counts = 0;
		for (int j = 0; j < mushrooms.size(); j++)
		{
			// If the this mushroom is is in this row and lower than bottom fifth of the board, take into account
			if (mushrooms[j]->pos.x == xPos && mushrooms[j]->pos.y < ((WORLD_TOP_ROW - WORLD_BOTTOM_ROW) * 0.2 + WORLD_TOP_ROW)) {
				counts++;
			}
		}
		//cout << randCol << endl;
		if (counts <= FLEA_MUSHGEN_MINHATE)
			fleas.add(new Flea(vec2(WORLD_LEFT_EDGE + randCol * COL_SPACING, WORLD_TOP_ROW)));
	}

	//For Flea position and mushroom Update
	for (int i = 0; i < fleas.size(); i++)
	{
		//Update positions
		fleas[i]->updatePose(elapsedTime);

		//Laying mushroom
		float seed = randIn01() * FLEA_MUSHGEN_STEP;
		if (seed < FLEA_MUSHGEN_RATE && mushrooms.size() <= MUSH_MAX_NUM)
		{
			for (int j = 0; j < mushrooms.size(); j++)
			{
				if (mushrooms[j]->pos != fleas[i]->pos)
				{
					mushrooms.add(new Mushroom(fleas[i]->pos));
					break;
				}
			}
		}

		//destory it if it over the screen
		if (fleas[i]->pos.y < -1.0) {
			fleas.remove(i);
			i--;
			continue;
		}
	}

	// Move dart and check for it hitting something.  (Do this here as
	// it requires operating on World components.)

	for (int i = 0; i < darts.size(); i++) {
		// Move it

		float distanceTravelled = DART_SPEED * elapsedTime * speedMultiplier;

		darts[i]->pos = darts[i]->pos + vec2(0, distanceTravelled);

		// Check for dart going off the top

		if (darts[i]->pos.y > WORLD_TOP_ROW) {
			darts.remove(i);
			i--;
			continue;
		}

		//See if Fleas along the dart's path
		if (fleas.size() != 0)
		{
			bool isSkip = FALSE;
			for (int f = 0; f < fleas.size(); f++)
			{
				if ((fleas[f]->pos - darts[i]->pos).length() < distanceTravelled)
				{
					fleas[f]->damage += 1;
					if (fleas[f]->damage >= FLEA_MAX_DAMAGE) {
						score += SCORE_DESTROY_FLEA;
						fleas.remove(f);
					}

					darts.remove(i);
					i--;
					isSkip = TRUE;
				}
			}
			if (isSkip)
				continue;
		}

		// See if there's a mushroom along the dart's path

		Mushroom* closestMush = findClosestMushroomAhead(darts[i]->pos, vec2(0, 1));

		if (closestMush) {
			// See if this mushroom will be hit within the next time step
			// (approximated using the distance travelled in the last time
			// step)

			if ((closestMush->pos - darts[i]->pos).length() < distanceTravelled) {
				//closestMush->damage += 1;
				closestMush->damage += 1;

				if (closestMush->damage >= MUSH_MAX_DAMAGE) { // mushroom is destroyed
					score += SCORE_DESTROY_MUSHROOM;
					mushrooms.remove(mushrooms.findIndex(closestMush));
				}

				darts.remove(i);
				i--;
				continue;
			}
		}

		// See if a centipede segment is hit

		float     closestSegDist = MAXFLOAT;
		Centipede* closestCent;
		int       closestSegIndex;

		for (int j = 0; j < centipedes.size(); j++) {
			Centipede* cent = centipedes[j];
			for (int k = 0; k < cent->segments.size(); k++) {
				Segment* seg = cent->segments[k];

				// Test segment/dart here
				vec2 diff = seg->pos - darts[i]->pos;
				float distAlongLine = diff * seg->dir;  // [YOUR CODE HERE]
				float distPerpToLine = sqrt(diff.squaredLength() - pow(distAlongLine, 2));  // [YOUR CODE HERE]

				if (distAlongLine > 0 && distPerpToLine < SEG_BODY_RADIUS && distAlongLine < closestSegDist) {
					closestSegDist = distAlongLine;
					closestCent = cent;
					closestSegIndex = k;
				}
			}
		}

		if (closestSegDist < distanceTravelled) {
			// Turn the hit segment into a mushroom (i.e. place a new
			// mushroom at that position).  Place the new mushroom on one of
			// the row/column points.

			vec2 pos = closestCent->segments[closestSegIndex]->pos;;;;

			int col = rint((pos.x - WORLD_LEFT_EDGE) / COL_SPACING - 1);
			int row = rint((WORLD_TOP_ROW - pos.y) / ROW_SPACING);

			vec2 worldPos(WORLD_LEFT_EDGE + (col + 1) * COL_SPACING, WORLD_TOP_ROW - row * ROW_SPACING);  // (same code as in World constructor)

			mushrooms.add(new Mushroom(worldPos));

			// a hit: Remove segment that was hit and split centipede into two.

			int tailCentSize = closestCent->segments.size() - 1 - closestSegIndex; // num of segs in (new) tail centipede

			if (tailCentSize > 0) { // The last segment wasn't hit, so create a centipede of the tail segments
				Centipede* newCent = new Centipede(tailCentSize, vec2(0, 0), vec2(0, 0));

				for (int j = 0; j < tailCentSize; j++)
					newCent->segments[j] = closestCent->segments[j + closestSegIndex + 1];

				centipedes.add(newCent);

				// turn the new segment (only if not already turning

				Segment* seg0 = newCent->segments[0];

				if (!seg0->turning) {
					seg0->turning = true; // start turning
					seg0->turnAngle = 0;
					seg0->turnCentre = vec2(seg0->pos.x, seg0->pos.y - CENTIPEDE_TURN_RADIUS);
					seg0->dirUponTurnEntry = seg0->dir.x;
					seg0->turningPositionX = seg0->pos.x;
				}
			}

			if (closestSegIndex == 0) // The first segment was hit, so just remove this centipede

				centipedes.remove(centipedes.findIndex(closestCent));

			else // First segment not hit, so just truncate this centipede where it was hit

				for (int j = 0; j < tailCentSize + 1; j++)
					closestCent->segments.remove(); // removes the last segment

					// Update score

			if (closestSegIndex)
				score += SCORE_CENTIPEDE_HEAD;
			else
				score += SCORE_CENTIPEDE_SEGMENT;
		}
	}

	// See if a centipede's HEAD eats the player

	for (int i = 0; i < centipedes.size(); i++)
		if ((centipedes[i]->segments[0]->pos - player->pos).length() < 0.75 * ROW_SPACING) {
			struct timeb t;
			ftime(&t);
			t.time += PAUSE_TIME_FOR_MESSAGE;

			playerDied = true;
			pauseForMessage = true;
			pauseUntil = t;

			break;
		}

	// See if Flea hit player
	for (int i = 0; i < fleas.size(); i++)
	{
		if ((fleas[i]->pos - player->pos).length() < 0.75 * ROW_SPACING) {
			struct timeb t;
			ftime(&t);
			t.time += PAUSE_TIME_FOR_MESSAGE;

			playerDied = true;
			pauseForMessage = true;
			pauseUntil = t;

			break;
		}
	}

	// Remove a life if player was destroyed

	if (playerDied) {
		livesRemaining--;

		if (livesRemaining < 1) {
			gameOver = true;
			pauseForMessage = false;
		}
	}
	else if (!gameOver && centipedes.size() == 0) { // end of level
   // Add points for any remaining mushrooms.  Restore damaged mushrooms.

		for (int i = 0; i < mushrooms.size(); i++)
			mushrooms[i]->damage = 0;

		score += mushrooms.size() * SCORE_REMAINING_MUSHROOM;

		// Start next level if game isn't yet over

		if (level < MAX_LEVEL)
			level++;

		struct timeb t;
		ftime(&t);
		t.time += PAUSE_TIME_FOR_MESSAGE;

		goToNextLevel = true;
		pauseForMessage = true;
		pauseUntil = t;
	}
}

// Consider only the mushrooms that are within ROW_SPACING/4 of the
// line starting at position 'pos' in direction 'dir'.  Of those,
// return the distance to the closest one.

Mushroom* World::findClosestMushroomAhead(vec2 pos, vec2 dir)

{
	float minDist = MAXFLOAT;
	Mushroom* minMushroom;

	dir = dir.normalize();

	for (int i = 0; i < mushrooms.size(); i++) {
		vec2& mushPos = mushrooms[i]->pos;

		// Test mushroom/ray here
		vec2 mushDir = mushPos - pos;

		float distAlongLine = mushDir * dir; // [YOUR CODE HERE]

		float distPerpToLine = sqrt(mushDir.squaredLength() - pow(distAlongLine, 2)); // [YOUR CODE HERE]

		//float distAlongLine  = -1; // [YOUR CODE HERE]
		//float distPerpToLine = -1; // [YOUR CODE HERE]

		if (distAlongLine > 0 && distPerpToLine < ROW_SPACING / 4) {
			if (distAlongLine < minDist) {
				minDist = distAlongLine;
				minMushroom = mushrooms[i];
			}
		}
	}

	if (minDist != MAXFLOAT)
		return minMushroom;
	else
		return NULL;
}

// Draw the whole world, including its inhabitants.

void World::draw()

{
	glClearColor(BACKGROUND_COLOUR.x, BACKGROUND_COLOUR.y, BACKGROUND_COLOUR.z, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	setWindowEdgeCoordinates();

	mat4 VP = ortho(l, r, b, t, 0, 1);

	// Draw everything

	gpuProg->activate();

#ifndef MACOS
	glLineWidth(4.0); // used for wider legs and outlines, which are rendered as lines
#endif

	for (int i = 0; i < mushrooms.size(); i++)
		mushrooms[i]->draw(VP);

	for (int i = 0; i < centipedes.size(); i++)
		centipedes[i]->draw(VP);

	for (int i = 0; i < fleas.size(); i++)
		fleas[i]->draw(VP);

	if (player)
		player->draw(VP);

	for (int i = 0; i < darts.size(); i++)
		darts[i]->draw(VP);

#ifndef MACOS
	glLineWidth(1.0);
#endif

	// Show lives remaining in upper-left corner

	for (int i = 0; i < livesRemaining; i++) {
		mat4 T = translate(WORLD_LEFT_EDGE + 1 * COL_SPACING + i * 0.7 * COL_SPACING - player->pos.x, TOP_TEXT_Y - player->pos.y + TEXT_SIZE / 2.0, 0);
		mat4 VP2 = VP * T;
		player->draw(VP2);
	}

	gpuProg->deactivate();

	// Draw status at top

	fontGPUProg->activate();

	if (!gameOver) { // game is still running
		{ // Draw score in middle
			stringstream ss;
			ss << "Score " << score;
			string str = ss.str();

			drawStrokeString(str,
				-((str.length() - 1) * TEXT_SIZE / 2.0), TOP_TEXT_Y, // centre the string at top of window
				TEXT_SIZE);
		}

		{ // Draw level or right
			stringstream ss;
			ss << "Level " << level + 1;
			string str = ss.str();

			drawStrokeString(str,
				WORLD_RIGHT_EDGE + COL_SPACING - ((str.length() - 1) * TEXT_SIZE / 2.0), TOP_TEXT_Y, // centre the string at top of window
				TEXT_SIZE);
		}
	}
	else { // game is over
		stringstream ss;
		ss << "GAME OVER     Score " << score << "     Press s to start";
		string str = ss.str();

		drawStrokeString(str,
			WORLD_LEFT_EDGE - 2.5 * COL_SPACING, TOP_TEXT_Y, // centre the string at top of window
			TEXT_SIZE);
	}

	// Show any message for which we're pausing on a line below the top line

	if (pauseForMessage) {
		struct timeb now;
		ftime(&now);

		if (now.time + now.millitm / 1000.0 > pauseUntil.time + pauseUntil.millitm / 1000.0)

			// pause is over, so start a new level

			initLevel();

		else if (playerDied) {
			// pausing after player died

			string str = "YOU DIED";
			drawStrokeString(str,
				-((str.length() - 1) * TEXT_SIZE / 2.0), TOP_TEXT_Y - ROW_SPACING,
				TEXT_SIZE);
		}
		else if (goToNextLevel) {
			// pausing after level ended

			string str = "END OF LEVEL";
			drawStrokeString(str,
				-((str.length() - 1) * TEXT_SIZE / 2.0), TOP_TEXT_Y - ROW_SPACING,
				TEXT_SIZE);
		}
	}

	fontGPUProg->deactivate();
}

// Find the coordinates of the window edges so that the game window
// fits in an area with coordinates [-GAME_ASPECT,+GAME_ASPECT] x [-1,1].

void World::setWindowEdgeCoordinates()

{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	float windowAspect = width / (float)height;                       // aspect ratio of screen window
	float totalAspect = GAME_ASPECT;

	if (totalAspect > windowAspect) { // game should resize to fit horizontally in screen window
		l = -GAME_ASPECT;
		r = -l;

		b = l / windowAspect;
		t = -b;
	}
	else { // game should resize vertically to fit  in screen window
		b = -1;
		t = +1;

		l = b * windowAspect;
		r = -l;
	}
}