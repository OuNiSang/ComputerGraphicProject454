// player.cpp

#include "player.h"
#include "main.h"
#include "worldDefs.h"

// Move player to a position (called upon mouse movement)

void Player::moveTo(vec2 _pos)

{
	// Keep the player in at the bottom of the screen

	if (_pos.x < WORLD_LEFT_EDGE + COL_SPACING)
		_pos.x = WORLD_LEFT_EDGE + COL_SPACING;

	if (_pos.x > WORLD_RIGHT_EDGE - COL_SPACING)
		_pos.x = WORLD_RIGHT_EDGE - COL_SPACING;

	if (_pos.y > WORLD_BOTTOM_ROW - ROW_SPACING)
		_pos.y = WORLD_BOTTOM_ROW - ROW_SPACING;

	if (_pos.y < -1 + ROW_SPACING) // -1 is the bottom edge of the screen
		_pos.y = -1 + ROW_SPACING;

	// Update position

	pos = _pos;
}

// Player geometry

#define PLAYER_GEOM_COUNT 12

#define PLAYER_GEOM_CENTRE vec2(3.5,3.5)
#define PLAYER_GEOM_SCALE  (0.6 * ROW_SPACING / 9.0)

#define PLAYER_EYE_X_RADIUS  1.5
#define PLAYER_EYE_Y_RADIUS  2.0

vec2 Player::playerGeometry[PLAYER_GEOM_COUNT] = { // triangle fan of interior
  vec2(2,2),
  vec2(2,-1),
  vec2(5,-1),
  vec2(5,2),
  vec2(7,3),
  vec2(7,5),
  vec2(6,5),
  vec2(4,8),
  vec2(3,8),
  vec2(1,5),
  vec2(0,5),
  vec2(0,3)
};

vec2 Player::eyeCentres[2] = {
  vec2(2,5.5),
  vec2(5,5.5)
};

void Player::generateVAOs()

{
	// Start setting up a VAO

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Define the VBO

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Define the attributes

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0); // two floats for a position
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float))); // three floats for a colour
	glEnableVertexAttribArray(1);

	// Stop setting up this VAO.

	glBindVertexArray(0);

	// ---------------- Set up the geometry ----------------

	db = new DrawBuffers(VAO);

	// Since we don't know beforehand how many vertices there are,
	// fill in these 'positions' and 'colours'.  Once done, copy them
	// to the VBO.

	seq<vec2> positions;
	seq<vec3> colours;

	GLuint offset; // offset to first vertex of the current piece of geometry

	// ---- Build the body from 'playerGeometry' ----

	offset = positions.size();

	for (int i = 0; i < PLAYER_GEOM_COUNT; i++)
		positions.add(PLAYER_GEOM_SCALE * (playerGeometry[i] - PLAYER_GEOM_CENTRE));

	// set the same colour for each vertex

	for (int i = offset; i < positions.size(); i++)
		colours.add(PLAYER_BODY_COLOUR);

	// Add this geometry to the DrawBuffers structure

	db->mode.add(GL_TRIANGLE_FAN);
	db->first.add(offset);
	db->count.add(positions.size() - offset);

	// ---- Build the eyes ----

	for (int i = 0; i < 2; i++) {
		offset = positions.size();

		vec2& eyeCentre = eyeCentres[i];

		positions.add(PLAYER_GEOM_SCALE * (eyeCentre - PLAYER_GEOM_CENTRE));

		for (float theta = 0; theta < 2 * M_PI; theta += 2 * M_PI / (float)PIECES_PER_CIRCLE)
			positions.add(PLAYER_GEOM_SCALE * (eyeCentre + (PLAYER_EYE_X_RADIUS * cos(theta)) * vec2(1, 0) + (PLAYER_EYE_Y_RADIUS * sin(theta)) * vec2(0, 1) - PLAYER_GEOM_CENTRE));

		positions.add(positions[offset + 1]);

		// set the same colour for each vertex

		for (int i = offset; i < positions.size(); i++)
			colours.add(PLAYER_EYE_COLOUR);

		// Add this geometry to the DrawBuffers structure

		db->mode.add(GL_TRIANGLE_FAN);
		db->first.add(offset);
		db->count.add(positions.size() - offset);
	}

	// ---------------- Copy positions and colours to the VBO ----------------

	int numFloats = positions.size() * (2 + 3);

	float* buffer = new float[numFloats];

	float* p = buffer;
	for (int i = 0; i < positions.size(); i++) {
		*p++ = positions[i].x;
		*p++ = positions[i].y;
		*p++ = colours[i].x;
		*p++ = colours[i].y;
		*p++ = colours[i].z;
	}

	glBufferData(GL_ARRAY_BUFFER, numFloats * sizeof(float), buffer, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Player::draw(mat4& worldToViewTransform)

{
	// Provide MVP to GPU program

	mat4 T = translate(pos.x, pos.y, 0);
	mat4 MVP = worldToViewTransform * T;

	gpuProg->setMat4("MVP", MVP);

	// Draw it

	db->draw();
}

DrawBuffers* Player::db = NULL;