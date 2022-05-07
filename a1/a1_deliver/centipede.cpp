// centipede.cpp

#include "centipede.h"
#include "main.h"


// VAOs for the centipede segments

seq<DrawBuffers> Segment::headSegParams;
seq<DrawBuffers> Segment::bodySegParams;


#include "world.h"
extern World *world;


#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))


void Centipede::updatePose( float elapsedTime )

{
  // Determine distance travelled
  
  float distanceToTravel = elapsedTime * (CENTIPEDE_INIT_SPEED + world->level*CENTIPEDE_SPEED_INC_PER_LEVEL) * speedMultiplier;

  // If the head is not already turning, check for an obstacle ahead
  // and return its position.  However, if the head is on the last
  // row, don't check for anything and let it continue off the edge of
  // the screen.

  float turningPositionX = MAXFLOAT;

  if (!segments[0]->turning) {

    if (segments[0]->dir.x < 0) // moving left
      turningPositionX = WORLD_LEFT_EDGE + 4*SEG_BODY_RADIUS;
    else // moving right
      turningPositionX = WORLD_RIGHT_EDGE - 4*SEG_BODY_RADIUS;

    // If there's a closer mushroom, set 'turningPositionX' to that
    // mushroom's x position

    Mushroom *closestMush = world->findClosestMushroomAhead( segments[0]->pos, segments[0]->dir );

    if (closestMush)
      if (segments[0]->dir.x < 0) { // moving left
	if (closestMush->pos.x > turningPositionX)
	  turningPositionX = closestMush->pos.x;
      } else // moving right
	if (closestMush->pos.x < turningPositionX)
	  turningPositionX = closestMush->pos.x;

    // Move the turning position closer to the centipede so that it
    // turns in time.
  
    if (segments[0]->dir.x < 0)
      turningPositionX = MIN( turningPositionX+COL_SPACING, segments[0]->pos.x );
    else
      turningPositionX = MAX( turningPositionX-COL_SPACING, segments[0]->pos.x );
  }
  
  // Update each segment position. If the segment is turning,
  // continue its turn until 'distance' has been travelled or the
  // segment comes out of the turn.

  for (int i=0; i<segments.size(); i++) {
    
    float distanceRemaining = distanceToTravel;

    float dir = signum(segments[i]->dir.x); // direction in x as +1 or -1

    while (distanceRemaining > 0) {
      
      // Step 1: If not turning already, travel horizontally as far as
      // possible without entering the next turn.

      if (!segments[i]->turning)
      
	if (turningPositionX == MAXFLOAT || distanceRemaining < dir * (turningPositionX - segments[i]->pos.x)) {

	  // No turning point, or distance to turning point is greater
	  // than distance remaining to travel, so just travel

	  segments[i]->pos = segments[i]->pos + distanceRemaining * segments[i]->dir;

	  distanceRemaining = 0;

	} else {

	  // Will reach the turning point before the distance to travel,
	  // so move to turning point, then start turning.

	  distanceRemaining -= fabs(turningPositionX - segments[i]->pos.x);

	  segments[i]->pos.x = turningPositionX; // advance

	  segments[i]->turning = true; // start turning
	  segments[i]->turnAngle = 0;
	  segments[i]->dirUponTurnEntry = dir;
	  segments[i]->turningPositionX = turningPositionX;

	  // Choose whether to turn upward or downward
	  
	  if (i == 0) {
	    
	    // For the head, turn down unless inside the player area
	    
	    if (segments[0]->pos.y < WORLD_BOTTOM_ROW - 2*ROW_SPACING) { // turn up or down if in player area

	      if (segments[0]->pos.y < -1 + 1.5*ROW_SPACING) // turn up if on last row
		segments[0]->turnDir = +1;
	      else
		segments[0]->turnDir = (randIn01() > 0.5 ? -1 : +1); // turn randomly otherwise

	    } else // not yet in player area

	      segments[0]->turnDir = -1; // turn down

	  } else // For body segments, copy the turning direction of the segment in front

	    segments[i]->turnDir = segments[i-1]->turnDir;

	  // Determine the turn centre

	  if (segments[i]->turnDir == -1) // downward
	    segments[i]->turnCentre = vec2( segments[i]->pos.x, segments[i]->pos.y - CENTIPEDE_TURN_RADIUS );
	  else // upward
	    segments[i]->turnCentre = vec2( segments[i]->pos.x, segments[i]->pos.y + CENTIPEDE_TURN_RADIUS );

	  // If this is a non-head segment (which has started
	  // turning), the 'turningPositionX' should be set to
	  // MAXFLOAT in the *preceding* segment, since that preceding
	  // segment no longer has to inform the segment behind of the
	  // turn.
	  
	  if (i > 0)
	    segments[i-1]->turningPositionX = MAXFLOAT;
	}

      // Step 2: If turning, travel in the turn as far as possible
      // without leaving the turn.

      if (segments[i]->turning) {

	float distRemainingInTurn = CENTIPEDE_TURN_RADIUS * (M_PI - segments[i]->turnAngle); // turn takes from 0 to pi radians (i.e. 180 degrees)

	if (distanceRemaining < distRemainingInTurn) {

	  // Distance to turn exit is greater than distance remaining to
	  // travel, so just travel along the turn.

	  segments[i]->turnAngle += distanceRemaining / CENTIPEDE_TURN_RADIUS;

	  segments[i]->dir = vec2( segments[i]->dirUponTurnEntry * cos(segments[i]->turnAngle),
				   segments[i]->turnDir * sin(segments[i]->turnAngle) );

	  segments[i]->pos = segments[i]->turnCentre + CENTIPEDE_TURN_RADIUS * vec2( segments[i]->dirUponTurnEntry * sin(segments[i]->turnAngle), 
										     -segments[i]->turnDir * cos(segments[i]->turnAngle ) );

	  distanceRemaining = 0;

	} else {

	  // Will reach turn exit before the distance to travel, so
	  // move to the turn exit, then start going straight.

	  segments[i]->pos = segments[i]->turnCentre + vec2( 0, segments[i]->turnDir * CENTIPEDE_TURN_RADIUS );
	  segments[i]->dir = vec2( -segments[i]->dirUponTurnEntry, 0 );

	  distanceRemaining -= distRemainingInTurn;

	  segments[i]->turning = false;
	}
      }
    }

    // The segment passes to the *next* segment (i.e. the next
    // iteration of this loop) the position at which it started
    // turning (or MAXFLOAT if it did not start turning).  In the next
    // loop iteration, the next segment will check this
    // turningPositionX.
    
    turningPositionX = segments[i]->turningPositionX;
  }

  // Update centipede's phase
  //
  // We have to make sure that the legs don't slide across the ground,
  // so the phase change must depend on the distance travelled.

  float legTravelPerCycle = 2 * SEG_LEG_LENGTH * sin( fabs( SEG_LEG_THETA0 - SEG_LEG_THETA1 ));

  phase += distanceToTravel / legTravelPerCycle;
}



void Segment::draw( bool isHead, float phase, mat4 &worldToViewTransform ) 

{
  // Move phase into range [0,1]
    
  phase = phase - floor(phase);

  // Provide MVP to GPU program

  mat4 T = translate( pos.x, pos.y, 0 );
  mat4 R = rotate( atan2( dir.y, dir.x ), vec3(0,0,1) ); // [for students ???]
  mat4 MVP = worldToViewTransform * T * R;

  gpuProg->setMat4( "MVP", MVP );

  // Draw things

  int phaseIndex = rint( phase / PHASE_RESOLUTION );

  if (phaseIndex >= headSegParams.size())
    phaseIndex = headSegParams.size()-1; // just in case ...

  if (isHead)
    headSegParams[phaseIndex].draw();
  else  
    bodySegParams[phaseIndex].draw();
};



void Segment::generateVAOs()

{
  int numPhases = ceil( 1.0 / PHASE_RESOLUTION );

  // Build the segments

  for (int isHead=0; isHead<2; isHead++) // treat 'isHead' as a boolean using values 0 and 1

    for (int phaseIndex=0; phaseIndex<numPhases; phaseIndex++) { // each phase has a slightly different positioning of the legs

      // Start setting up a VAO
    
      GLuint VAO;
      glGenVertexArrays( 1, &VAO );
      glBindVertexArray( VAO );

      // Define the VBO
    
      GLuint VBO;
      glGenBuffers( 1, &VBO );
      glBindBuffer( GL_ARRAY_BUFFER, VBO );

      // Define the attributes

      glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0 ); // two floats for a position
      glEnableVertexAttribArray( 0 );

      glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*) (2*sizeof(float)) ); // three floats for a colour
      glEnableVertexAttribArray( 1 );

      // Stop setting up this VAO.

      glBindVertexArray( 0 );

      // ---------------- Set up the geometry ----------------

      DrawBuffers db( VAO );

      // Since we don't know beforehand how many vertices there are,
      // fill in these 'positions' and 'colours'.  Once done, copy them
      // to the VBO.
    
      seq<vec2> positions;
      seq<vec3> colours;
    
      GLuint offset; // offset to first vertex of the current piece of geometry

      // ---- Build the body circle with a triangle fan ----

      offset = positions.size();

      positions.add( vec2(0,0) );

      for (float theta=0; theta<2*M_PI; theta += 2*M_PI / (float) PIECES_PER_CIRCLE)
      	positions.add( vec2( SEG_BODY_RADIUS*cos(theta), SEG_BODY_RADIUS*sin(theta)  ) );

      positions.add( positions[offset+1] ); // add the first one again to close the circle ... just in case

      // set the same colour for each vertex

      for (int i=offset; i<positions.size(); i++)
      	colours.add( SEG_BODY_COLOUR );

      // Add this geometry to the DrawBuffers structure

      db.mode.add( GL_TRIANGLE_FAN );
      db.first.add( offset );
      db.count.add( positions.size() - offset );

      // ---- Build a halo around the body circle with a triangle strip ----

      offset = positions.size();

      for (float theta=0; theta<2*M_PI; theta += 2*M_PI / (float) PIECES_PER_CIRCLE) {
      	positions.add( vec2( SEG_BODY_RADIUS*cos(theta), SEG_BODY_RADIUS*sin(theta)  ) );
      	positions.add( vec2( SEG_HALO_RADIUS*cos(theta), SEG_HALO_RADIUS*sin(theta)  ) );
      }

      positions.add( positions[ offset   ] );
      positions.add( positions[ offset+1 ] );

      // set the same colour for each vertex

      for (int i=offset; i<positions.size(); i++)
      	colours.add( SEG_HALO_COLOUR );

      // Add this geometry to the DrawBuffers structure

      db.mode.add( GL_TRIANGLE_STRIP );
      db.first.add( offset );
      db.count.add( positions.size() - offset );

      // ---- Build the legs with lines ----

      offset = positions.size();

      // The phaseIndex determines the leg orientation, which varies
      // linearly in [ SEG_LEG_THETA0, SEG_LEG_THETA1 ].

      float legPhase = sin( phaseIndex / (float) numPhases * 2*M_PI );  // varies in [-1,1)

      float legAngle = 0.5*(SEG_LEG_THETA0 + SEG_LEG_THETA1) + legPhase * 0.5 * (SEG_LEG_THETA1 - SEG_LEG_THETA0);

      // left leg
      
      positions.add( vec2( SEG_BODY_RADIUS*cos(legAngle), SEG_BODY_RADIUS*sin(legAngle)  ) );
      positions.add( vec2( SEG_LEG_LENGTH*cos(legAngle), SEG_LEG_LENGTH*sin(legAngle)  ) );

      // right leg
      
      positions.add( vec2( SEG_BODY_RADIUS*cos(-legAngle), SEG_BODY_RADIUS*sin(-legAngle)  ) );
      positions.add( vec2( SEG_LEG_LENGTH*cos(-legAngle), SEG_LEG_LENGTH*sin(-legAngle)  ) );

      // set the same colour for each vertex

      for (int i=offset; i<positions.size(); i++)
	colours.add( SEG_LEG_COLOUR );

      // Add this geometry to the DrawBuffers structure

      db.mode.add( GL_LINES );
      db.first.add( offset );
      db.count.add( positions.size() - offset );

      // ---- For a head, add oval eyes ----

      // [YOUR CODE HERE]
      
      if (isHead)

	for (int i=0; i<2; i++) { // two eyes
    
	  offset = positions.size();

	  float theta = SEG_EYE_ANGLE * (i*2-1); // -angle on one iteration, +angle on other iteration

	  vec2 eyeCentre( SEG_EYE_DISTANCE * cos(theta), SEG_EYE_DISTANCE * sin(theta) );
	  vec2 eyeXDir = eyeCentre.normalize();
	  vec2 eyeYDir = vec2( -eyeXDir.y, eyeXDir.x );

	  positions.add( eyeCentre );

	  for (float theta=0; theta<2*M_PI; theta += 2*M_PI / (float) PIECES_PER_CIRCLE)
	    positions.add( eyeCentre + (SEG_EYE_X_RADIUS * cos(theta)) * eyeXDir + (SEG_EYE_Y_RADIUS * sin(theta)) * eyeYDir );

	  positions.add( positions[ offset+1 ] );

	  // set the same colour for each vertex

	  for (int i=offset; i<positions.size(); i++)
	    colours.add( SEG_EYE_COLOUR );

	  // Add this geometry to the DrawBuffers structure

	  db.mode.add( GL_TRIANGLE_FAN );
	  db.first.add( offset );
	  db.count.add( positions.size() - offset );
	}

      // ---------------- Copy positions and colours to the VBO ----------------

      int numFloats = positions.size() * (2+3);

      float *buffer = new float[ numFloats ];

      float *p = buffer;
      for (int i=0; i<positions.size(); i++) {
	*p++ = positions[i].x;
	*p++ = positions[i].y;
	*p++ = colours[i].x;
	*p++ = colours[i].y;
	*p++ = colours[i].z;
      }

      glBufferData( GL_ARRAY_BUFFER, numFloats*sizeof(float), buffer, GL_STATIC_DRAW );
      
      glBindBuffer( GL_ARRAY_BUFFER, 0 );

      // Add to the DrawBuffers

      if (isHead)
	headSegParams.add( db );
      else
	bodySegParams.add( db );
    }
}
