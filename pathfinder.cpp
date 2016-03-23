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
                start.weight = 1;
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
                    break;
            }
            res = g.graph_vector[tempid*7];
            return res;

        }
    }
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

void dijkstra(graph & g, RRRobot & robot_start, RRRobot & robot_goal, std::vector<movement> & res)
{
    unsigned int ds = 0;
    movement current_mov = robotpos_to_movement(robot_start, g);
    current_mov.weight_from_start = ds;
    std::cout << "Dijkstra" << std::endl;
    std::vector<movement> queue;
    std::set<unsigned int> done;
    for (std::vector<movement>::iterator it = g.graph_vector.begin(); it != g.graph_vector.end(); ++it)
    {
        it->weight_from_start = -1;
    }
    queue.push_back(current_mov);
    done.insert(current_mov.current_state);
    while(!queue.empty())
    {
        std::cout << "d loopw" << std::endl;
        //get the first vetor's element
        current_mov = queue.front();
        //then remove it
        res.push_back(current_mov);
        queue.erase(queue.begin());

        for(unsigned int i = 0; i < 7; ++i)
        {
            unsigned int weight_from_prec = g.graph_vector[current_mov.current_state*7+i].weight;
            unsigned int compare_weight = weight_from_prec + g.graph_vector[current_mov.current_state*7+i].weight_from_start;
            if(g.graph_vector[current_mov.current_state*7+i].arrival_state != 4000000000)
            {
                if(g.graph_vector[current_mov.arrival_state*7].weight_from_start > compare_weight)
                {
                    for(unsigned int j = 0; j < 7; ++j)
                    {
                        g.graph_vector[current_mov.arrival_state*7+j].weight_from_start = compare_weight;
                    }

                }


                if(done.find(g.graph_vector[current_mov.current_state*7+i].arrival_state) == done.end())
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
                            std::cout << "d loop f2 if: " << std::endl;
                            queue[j].weight_from_start = g.graph_vector[g.graph_vector[current_mov.current_state*7+i].arrival_state*7].weight_from_start;
                            j = queue.size();
                        }
                    }  
                    //std::cout << "d loop f2 if: " << std::endl;
                }
                std::cout << "d loop f1 " << queue.size() << std::endl;
                std::sort(queue.begin(), queue.end(), sort_mov);
            }
        }

    }

}


/*  movement shortest_path(graph & g, RRRobot & robot_start, RRRobot & robot_goal)
{
    //TODO: add unsigned int weight_from_start to movement
    //set it to max int
    //add done movement vector
    std::vector<movement> done_mvt;
    movement init = robotpos_to_movement(robot_start, g);
    unsigned int dist_to_start = 0;
    init.weight_from_start = 0;
    std::priority_queue<movement, std::vector<movement>, movement_compare> queue;
    queue.push(init);
    //updt done vector
    std::cout << "init " << init.current_state << ", " << init.arrival_state << std::endl;
    while(!queue.empty())
    {
        movement temp_mov = queue.top();
        queue.pop();
       // if(temp_mov.arrival_state != 4000000000)
        //{
            std::cout << temp_mov.current_state << " yyyy " << queue.size() << "size" << temp_mov.arrival_state << std::endl;
            if(temp_mov.current_state == robotpos_to_movement(robot_goal, g).current_state) return temp_mov;
            dist_to_start = g.graph_vector[temp_mov.current_state*7].weight_from_start;
            for(unsigned int i = 0; i < 7; i++)
            {
                unsigned int edge_weight = g.graph_vector[temp_mov.current_state*7+i].weight;
                if(g.graph_vector[temp_mov.current_state*7+i].weight_from_start > (dist_to_start+edge_weight))
                {
                    g.graph_vector[temp_mov.current_state*7+i].weight_from_start = dist_to_start + edge_weight;

                }
                if(g.graph_vector[temp_mov.current_state*7+i].arrival_state != 4000000000 && !mov_in_vector(done_mvt, g.graph_vector[g.graph_vector[temp_mov.current_state*7+i].arrival_state*7]))
                {
                    queue.push(g.graph_vector[g.graph_vector[temp_mov.current_state*7+i].arrival_state*7]);
                    //std::cout << "add  " << queue.top().current_state << std::endl;
                    done_mvt.push_back(g.graph_vector[g.graph_vector[temp_mov.current_state*7+i].arrival_state*7]);
                }
            }

        //}
    }
    return init;
}*/



