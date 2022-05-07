// centipede.h


#ifndef CENTIPEDE_H
#define CENTIPEDE_H

#include "headers.h"
#include "main.h"
#include "drawbuffer.h"
#include "seq.h"
#include "worldDefs.h"

#define PHASE_DELTA_PER_SEG 0.1
#define PHASE_RESOLUTION 0.05

#define SEG_BODY_RADIUS  (0.25 * ROW_SPACING)
#define SEG_HALO_RADIUS  (1.2 * SEG_BODY_RADIUS) 
#define SEG_LEG_LENGTH   (1.6 * SEG_BODY_RADIUS)   // distance from body centre to tip of leg
#define SEG_SEG_DISTANCE (1.8 * SEG_BODY_RADIUS) 

#define SEG_EYE_ANGLE    (45 * M_PI/180.0)         // angle from forward direction to eye centre
#define SEG_EYE_DISTANCE (0.6 * SEG_BODY_RADIUS)   // distance from body centre
#define SEG_EYE_X_RADIUS (0.2 * SEG_BODY_RADIUS)   // eye ellipse minor radius
#define SEG_EYE_Y_RADIUS (0.3 * SEG_BODY_RADIUS)   // eye ellipse major radius

#define SEG_BODY_COLOUR vec3( 1.000, 0.129, 0.741 )
#define SEG_LEG_COLOUR  vec3( 0.694, 0.467, 0.376 )
#define SEG_EYE_COLOUR  vec3( 0.031, 0.851, 0.776 )
#define SEG_HALO_COLOUR BACKGROUND_COLOUR

#define SEG_LEG_THETA0  ( 70.0 * M_PI/180.0) // range of angles that legs move through
#define SEG_LEG_THETA1  (125.0 * M_PI/180.0) //   for left side.  right side is mirrored.

#define CENTIPEDE_TURN_RADIUS (0.5 * ROW_SPACING)


class Segment {

  friend class Centipede;
  friend class World;

  // state
  
  vec2 pos;
  vec2 dir;

  bool  turning;
  float turnAngle;
  vec2  turnCentre;
  float turningPositionX;
  float dirUponTurnEntry;
  int   turnDir;

  // VAOs for drawing ("static", so these are shared by all Segment instances)
  
  static seq<DrawBuffers> headSegParams;
  static seq<DrawBuffers> bodySegParams;

 public:

  Segment( vec2 _pos, vec2 _dir ) {

    pos = _pos;
    dir = _dir;
    turning = false;
    turningPositionX = MAXFLOAT;

    // For the first segment ever created, set up the VAOs
    
    if (headSegParams.size() == 0)
      generateVAOs();
  }

  void generateVAOs();

  void draw( bool isHead, float phase, mat4 &worldToViewTransform );
};



class Centipede {

  float phase = 0; // in [0,1] for the phase of the centipede's leg movement

 public:

  seq<Segment*> segments;

  Centipede( int numSegs, vec2 headPos, vec2 dir ) {

    // Set up segments with head at headPos, and each other segment in
    // the -dir direction from the head, with 'SEG_SEG_DISTANCE' spacing
    // so that they overlap a bit.
    
    for (int i=0; i<numSegs; i++)
      segments.add( new Segment( headPos - i * SEG_SEG_DISTANCE * dir, dir ) );
  };


  void draw( mat4 &worldToViewTransform ) {

    for (int i=0; i<segments.size(); i++)
      segments[i]->draw( (i==0), // first argument to draw() is true for the head segment only
			 phase - i*PHASE_DELTA_PER_SEG, // phase varies with segment to make legs ripple
			 worldToViewTransform );
  }

  void updatePose( float elapsedTime );
};


#endif
