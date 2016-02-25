#include "board.hpp"

#include <cstdio>
#include <cstdlib>

/**** Types ****/

/** Movements **/

typedef enum {
  DIRECTION_EAST,
  DIRECTION_NORTH,
  DIRECTION_WEST,
  DIRECTION_SOUTH,
  DIRECTION_NONE
} Direction ;

typedef enum {
  ROTATION_RIGHT,
  ROTATION_LEFT,
  ROTATION_NONE
} Rotation ;

/**** Board tiles *****/

/** Utilities **/

/* generic comparison for tiles */

static int tile_compare(
    const void* generic_tile_1,
    const void* generic_tile_2
    )
{
  const RRTile* tile_1 = (RRTile*) generic_tile_1 ;
  const RRTile* tile_2 = (RRTile*) generic_tile_2 ;

  int c = (tile_2->line < tile_1->line) - (tile_1->line < tile_2->line) ;
  if(c) return c ;

  return (tile_2->column < tile_1->column) - (tile_1->column < tile_2->column) ;
}

/* search in a sorted tile array */

static RRTile* tile_search (
    const RRTile* tile_start,
    unsigned int tile_size,
    const RRTile* query
    )
{
  return (RRTile*) std::bsearch(
      query,
      tile_start,
      tile_size,
      sizeof(RRTile),
      tile_compare
      ) ;
}

/* recovering neighboring tiles */

static const RRTile* tile_get_N (
    const RRBoard* board,
    const RRTile* query /* must belong to the board */
    )
{
  RRTile key ;
  key.line = query->line - 1 ;
  key.column = query->column ;
  /* knowing the width of the board, the north tile is no further before */
  if(query - board->width > board->tiles)
  {
    return tile_search(query - board->width, board->width, &key) ;
  }
  else
  {
    return tile_search(board->tiles, query - board->tiles, &key) ;
  }
}

static const RRTile* tile_get_S (
    const RRBoard* board,
    const RRTile* query /* must belong to the board */
    )
{
  RRTile key ;
  key.line = query->line + 1 ;
  key.column = query->column ;
  /* knowing the width of the board, the south tile is no further after */
  if(query + board->width < board->tiles + board->tile_size)
  {
    return tile_search(query + 1, board->width, &key) ;
  }
  else
  {
    return tile_search(
        query + 1, 
        board->tiles + board->tile_size - query - 1 , 
        &key
        ) ;
  }
}

static const RRTile* tile_get_W (
    const RRBoard* board,
    const RRTile* query /* must belong to the board */
    )
{
  if(query > board->tiles)
  {
    /* the west tile is necessary just before */
    const RRTile* candidate = query - 1 ;
    if(candidate->line == query->line && candidate->column == query->column - 1)
    {
      return candidate ;
    }
    else
    {
      return NULL ;
    }
  }
  else
  {
    return NULL ;
  }
}

static const RRTile* tile_get_E (
    const RRBoard* board,
    const RRTile* query /* must belong to the board */
    )
{
  if(query < board->tiles + board->tile_size - 1)
  {
    /* the east tile is necessary just after */
    const RRTile* candidate = query + 1 ;
    if(candidate->line == query->line && candidate->column == query->column + 1)
    {
      return candidate ;
    }
    else
    {
      return NULL ;
    }
  }
  else
  {
    return NULL ;
  }
}

static const RRTile* tile_get(
    const RRBoard* board,
    const RRTile* query,
    Direction direction
    )
{
  switch(direction)
  {
    case DIRECTION_EAST  : return tile_get_E(board, query) ;
    case DIRECTION_NORTH : return tile_get_N(board, query) ;
    case DIRECTION_WEST  : return tile_get_W(board, query) ;
    case DIRECTION_SOUTH : return tile_get_S(board, query) ;
    case DIRECTION_NONE  : return query ;
  }

  return NULL ;
}

/* Check whether a belt is fast */
int tile_is_fast( 
    const RRTile* tile
    ) 
{
  return 
    tile && 
    tile->type > RR_TILE_JOIN_EW_TO_S && 
    tile->type < RR_TILE_ROTATE_RIGHT ;
}

/* Tile belt direction */

Direction tile_direction(
    const RRTile* tile
    )
{
  /* rotating tile and regular tiles have no direction */
  if(tile->type > RR_TILE_FAST_JOIN_EW_TO_S) 
    return DIRECTION_NONE ;

  /* fast belts are ordered the same way as regular belts */
  RRTileType base_type = (RRTileType) (tile->type % 24) ;

  /* non rotating belts, enum conversion is straightforward */
  if(base_type < RR_TILE_TURN_SE) 
    return (Direction) base_type ;

  /* conversion table for turns and perpendicular joins */
  static const Direction directions[8] = 
  {
    DIRECTION_EAST,
    DIRECTION_WEST,
    DIRECTION_NORTH,
    DIRECTION_NORTH,
    DIRECTION_WEST,
    DIRECTION_EAST,
    DIRECTION_SOUTH,
    DIRECTION_SOUTH
  } ;

  /* turns and perpendicular joins */
  if(base_type < RR_TILE_JOIN_NS_TO_E) 
    return directions[(base_type - RR_TILE_TURN_SE)%8] ;

  /* opposite joins, enum conversion is straightforward */
  return (Direction) (base_type - RR_TILE_JOIN_NS_TO_E) ;
}

Rotation tile_rotation(
    const RRTile* tile,
    Direction direction /* some tiles depend on the incoming direction */
    )
{
  /* static tiles */
  if(tile->type == RR_TILE_NOTHING) 
    return ROTATION_NONE ;

  /* simple rotating tiles */
  if(tile->type > RR_TILE_FAST_JOIN_EW_TO_S) 
    return (Rotation) ((tile->type - RR_TILE_ROTATE_RIGHT) % 2) ;

  /* fast belts are ordered the same way as regular belts */
  RRTileType base_type = (RRTileType) (tile->type % 24) ;

  /* non rotating belts */
  if(base_type < RR_TILE_TURN_SE) 
    return ROTATION_NONE ;

  /* simple rotating belts */
  if(base_type < RR_TILE_JOIN_S_TO_E) 
    return (Rotation) ((base_type - RR_TILE_TURN_SE) % 2) ;

  /* joins depend on the incoming direction */
  if(direction == DIRECTION_NONE) 
    return ROTATION_NONE ;

  /* constraint table for perpendicular joins */
  static const Direction perp_directions[8] = 
  {
    DIRECTION_SOUTH,
    DIRECTION_SOUTH,
    DIRECTION_EAST,
    DIRECTION_WEST,
    DIRECTION_NORTH,
    DIRECTION_NORTH,
    DIRECTION_WEST,
    DIRECTION_EAST
  } ;

  /* perpendicular joins */
  if(base_type < RR_TILE_JOIN_NS_TO_E)
  {
    if(direction == perp_directions[base_type - RR_TILE_JOIN_S_TO_E]) 
    {
      return (Rotation) (base_type % 2) ;
    }
    else
    {
      return ROTATION_NONE ;
    }
  }

  /* opposite joins */
  if((base_type - RR_TILE_JOIN_NS_TO_E) % 2 != direction % 2)
  {    
    /* this will be hard to figure in a month */
    return (Rotation) ((
        direction/2 + 
        direction%2 + 
        (base_type - RR_TILE_JOIN_NS_TO_E)/2
        ) % 2) ;
  }
  return ROTATION_NONE ;
}

/**** Robot ****/

/** Utilities **/

/* locate the tile the robot corresponding to the robot position */

static const RRTile* robot_locate(
    const RRTile* tiles,
    unsigned int tile_size,
    const RRRobot* robot
    )
{
  RRTile key ;
  key.line = robot->line ;
  key.column = robot->column ;
  return tile_search(tiles, tile_size, &key) ;
}

/** Moving **/

/* move the robot in a direction */
static void robot_move(
      RRRobot* robot,
      Direction direction
    )
{
  /* a dead robot does not move (OS dependent) */
  if(robot->status == RR_ROBOT_DEAD) return ;

  /* no direction, no chocolate */
  if(direction == DIRECTION_NONE) return ;

  /* good luck with that */
  robot->line += (direction%2) * (direction - 2) ;
  robot->column += ((1 + direction)%2) * (1 - direction) ;
}

/* rotate robot in a direction */
static void robot_rotate(
    RRRobot* robot,
    Rotation rotation
    )
{
  /* a dead robot does not rotate (OS dependent) */
  if(robot->status == RR_ROBOT_DEAD) return ;

  /* no rotation, no chocolate */
  if(rotation == ROTATION_NONE) return ;

  /* enum conversion is straightforward */
  if(rotation == ROTATION_LEFT)
  {
    robot->status = (RRRobotStatus) ((robot->status + 1) % 4) ;
  }
  else
  {
    robot->status = (RRRobotStatus) ((robot->status + 3) % 4) ;
  }
}

/* apply one tile to the robot, return the destination tile */
static const RRTile* robot_apply(
    RRRobot* robot,
    const RRBoard* board,
    const RRTile* tile /* this tile has to belong to the board */
    )
{
  if(tile)
  {
    /* direction of the robot tile */
    Direction dir = tile_direction(tile) ;

    if(dir == DIRECTION_NONE)
    {
      /* rotate the robot if necessary */
      robot_rotate(robot, tile_rotation(tile, DIRECTION_NONE)) ;
      /* the tile did not change */
      return tile ;
    }

    /* the robot is on a belt, move it */
    robot_move(robot, dir) ;

    /* get the arrival tile */
    const RRTile* dest_tile = tile_get(board, tile, dir) ;

    /* handle the case when the robot moves out of the board */
    if(dest_tile == NULL)
    {
      robot->status = RR_ROBOT_DEAD ;
      return dest_tile ;
    }

    /* handle the rotation of the destination tile */
    robot_rotate(robot, tile_rotation(dest_tile, (Direction) ((dir+2)%4))) ;

    /* return the destination tile */
    return dest_tile ;
  }
  else
  {
    /* the robot is in the void */
    robot->status = RR_ROBOT_DEAD ;
    return NULL ;
  }
}

/**** Public interface ****/

/** Construction **/

void rr_board_init (
    RRBoard& board_in,
    const char* filename
    )
{
  RRBoard* board = &board_in ;

  /* initialize board values */
  board->tile_size = 0 ;
  board->tiles = NULL ;
  board->width = 0 ;
  board->height = 0 ;

  /* check that the board is not NULL */
  if(!board) {
    printf("board location is not valid\n") ;
    return ;
  }

  /* open board file */
  FILE* board_file = fopen(filename, "r") ;
  if(!board_file) {
    printf("unable to open board file\n") ;
    return ;
  }

  /* read the board */
  int tile_size = 0 ; /* number of tiles read */
  int alloc_size = 200 ; /* allocated space for tiles. DO NOT CHANGE TO 0 */
  board->tiles = (RRTile*) malloc(alloc_size * sizeof(RRTile)) ;
  while(
      fscanf(
        board_file,
        "%d %d %d",
        &(board->tiles[tile_size].line),
        &(board->tiles[tile_size].column),
        (int*) &(board->tiles[tile_size].type)
        )
      == 3
    )
  {
    ++tile_size ;
    if(tile_size == alloc_size) {
      /* extend the available memory for tiles */
      RRTile* tmp = (RRTile*) realloc(board->tiles, 2*alloc_size*sizeof(RRTile)) ;
      if(!tmp) 
      {
        /* reallocation failed for twice the original size */
        tmp = (RRTile*) realloc(board->tiles, (alloc_size + 1)*sizeof(RRTile)) ;
        if(!tmp)
        {
          /* reallocation failed for one additional tile */
          printf("not able to fit all the tiles in memory\n") ;
          free(board->tiles) ;
          fclose(board_file) ;
          board->tiles = NULL ;
          return ;
        }
        else
        {
          /* one extra tile could be allocated */
          ++alloc_size ;
          board->tiles = tmp ;
        }
      }
      else
      {
        /* successfully allocated twice the original size */
        alloc_size *= 2 ;
        board->tiles = tmp ;
      }
    }
  }

  /* closing the file */
  fclose(board_file) ;

  /* shrink the memory for the tiles */
  RRTile* tmp_tiles = (RRTile*) realloc(board->tiles, tile_size * sizeof(RRTile)) ;
  if(!tmp_tiles)
  {
    printf("unable to shrink tile memory, weird but not fatal\n") ;
  }
  else
  {
    board->tiles = tmp_tiles ;
  }
  /* record tile size */
  board->tile_size = tile_size ;

  /* normalize coordinates, compute width and height */
  if(board->tile_size)
  {
    unsigned int lmin = board->tiles[0].line ;
    unsigned int lmax = board->tiles[0].line ;
    unsigned int cmin = board->tiles[0].column ;
    unsigned int cmax = board->tiles[0].column ;

    for(
        RRTile* t = board->tiles + 1 ; 
        t < board->tiles + board->tile_size ;
        ++t
        )
    {
      lmin = (unsigned int) t->line < lmin ? t->line : lmin ;
      lmax = (unsigned int) t->line > lmax ? t->line : lmax ;
      cmin = (unsigned int) t->column < cmin ? t->column : cmin ;
      cmax = (unsigned int) t->column > cmax ? t->column : cmax ;
    }

    /* assign width and height */
    board->width = cmax - cmin + 1 ;
    board->height = lmax - lmin + 1 ;

    /* normalize coordinates */
    for(
        RRTile* t = board->tiles ; 
        t < board->tiles + board->tile_size ; 
        ++t
        )
    {
      t->line   -= lmin ;
      t->column -= cmin ;
    }
  }

  /* sort the tiles for easy finding */
  std::qsort(board->tiles, board->tile_size, sizeof(RRTile), tile_compare) ;
}

void rr_board_destroy (
    RRBoard& board
    )
{
  /* free board members */
  free(board.tiles) ;
  board.tiles = NULL ;
  board.tile_size = 0 ;
}

void rr_board_save(
    RRBoard& board,
    const char* filename
    )
{
  /* open the file for writing */
  FILE* board_file = fopen(filename, "w") ;

  for(
      RRTile* t = board.tiles ; 
      t < board.tiles + board.tile_size ; 
      ++t
      )
  {
    fprintf(board_file, "%u %u %u\n", t->line, t->column, t->type) ;
  }

  fclose(board_file) ;
}

/** Robot moving **/

static void board_apply (
    const RRBoard* board,
    RRRobot* robot
    )
{
  /* a dead robot does not move (OS dependent) */
  if(robot->status == RR_ROBOT_DEAD) return ;

  /* location of the robot */
  const RRTile* tile = robot_locate(board->tiles, board->tile_size, robot) ;

  /* apply the tile to the robot */
  const RRTile* dest_tile = robot_apply(robot, board, tile) ;

  /* handle fast belts */
  if(tile_is_fast(tile) && tile_is_fast(dest_tile))
  {
    robot_apply(robot, board, dest_tile) ;
  }
}

void rr_board_play (
    const RRBoard& board,
    RRRobot& robot,
    RRRobotMove move
    )
{
  /* fast exit for dead robots */
  if(robot.status == RR_ROBOT_DEAD) return ;

  /* determine the varying coordinate */
  switch(move)
  {
    case RR_MOVE_FORWARD_1 : 
      robot_move(&robot, (Direction) robot.status) ;
      break ;
    case RR_MOVE_FORWARD_2 : 
      robot_move(&robot, (Direction) robot.status) ;
      robot_move(&robot, (Direction) robot.status) ;
      break ;
    case RR_MOVE_FORWARD_3 :
      robot_move(&robot, (Direction) robot.status) ;
      robot_move(&robot, (Direction) robot.status) ;
      robot_move(&robot, (Direction) robot.status) ;
      break ;
    case RR_MOVE_BACKWARD_1 : 
      robot_move(&robot, (Direction) ((robot.status+2)%4)) ;
      break ;
    case RR_TURN_LEFT :
      robot_rotate(&robot, ROTATION_LEFT) ;
      break ;
    case RR_TURN_RIGHT :
      robot_rotate(&robot, ROTATION_RIGHT) ;
      break ;
    case RR_U_TURN :
      robot_rotate(&robot, ROTATION_RIGHT) ;
      robot_rotate(&robot, ROTATION_RIGHT) ;
      break ;
  }
  board_apply(&board, &robot) ;
}
