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
}movement;


//a graph in memory
typedef std::vector<movement> graph;


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
int get_graph_state(int column, int line, RRBoard & board);


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

