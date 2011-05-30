#include "config.h"
#include "fsm.h"
#include "igreboard.h"
#include "aversive.h"
#include "sharp.h"


extern igreboard_dev_t igreboard_device;
extern aversive_dev_t aversive_device;


enum emptyarea_state
{
  STATE_INIT = 0,
  STATE_LEAVE_START_AREA,
  STATE_FIND_PAWN,
  STATE_PLACE_TO_GRAB,
  STATE_SKIP_PAWN,
  STATE_TAKE_PAWN,
  STATE_DROP_PAWN_0,
  STATE_DROP_PAWN_1,
  STATE_DROP_PAWN_COMMON,
  STATE_DROP_GO_BACK,
  STATE_DISTRI_DONE,
  STATE_INVALID
};


typedef struct emptyarea_fsm
{
  enum emptyarea_state state;
  enum emptyarea_state next_state;

} emptyarea_fsm_t;


static unsigned int emptyarea_fsm_isdone(void* data_)
{
  emptyarea_fsm_t* const fsm = data_;
  return fsm->state = STATE_DISTRI_DONE;
}


static void emptyarea_fsm_next(void* data_)
{
#if 0 /* TODO */

  printf("distri strategy\n");

  unsigned int tiles[tiles_per_row * tiles_per_col];
  init_tiles(tiles);

  b._ticker.reset();
  b._asserv.set_velocity(400);

  // bot color
  const bool is_red = b.is_red();

  // angular velocity
  const int w = is_red ? 300 : -300;

  // angle to turn to at initialisation
  const unsigned int init_a = is_red ? 100 : 80;

  // angle to turn to at grabbing
  unsigned int grab_a;

  // minimum grabbing distance
  const unsigned int grab_dist = b._clamp.grabbing_distance();

  // index of the side sharps
  const size_t side_lo =
    is_red ? bot::RIGHT_LOW_MIDDLE : bot::LEFT_LOW_MIDDLE;
  const size_t side_co =
    is_red ? bot::RIGHT_LOW_FCORNER : bot::LEFT_LOW_FCORNER;

  // automaton state
  enum state state = STATE_LEAVE_START_AREA;

  // side sharps distance
  unsigned int dist_co;
  unsigned int dist_lo;

  // front sharps distance
  unsigned int ldist;
  unsigned int rdist;

  // current position
  int cur_x;
  int cur_y;

  // saved position
  int saved_x;
  int saved_y;

  // tile position
  unsigned int tile_x = 0;
  unsigned int tile_y = 0;

  // difference
  int diff;

#if 0 // unused
  // seen figure count
  unsigned int fig_count = 0;
#endif // unused

  // schedule automaton
  while (1)
  {
    switch (state)
    {
    case STATE_INIT:
    case STATE_LEAVE_START_AREA:
      printf("state_leave_start\n");
      b._asserv.move_forward(350);
      b._asserv.wait_done();
      b._asserv.turn(95, w);
      b._asserv.wait_done();
      b._asserv.move_forward(300);
      b._asserv.wait_done();
      state = STATE_FIND_PAWN;
      break ;

    case STATE_FIND_PAWN:
      printf("state_find_pawn\n");

      dist_co = (unsigned int)-1;
      dist_lo = (unsigned int)-1;

      b._asserv.turn_to(init_a);
      b._asserv.wait_done();

      b._asserv.move_forward(3000);

      while (b._asserv.is_done() == false)
      {
	// assume a wall is reached
	ldist = b._sharps[bot::FRONT_LOW_LCORNER].read();
	rdist = b._sharps[bot::FRONT_LOW_RCORNER].read();
	if ((ldist <= 150) && (rdist <= 150))
	{
	  printf("wall reached\n");

	  b._asserv.stop();
	  b._asserv.wait_done();
	  state = STATE_DISTRI_DONE;
	  break ;
	}

	// stop on a pawn until 2 sharps see it < 200
	if (dist_co == (unsigned int)-1)
	{
	  dist_co = b._sharps[side_co].read();
	  if (dist_co > 200)
	    dist_co = (unsigned int)-1;
	  continue ;
	}
	else if (dist_lo == (unsigned int)-1)
	{
	  dist_lo = b._sharps[side_lo].read();
	  if (dist_lo > 200)
	    dist_lo = (unsigned int)-1;
	  continue ;
	}

	// the 2 sharps saw it
	b._asserv.stop();
	b._asserv.wait_done();

	state = STATE_PLACE_TO_GRAB;
	break ;
      }
      break ;

    case STATE_PLACE_TO_GRAB:
      printf("place_to_grab\n");

      b._asserv.turn(90, w);
      b._asserv.wait_done();

      ldist = b._sharps[bot::FRONT_LOW_LEFT].read();
      rdist = b._sharps[bot::FRONT_LOW_RIGHT].read();

      // adjust orientation
      grab_a = 8;
      while (fabs(ldist - rdist) > 50)
      {
	if (ldist > rdist)
	  b._asserv.turn_right(grab_a);
	else
	  b._asserv.turn_left(grab_a);
	b._asserv.wait_done();

	if (grab_a == 1)
	  break ;

	grab_a -= 1;

	ldist = b._sharps[bot::FRONT_LOW_LEFT].read();
	rdist = b._sharps[bot::FRONT_LOW_RIGHT].read();
      }

      // get the position before moving to grab
      b._asserv.get_position(saved_x, saved_y);

#if 0 // unused
      if (b._sharps[bot::FRONT_HIGH_MIDDLE].read() < 200)
      {
	// skip the second figure, keep for bonus area
	if ((++fig_count) == 2)
	{
	  state = STATE_SKIP_PAWN;
	  break ;
	}
      }
#endif // unused

      // make ldist contain the max dist
      if (ldist < rdist)
	ldist = rdist;

      // move near enough to grab
      if (ldist > grab_dist)
      {
	b._asserv.move_forward(ldist - grab_dist + 20);
	b._asserv.wait_done();
      }

      state = STATE_TAKE_PAWN;
      break ;

#if 0 // unused
    case STATE_SKIP_PAWN:
      // skip the pawn
      b._asserv.turn_to(90);
      b._asserv.wait_done();
      b._asserv.move_forward(100);
      b._asserv.wait_done();
      state = STATE_FIND_PAWN;
      break ;
#endif // unused

    case STATE_TAKE_PAWN:
      printf("take_pawn\n");

      // grabbing failure
      if (b._clamp.grab() == false)
      {
	printf("grab() == false\n");
	state = STATE_FIND_PAWN;
	break ;
      }

      // get the tile we are on and normalize
      b._asserv.get_position(cur_x, cur_y);
      tile_x = (unsigned int)cur_x;
      tile_y = (unsigned int)cur_y;
      if (tile_x < 450) tile_x = 450;
      else if (tile_x > (3000 - 500)) tile_x = 3000 - 500;

      // if same color case 0, else case 1
      world_to_tile(tile_x, tile_y);
      if (is_red == is_tile_red(tile_x, tile_y))
	state = STATE_DROP_PAWN_0;
      else
	state = STATE_DROP_PAWN_1;

      // orient east west so that linear trajectory
      if (is_red) b._asserv.turn_to(180);
      else b._asserv.turn_to(0);

      break ;

    case STATE_DROP_PAWN_0:
      // move on the cell behind 
      printf("drop_pawn_0\n");

      tile_to_world(tile_x, tile_y);
      diff = ::abs(cur_x - (int)tile_x);
      b._asserv.move_forward(-diff);
      b._asserv.wait_done();

      state = STATE_DROP_PAWN_COMMON;

      break ;

    case STATE_DROP_PAWN_1:
      // move one cell behind
      printf("drop_pawn_1\n");

      if (is_red) tile_x += 1;
      else tile_x -= 1;

      tile_to_world(tile_x, tile_y);
      diff = ::abs(cur_x - (int)tile_x);
      b._asserv.move_forward(-diff);
      b._asserv.wait_done();

      state = STATE_DROP_PAWN_COMMON;

      break ;

    case STATE_DROP_PAWN_COMMON:
      // turn south, move back, drop pawn

      b._asserv.turn_to(270);
      b._asserv.wait_done();

      // next tile minus something
      tile_y += 350 - 110;
      b._asserv.get_position(cur_x, cur_y);
      b._asserv.move_forward(-(abs(cur_y - tile_y)));
      b._asserv.wait_done();

      b._clamp.drop();

      state = STATE_DROP_GO_BACK;

      break ;

    case STATE_DROP_GO_BACK:
      // go back to original pos

      b._asserv.move_forward(-50);
      b._asserv.wait_done();

      b._asserv.get_position(cur_x, cur_y);
      b._asserv.move_to(saved_x, cur_y);
      b._asserv.wait_done();

      state = STATE_FIND_PAWN;

      break ;

    case STATE_DISTRI_DONE:
      printf("distri_done\n");
      b._asserv.stop();
      b._asserv.wait_done();
      return ;
      break ;

    default:
      break ;
    }
  }

#endif

}


void emptyarea_fsm_initialize(fsm_t* fsm)
{
  emptyarea_fsm_t data;

  default_fsm_initialize(fsm);

  fsm->next = emptyarea_fsm_next;
  fsm->is_done = emptyarea_fsm_isdone;
  fsm->data = &data;

  data.state = STATE_INIT;
}
