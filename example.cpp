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


    /* On commence par lancer dijkstra qui, pour un robot start donné va calculer tous les plus courts
     * chemins jusqu'au autre cases et stocker ces résultat dans le vector res*/
    std::vector<movement> res;
    dijkstra(g, robot, res);

    /* On affiche le contenu du tableau récupéré */
    for(unsigned int i = 0; i < res.size(); i++)
    {
        std::cout << "state  " << res[i].current_state << " can go to " << res[i].arrival_state << " taking " << res[i].weight_from_start << " moves from start" << std::endl;
    }


    /* On éxécute ensuite get_way_to qui, pour un robot goal donné et le tableau resultat issus de
     * dijkstra, renvoie un string détaillant son parcour optimal et remplit un vector d'action de
     * robot correspondant */
    std::string r;
    std::vector<RRRobotMove> actions_for_robot;
    r = get_way_to(robot_goal, res, g, actions_for_robot);
    std::cout << "\n\n**** Chemin le plus court via dijkstra ****" << std::endl;
    std::cout << "\nPour aller du robot de départ, en " << robot.line << "," << robot.column << " regardant vers " << robot.status << "\njusqu'au robot en " << robot_goal.line << "," << robot_goal.column << " regardant vers " << robot_goal.status << "\nIl faut : " << r << "\n\n" << std::endl;


    /* Ici, on crer un pool de mouvement possible pour le joueur artificiel. artificial_player calcule 
     * le chemin le plus court en utilisant jusqu'a 5 de ces cartes pour atteindre goal. Si goal n'est
     * pas atteint, on applique dijkstra pour trouver la suite de mouvements nous rapprochant le plus
     * de l'objectif. actions est le pool d'actions; finalac est le pool resultat; nb_mvt contient 
     * le nombre de mouvements nécessaires*/
    std::cout << "**** Chemin le plus court pour un set de mouvement (joueur artificiel) ****" << std::endl;


    std::vector<RRRobotMove> actions;
    std::vector<RRRobotMove> finalac;
    

    //set aléatoire de 9 mouvements
    srand(time(NULL));
    for(unsigned int i = 0; i < 9; i++)
    {
        RRRobotMove t = (RRRobotMove)(rand()%7);
        actions.push_back(t);
    }

    int nb_mvt;
    std::string dijkstra_movs;
    dijkstra_movs = "";
    finalac = artificial_player(board, g, robot, robot_goal, actions, nb_mvt, dijkstra_movs);
    std::cout << "\nOn peut rejoindre l'objectif en " << nb_mvt << " mouvements."  << std::endl; 
    std::cout << "Le pool d'actions résultant est le suivant : " << std::endl;
    for(unsigned int i = 0; i < finalac.size(); i++)
    {
        std::cout << finalac[i] << ", ";
    }
    if(nb_mvt > 5) std::cout << "\nInatteignable directement.\n\nLes mouvements suivants doivent être éxécuté\naprès les 5 premiers du pool pour atteindre l'objectif : " << dijkstra_movs << "\n" << std::endl;
    std::cout << std::endl;

    rr_board_destroy(board) ;

    return 0 ;
}
