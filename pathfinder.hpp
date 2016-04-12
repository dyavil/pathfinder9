/*
 * =====================================================================================
 *
 *       Filename:  pathfinder.hpp
 *
 *    Description:  pathfinder header file
 *
 *        Version:  1.0
 *        Created:  25/02/16 16:20:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Quintard Livaï, livai.quintard@etu.univ-lyon1.fr 11508352
 *                  Georges Antoine, antoine.georges@etu.univ-lyon1.fr 11200730
 *   Organization:  
 *
 * =====================================================================================
 */


#include <stdio.h>
#include <vector>
#include <set>
#include <algorithm>
#include <queue>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "board.hpp"

//a graph node
typedef struct movement{
    unsigned int current_state;/* position and direction on the board currently */
    char action;/* movement */
    unsigned int arrival_state;/* position and direction on the board at the end of action */
    unsigned int weight;/* edge weight */
    unsigned int weight_from_start;
}movement;

//board/graph correspondence array
typedef struct corresp_item
{
    unsigned int column;
    unsigned int line;
    unsigned int movement_id;
}corresp_item;
typedef std::vector<corresp_item> corresp_array; 

//a graph in memory
typedef struct graph{
    std::vector<movement> graph_vector;
    corresp_array graph_to_board;
}graph;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  create_graph
 *  Description:  convert a RRBoard to a graph in order to be used with pathfinder
 * =====================================================================================
 */
void create_graph(RRBoard board, graph & g);


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  quicker_pace
 *  Description:  return the minimum number of steps
 * =====================================================================================
 */
int quicker_pace(graph & g);


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_graph_state
 *  Description:  return an int representing the board block location
 * =====================================================================================
 */
int get_graph_state(RRBoard & board, RRRobot & robot);
int get_graph_state_range(int column, int line, RRBoard & board);


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  robotpos_to_movement
 *  Description:  return the corresponding movement
 * =====================================================================================
 */
movement robotpos_to_movement(RRRobot & robot, graph & g);


void build_corresp_array(RRBoard & board, graph & g);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  graph_to_string
 *  Description:  write a string representation of the graph
 * =====================================================================================
 */
std::string graph_to_string(graph & g);


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  graph_to_file
 *  Description:  take the previous function result and write it in a file
 * =====================================================================================
 */
void graph_to_file(graph & g);



//------------------------------------------------------------------------------------------------//


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dijkstra
 *  Description:  using dijkstra alogorithm, fill res with the shortest path to each
 *                position from the robot_start position
 * =====================================================================================
 */
void dijkstra(graph & g, RRRobot & robot_start, std::vector<movement> & res);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_way_to
 *  Description:  using dijkstra alogorithm, determine the shortest path to robot_goal
 * =====================================================================================
 */
std::string get_way_to(RRRobot & robot_goal, std::vector<movement> ways, graph g, std::vector<RRRobotMove> & rmoves);


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  artificial_player
 *  Description:  taking a action pool of 9 values, generate the best 5 actions to reach 
 *                the robot_goal position from robot position. If none of these actions
 *                permit it, dijkstra is used to get the best one(the closest one).
 * =====================================================================================
 */
std::vector<RRRobotMove> artificial_player(RRBoard board, graph g, RRRobot & robot, RRRobot & goal, std::vector<RRRobotMove> & actions, int & nb_mvt, std::string & dijkstra_movs);



