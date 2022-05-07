// mushroom.h

#include "headers.h"
#include "drawbuffer.h"

#define MUSH_BODY_COLOUR      vec3( 1.000, 0.129, 0.741 )
#define MUSH_OUTLINE_COLOUR   vec3( 0.031, 0.851, 0.776 )
#define MUSH_DAMAGE_COLOUR    vec3( 0,	   0,	  0		)

class Mushroom {
	// VAOs for drawing ("static", so these are shared by all Mushroom instances)

	static DrawBuffers* db;
	static DrawBuffers* db2;
	static GLuint VAO; //The Unique VBO identify idex for this mushroom
	static vec2 mushroomOutlineGeometry[]; // The Geometry for mushroom's Head
	static vec2 mushroomBody1Geometry[]; // The Geometry for mushroom's Body
	static vec2 mushroomBody2Geometry[];
	static vec2 mushroomDamageGeometry[];// The Geometry for mushroom's damge
	static vec2 mushroomDamageCenter[];	//Centre of a Geometry mushroom

public:

	vec2 pos;
	int  damage;

	Mushroom(vec2 _pos) {
		pos = _pos;
		damage = 0;

		if (VAO == 0)
			generateVAOs();
	}

	void generateVAOs();

	void draw(mat4& worldToViewTransform);
};
