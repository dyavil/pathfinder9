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
    RRRobot temp_robot;
    RRTile *current_tile = board.tiles;
    for(unsigned int i = 0; i <= board.tile_size; i++)
    {
        for(unsigned int j = 0; j <= 4; j++)
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
            for(unsigned int rmove = RR_MOVE_FORWARD_1; rmove != RR_U_TURN; rmove++)
            {
                savedtemp_robot = temp_robot;
                rr_board_play(board, savedtemp_robot, (RRRobotMove)rmove); 
                start.action = new_action;
                //convert line+column to state in graph
                unsigned int got_state = get_graph_state(savedtemp_robot.column, savedtemp_robot.line, board);
                start.arrival_state = got_state;
                switch(savedtemp_robot.status){
                    case RR_ROBOT_E:
                        start.arrival_state += 3;
                        break;
                    case RR_ROBOT_N:
                        start.arrival_state += 2;
                        break;
                    case RR_ROBOT_W :
                        start.arrival_state += 1;
                        break;
                    case RR_ROBOT_S :
                        start.arrival_state += 0;
                        break;
                    default : 
                        start.arrival_state = 4000000000;
                        break;
            }
                new_action++;
                g.push_back(start);
            }

        }
    }
}

int get_graph_state(int column, int line, RRBoard & board)
{
    int step = sizeof(RRTile);
    int count = 0;
    RRTile *current_tile = board.tiles;
    for(unsigned int i = 0; i < board.tile_size; i++)
    {
        if(current_tile->line == line && current_tile->column == column)
        {
            return count*4;
        }
        count++;
        current_tile = current_tile + count*step;
    }
    return -1;
}




