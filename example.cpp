#include "board.hpp"
#include "pathfinder.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

void robot_cpy(const RRRobot& from, RRRobot& to)
{
  std::memcpy(&to, &from, sizeof(RRRobot)) ;
}

void move_print(
    RRRobotMove move
    )
{
  static const char* translations[7] =
  {
    "movind forward 1 tile",
    "movind forward 2 tiles",
    "movind forward 3 tiles",
    "moving backward 1 tile",
    "rotating left",
    "rotating right",
    "u turning"
  } ;

  std::printf("%s\n", translations[move]) ;
}

void robot_print(const RRRobot& robot)
{
  static const char* translations[5] =
  {
    "east",
    "north",
    "west",
    "south",
    "dead"
  } ;

  std::printf(
      "robot is at (%d, %d) looking %s\n", 
      robot.line,
      robot.column,
      translations[robot.status]
      ) ;
}

int main (int argc, char **argv)
{
  RRBoard board ;

  rr_board_init(board, "board.txt") ;
    
  graph g;
  create_graph(board, g);
  graph_to_file(g);
  

  if(!board.tile_size) {
    std::printf("the board has no tile\n") ;
    return 0 ;
  }

  RRRobot robot ;
  robot.line = board.tiles[0].line ;
  robot.column = board.tiles[0].column ;
  robot.status = RR_ROBOT_S ;

  RRRobot prev_robot ;
  robot_cpy(robot, prev_robot) ;


  RRRobot robot_goal ;
  robot_goal.line = board.tiles[24].line ;
  robot_goal.column = board.tiles[24].column ;
  robot_goal.status = RR_ROBOT_N;
	
  std::vector<movement> res;
  dijkstra(g, robot, robot_goal, res);
  for(unsigned int i = 0; i < res.size(); i++)
  {
    std::cout << "res i " << i << " is current : " << res[i].current_state << " arrival: " << res[i].arrival_state << std::endl;
  }

	/*movement res = shortest_path(g, robot, robot_goal);
	std::cout << res.arrival_state << std::endl;
	std::cout << res.action << std::endl;
	std::cout << res.current_state << std::endl;*/

  const RRRobotMove moves[13] =
  {
    RR_TURN_RIGHT,
    RR_MOVE_FORWARD_2,
    RR_MOVE_FORWARD_1,
    RR_U_TURN,
    RR_MOVE_FORWARD_1,
    RR_TURN_RIGHT,
    RR_MOVE_FORWARD_1,
    RR_MOVE_BACKWARD_1,
    RR_MOVE_FORWARD_2,
    RR_TURN_LEFT,
    RR_MOVE_FORWARD_3,
    RR_MOVE_BACKWARD_1,
    RR_MOVE_FORWARD_2
  } ;

  robot_print(robot) ;

  for(unsigned int i = 0; i < 13; ++i)
  {
    move_print(moves[i]) ;
    robot_cpy(robot, prev_robot) ;
    rr_board_play(board, robot, moves[i]) ;
    robot_print(robot) ;
  }

  rr_board_destroy(board) ;

  return 0 ;
}
