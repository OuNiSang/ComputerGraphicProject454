// player.h

#include "headers.h"
#include "drawbuffer.h"
#include "worldDefs.h"

#define PLAYER_BODY_COLOUR  vec3( 1.000, 1.000, 0.804 )
#define PLAYER_EYE_COLOUR   vec3( 1.000, 0.000, 0.000 )

class Player {
	static DrawBuffers* db;
	static vec2 playerGeometry[];
	static vec2 eyeCentres[];

public:

	vec2 pos; // position

	Player(vec2 _pos) {
		pos = _pos;

		if (db == NULL)
			generateVAOs();
	}

	void generateVAOs();
	void draw(mat4& worldToViewTransform);
	void moveTo(vec2 pos);
	void fire();
};
