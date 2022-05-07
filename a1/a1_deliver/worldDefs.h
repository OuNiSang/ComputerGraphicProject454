// worldDefs.h

#define GAME_ASPECT      0.75   // width/height ratio of game

#define NUM_ROWS         20

#define ROW_SPACING      0.07
#define COL_SPACING      0.07

#define TOP_TEXT_Y       0.9

#define WORLD_TOP_ROW    0.8
#define WORLD_BOTTOM_ROW (WORLD_TOP_ROW - (NUM_ROWS-1)*ROW_SPACING)

#define WORLD_LEFT_EDGE   (-GAME_ASPECT)
#define WORLD_RIGHT_EDGE  GAME_ASPECT

#define PIECES_PER_CIRCLE 32  // number of straight pieces with which to approximate a circle

#define PAUSE_TIME_FOR_MESSAGE 2

#define MAX_CENTIPEDE_SEGMENTS 10
#define MAX_LEVEL              (MAX_CENTIPEDE_SEGMENTS - 1)

#define CENTIPEDE_INIT_SPEED           0.4 // distance per second in WCS
#define CENTIPEDE_SPEED_INC_PER_LEVEL  0.1

#define INIT_CENTIPEDE_POS vec2( 0.4, WORLD_TOP_ROW )
#define INIT_CENTIPEDE_DIR vec2( 1.0, 0.0 )

#define INIT_NUM_MUSHROOMS 60
#define MUSH_MAX_NUM		100
#define MUSH_MAX_DAMAGE       4 // number of hits before mushroom is destroyed

#define FLEA_INIT_SPEED			0.75
#define FLEA_SPEEDUP_SCORE		6000
#define FLEA_MAX_DAMAGE			2
#define FLEA_MUSHGEN_RATE		0.1	//Rate of flea generating mushroom durning in its wake
#define FLEA_MUSHGEN_STEP		5	//sorts of combination for mush generation
#define FLEA_MUSHGEN_MINHATE	4	//The minimum amount of mushroom trigger generation of flea on the row

#define INIT_LIVES_REMAINING 3

#define INIT_PLAYER_POS vec2( 0.0, -0.9 )

#define MAX_DARTS_AT_ONCE  3

#define SCORE_DESTROY_MUSHROOM   1
#define SCORE_REMAINING_MUSHROOM 5
#define SCORE_CENTIPEDE_HEAD     100
#define SCORE_CENTIPEDE_SEGMENT  10
#define SCORE_DESTROY_FLEA		 200
