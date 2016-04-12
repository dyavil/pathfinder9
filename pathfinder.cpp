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
                start.weight_from_start = -1;
                new_action++;
                start.weight = 1;
                g.graph_vector.push_back(start);
            }

        }
		current_tile = (RRTile*)((char*)current_tile + step);
    }
    build_corresp_array(board, g);
}


void build_corresp_array(RRBoard & board, graph & g)
{
    unsigned int step = sizeof(RRTile);
    unsigned int count = 0;
    RRTile *current_tile = board.tiles;
    corresp_item temp_item;
    for(unsigned int i = 0; i < board.tile_size; i++)
    {
        temp_item.column = current_tile->column;
        temp_item.line = current_tile->line;
        temp_item.movement_id = count;
        g.graph_to_board.push_back(temp_item);
        count = count+4;
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


movement robotpos_to_movement(RRRobot & robot, graph & g)
{
    movement res;
    bool test = false;
    for(unsigned int i = 0; i < g.graph_to_board.size(); i++)
    {
        if(g.graph_to_board[i].column == robot.column && g.graph_to_board[i].line == robot.line)
        {
            unsigned int tempid = g.graph_to_board[i].movement_id;
	        switch(robot.status){
                case RR_ROBOT_E:
                    tempid += 3;
                    break;
                case RR_ROBOT_N:
                    tempid += 2;
                    break;
                case RR_ROBOT_W :
                    tempid += 1;
                    break;
                case RR_ROBOT_S :
                    tempid += 0;
                    break;
                default : 
		            test = true;
                    break;
            }
            res = g.graph_vector[tempid*7];
	        if(test) res.current_state = 4000000000;
            return res;

        }
    }
    res.current_state = 4000000000;
    return res;
}



std::string graph_to_string(graph & g)
{

	std::stringstream ss;
	std::string str;
	movement tmp;

	for(size_t i = 0; i < g.graph_vector.size(); ++i)
	{
		tmp = g.graph_vector[i];
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


struct movement_compare
{
    bool operator()(const movement & mvt, const movement & mvt2)const
    {
        return (mvt.weight_from_start > mvt2.weight_from_start);
    }
};

bool mov_in_vector(std::vector<movement> & v, movement & m)
{
    for(unsigned int i = 0; i < v.size(); i++)
    {
        if(v[i].current_state == m.current_state) return true;
    }
    return false;
}

bool sort_mov(movement m1, movement m2)
{
    return m1.weight_from_start < m2.weight_from_start;
}

void dijkstra(graph & g, RRRobot & robot_start, std::vector<movement> & res)
{
    unsigned int ds = 0;
    movement current_mov = robotpos_to_movement(robot_start, g);
    current_mov.weight_from_start = ds;
    std::vector<movement> queue;
    std::set<unsigned int> done;
    for (std::vector<movement>::iterator it = g.graph_vector.begin(); it != g.graph_vector.end(); ++it)
    {
        it->weight_from_start = -1;
    }
    for(unsigned int i = 0; i < 7; i++)
    {
        g.graph_vector[current_mov.current_state*7+i].weight_from_start = ds;
    }
    queue.push_back(current_mov);
    done.insert(current_mov.current_state);
    current_mov.weight_from_start = -1;
    while(!queue.empty())
    {
        //get the first vetor's element
        current_mov = queue.front();
        //then remove it
        //res.push_back(current_mov);
        queue.erase(queue.begin());

        for(unsigned int i = 0; i < 7; ++i)
        {
            unsigned int weight_from_prec = current_mov.weight;
            unsigned int compare_weight = weight_from_prec + current_mov.weight_from_start;
            if(g.graph_vector[current_mov.current_state*7+i].arrival_state != 4000000000)
            {
                if(g.graph_vector[g.graph_vector[current_mov.current_state*7+i].arrival_state*7].weight_from_start > compare_weight)
                {
                    for(unsigned int j = 0; j < 7; ++j)
                    {
                        g.graph_vector[g.graph_vector[current_mov.current_state*7+i].arrival_state*7+j].weight_from_start = compare_weight;
                    }

                }


                if(done.find(g.graph_vector[g.graph_vector[current_mov.current_state*7+i].arrival_state*7].current_state) == done.end())
                {
                    queue.push_back(g.graph_vector[g.graph_vector[current_mov.current_state*7+i].arrival_state*7]);
                    done.insert(g.graph_vector[current_mov.current_state*7+i].arrival_state); 
                }
                else
                {
                    for(unsigned int j = 0; j < queue.size(); j++)
                    {
                        //std::cout << "d loop f2 : " << it->weight_from_start << std::endl;
                        if(queue[j].current_state == g.graph_vector[current_mov.current_state*7+i].arrival_state)
                        {
                            queue[j].weight_from_start = g.graph_vector[g.graph_vector[current_mov.current_state*7+i].arrival_state*7].weight_from_start;
                            j = queue.size();
                        }
                    }  
                    //std::cout << "d loop f2 if: " << std::endl;
                }
                std::sort(queue.begin(), queue.end(), sort_mov);
            }

        }
        for(unsigned int i = 0; i < 7; i++)
        {
            res.push_back(g.graph_vector[current_mov.current_state*7+i]);
        }
        //res.push_back(current_mov);
    }

}


std::string get_way_to(RRRobot & goal, std::vector<movement> ways, graph g, std::vector<RRRobotMove> & rmoves)
{

    movement m = robotpos_to_movement(goal, g);
    unsigned int min = 4000000000;
    std::string res = "";
    std::string resf = "";
    std::string rr ="";
    movement tm;
    for(unsigned int i = 0; i < ways.size(); i++)
    {
        if(ways[i].arrival_state == m.current_state && ways[i].arrival_state != ways[i].current_state)
        {
            if(ways[i].weight_from_start < min){ 
                min = ways[i].weight_from_start;
                tm = ways[i];
            }
        }
    }
    min += 1;
    res += "On arrive à " + std::to_string(tm.arrival_state) + " en " + std::to_string(min) + " en passant par " + std::to_string(tm.current_state) + ", ";
    rr += tm.action;
    if(min == 4000000001)
    {
        resf = "Inatteignable";
        rmoves.clear();
        return resf;
    }
    //std::cout << std::to_string(min) << std::endl;
    for(unsigned int i = 0; i < min-1; i++)
    {
        for(unsigned int j = 0; j < ways.size(); j++)
        {
            if(ways[j].arrival_state == tm.current_state && ways[j].weight_from_start == min-2-i)
            {
                res += std::to_string(ways[j].current_state) + ", ";
                tm = ways[j];
                rr += tm.action;
                j = ways.size();
            }
        }
    }

    //std::cout << rr.size() << std::endl;
    for( int i = rr.length()-1; i >=0; --i)
    {
        //std::cout << i << std::endl;
        char t = rr[i];
        switch(t)
        {
            case 'a':
                resf += "Avance de 1 ,";
                rmoves.push_back((RRRobotMove)0);
                break;
            case 'b':
                resf += "Avance de 2 ,";
                rmoves.push_back((RRRobotMove)1);
                break;
            case 'c':
                resf += "Avance de 3 ,";
                rmoves.push_back((RRRobotMove)2);
                break;
            case 'd':
                resf += "Recule de 1 ,";
                rmoves.push_back((RRRobotMove)3);
                break;
            case 'e':
                resf += "Tourne à gauche ,";
                rmoves.push_back((RRRobotMove)4);
                break;
            case 'f':
                resf += "Tourne à droite ,";
                rmoves.push_back((RRRobotMove)5);
                break;
            case 'g':
                resf += "Demi tour ,";
                rmoves.push_back((RRRobotMove)6);
                break;
        }
    } 
    return resf;
}

std::vector<RRRobotMove> artificial_player(RRBoard board, graph g, RRRobot & robot, RRRobot & goal, std::vector<RRRobotMove> & actions, int & nb_mvt)
{
    std::vector<movement> ways;
    unsigned int postemp;
    nb_mvt = 0;
    RRRobot robtemp = robot;
    std::vector<RRRobotMove> action_final;
    std::vector<RRRobotMove> best;
    std::vector<RRRobotMove> best_by_dijkstra;
    std::string dijkstra_movs = "";

    best = actions;
    unsigned int weight_dijkstra = 4000000000;
    movement st = robotpos_to_movement(robot, g);
    movement goalmov = robotpos_to_movement(goal, g);
    if(robotpos_to_movement(robtemp, g).current_state == goalmov.current_state) std::cout << "found" << std::endl;
    for(unsigned int i = 0; i < 9; i++)
    {
        action_final.clear();
        action_final.push_back(actions[i]);
        robtemp = robot;
        rr_board_play(board, robtemp, (RRRobotMove)actions[i]);
        if(robotpos_to_movement(robtemp, g).current_state == goalmov.current_state && action_final.size() < best.size()) best = action_final;
        RRRobot robtempj = robtemp;
        for(unsigned int j = 0; j < 9; j++)
        {
	        robtemp = robtempj;
            if(j!= i && robotpos_to_movement(robtemp, g).current_state != 4000000000)
            {
                action_final.erase(action_final.begin()+1, action_final.end());
                action_final.push_back(actions[j]);
                rr_board_play(board, robtemp, (RRRobotMove)actions[j]);
                if(robotpos_to_movement(robtemp, g).current_state == goalmov.current_state && action_final.size() < best.size()) best = action_final;
		        RRRobot robtempk = robtemp;
                for(unsigned int k = 0; k < 9; k++)
                {
		            robtemp = robtempk;
                    if(j != k && i != k && robotpos_to_movement(robtemp, g).current_state != 4000000000)
                    {
                        action_final.erase(action_final.begin()+2, action_final.end());
                        action_final.push_back(actions[k]);
                        rr_board_play(board, robtemp, (RRRobotMove)actions[k]);
                        if((robotpos_to_movement(robtemp, g).current_state == goalmov.current_state) && (action_final.size() < best.size())) best = action_final;
			            RRRobot robtempl = robtemp;
                        for(unsigned int l = 0; l < 9; l++)
                        {
			                robtemp = robtempl;
                            if(l != i && l != j && l != k && robotpos_to_movement(robtemp, g).current_state != 4000000000)
                            {
                                action_final.erase(action_final.begin()+3, action_final.end());
                                action_final.push_back(actions[l]);
                                rr_board_play(board, robtemp, (RRRobotMove)actions[l]);
                                if(robotpos_to_movement(robtemp, g).current_state == goalmov.current_state && action_final.size() < best.size()) best = action_final;
				                RRRobot robtempm = robtemp;
                                for(unsigned int m = 0; m < 9; m++)
                                {
				                    robtemp = robtempm;
                                    if(m != i && m != j && m != k && m != l && robotpos_to_movement(robtemp, g).current_state != 4000000000)
                                    {
                                        action_final.erase(action_final.begin()+4, action_final.end());
                                        action_final.push_back(actions[m]);
                                        rr_board_play(board, robtemp, (RRRobotMove)actions[m]);
                                        if(robotpos_to_movement(robtemp, g).current_state != 4000000000 && robotpos_to_movement(robtemp, g).current_state == goalmov.current_state && action_final.size() < best.size())
                                        {
                                            best = action_final;
                                        }
                                        else{
					                        if(robotpos_to_movement(robtemp, g).current_state != 4000000000){
                                                //std::cout << std::endl;
                                                std::vector<movement> temp_dijkstra;
                                                dijkstra(g, robtemp, temp_dijkstra);
                                                std::vector<RRRobotMove> current_by_dijkstra;
                                                std::string strtmp = get_way_to(goal, temp_dijkstra, g, current_by_dijkstra);
                                                if(current_by_dijkstra.size() < weight_dijkstra)
                                                {
                                                    weight_dijkstra = current_by_dijkstra.size();
                                                    best_by_dijkstra = action_final;
                                                    dijkstra_movs = strtmp;
                                                }
                                            }
                                        }

                                        movement tmp = robotpos_to_movement(robtemp, g);
                                        ways.push_back(tmp);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(best.size() > 5)
    {
        best = best_by_dijkstra;
        nb_mvt = 5 + weight_dijkstra;
    }else{
        nb_mvt = best.size();
        dijkstra_movs = "";
    }
    
    std::vector<RRRobotMove> temp_ac = actions;
    for(int n = 0; n < best.size(); n++)
    {
	    for(int o = 0; o < 9; o++)
	    {
	        if(temp_ac[o] == best[n])
	        {
                temp_ac[o] = (RRRobotMove)7;
	            o = 9;
	        }
	    }
    }
    for(int n = 0; n < 9; n++)
    {
		if(temp_ac[n] != (RRRobotMove)7){
		    best.push_back(temp_ac[n]);
		}
    }
    std::cout << dijkstra_movs << std::endl;
    return best;
}


