#ifndef ROBORALLY_BOARD_DISPLAY_HPP_
#define ROBORALLY_BOARD_DISPLAY_HPP_

#include "board.hpp"

/* display a board */

void rr_board_show (
    const RRBoard& board,
    const char* style,
    RRRobot& robot, /* use NULL if none, modified by clicks */
    RRRobot& previous_robot /* use NULL if none, modified by clicks */
    ) ;

/* edit a board file */

void rr_board_edit (
    RRBoard& board,
    const char* style
    ) ;

#endif
