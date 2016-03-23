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
#include <queue>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
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
 *         Name:  shortest_path
 *  Description:  using dijkstra alogorithm, determine the shortest path
 * =====================================================================================
 */
movement shortest_path(graph & g, RRRobot & robot_start, RRRobot & robot_goal);



