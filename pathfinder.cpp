/*
 * =====================================================================================
 *
 *       Filename:  pathfinder.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  25/02/16 16:47:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Quintard Livaï (dyavil), livai.quintard@etu.univ-lyon1.fr
 *   Organization:  
 *
 * =====================================================================================
 */
#include "pathfinder.hpp"

graph used_graph;

int get_axis(movement & m)
{
    return m.current_state % 4;
}

int get_position(movement & m)
{
    return m.current_state/4;
}


void create_graph(RRBoard board, graph & g)
{
    unsigned int updated_state = 0;
    unsigned int step = sizeof(RRTile);
	//std::cout << sizeof(RRTile) << std::endl;
    RRRobot temp_robot;
    RRTile *current_tile = board.tiles;
	std::cout << current_tile->line << ", y :" << current_tile->column << std::endl;
    for(unsigned int i = 1; i <= board.tile_size; i++)
    {
        for(unsigned int j = 0; j < 4; j++)
        {
            movement start;
            start.current_state = updated_state;
            updated_state++;
            temp_robot.line = current_tile->line;
            temp_robot.column = current_tile->column;
            switch(j){
                case 0 : 
                    temp_robot.status = RR_ROBOT_S;
                    break;
                case 1 :
                    temp_robot.status = RR_ROBOT_W;
                    break;
                case 2 :
                    temp_robot.status = RR_ROBOT_N;
                    break;
                case 3 :
                    temp_robot.status = RR_ROBOT_E;
                    break;
                default : 
                    break;
            }
            char new_action = 'a';
            RRRobot savedtemp_robot;
            for(int rmove = RR_MOVE_FORWARD_1; rmove != END; rmove++)
            {
                savedtemp_robot = temp_robot;
		
                rr_board_play(board, savedtemp_robot, (RRRobotMove)rmove); 
                start.action = new_action;
                //convert line+column to state in graph
                unsigned int got_state = get_graph_state(board, savedtemp_robot);	
                start.arrival_state = got_state;

                new_action++;
                g.push_back(start);
            }

        }
		current_tile = (RRTile*)((char*)current_tile + step);
    }
}


int get_graph_state(RRBoard & board, RRRobot & robot)
{
	unsigned int res = get_graph_state_range(robot.column, robot.line, board);
	switch(robot.status){
                    case RR_ROBOT_E:
                        res += 3;
                        break;
                    case RR_ROBOT_N:
                        res += 2;
                        break;
                    case RR_ROBOT_W :
                        res += 1;
                        break;
                    case RR_ROBOT_S :
                        res += 0;
                        break;
                    default : 
                        res = 4000000000;
                        break;
            }
	return res;
}

int get_graph_state_range(int column, int line, RRBoard & board)
{
    unsigned int step = sizeof(RRTile);
    unsigned int count = 0;
    RRTile *current_tile = board.tiles;
    for(unsigned int i = 0; i < board.tile_size; i++)
    {
        if(current_tile->line == line && current_tile->column == column)
        {
            
            return count*4;
        }
        count++;
        current_tile = (RRTile*)((char*)current_tile + step);
    }
    return -1;
	
}


std::string graph_to_string(graph & g)
{
	
	std::stringstream ss;
	std::string str;
	movement tmp;
	
	for(size_t i = 0; i < g.size(); ++i)
	{
		tmp = g[i];
		ss << tmp.current_state << " " << tmp.action << " " << tmp.arrival_state << "\n";
	
	}
	
	str = ss.str();
	
	return str;

}


void graph_to_file(graph & g)
{

	std::ofstream os("./log/log.txt");
	
	if (!os)
	{
	
		std::cerr << "Error" << std::endl;
		
	}
	else
	{  
	
  		os << graph_to_string(g); 
  		 
	}

}


void shorter_path(graph & g, RRRobot & robot, RRBoard & board)
{
    movement start;
    unsigned int cur_state = get_graph_state(board, robot);
    start = g[1+(cur_state*7)];
    char action = 'a';
    for(int i = 0; i < 7; i++)
    {
        
        action++;
    }

}

