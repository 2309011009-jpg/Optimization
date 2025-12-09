#include "defines/lyu_and_yu_parser.h"
#include "defines/problem_definitions.h"
#include "defines/alns_definitions.h"
#include "/home/inthezone/Codes/Optimization/libraries/adaptive-large-neighbourhood-search/src/PALNS.h"

#include <iostream>
using namespace std;

int main(){

  PDPTWT* problem = parse("/home/inthezone/Codes/Optimization/input_data/example1.txt");

  cout << "Problem Information:" << endl;
  cout << "Node Amount: " << problem->node_amount << endl;
  cout << "Vehicle Amount: " << problem->vehicle_amount << endl;
  cout << "Vehicle Capacity: " << problem->vehicles[0].capacity << endl;

  for(int i = 0; i < problem->node_amount; i++){
    cout << problem->nodes[i].id << " --- " << problem->nodes[i].node_type << endl;
  }

  cout << "Node index " << problem->nodes[2].id << " is paired with " << problem->nodes[2].pair_node->id << endl;


  Node* nodes = problem->nodes;

  Node* nodes_to_visit[6] = {&nodes[4], &nodes[0], &nodes[1], &nodes[2], &nodes[3], &nodes[5]};

  Route new_route = Route(problem, problem->vehicles, nodes_to_visit, 6);

  for(int i = 0; i < new_route.stop_amount; i++){
    cout << "Visiting Node Index: " << new_route.stops[i]->id  << " for: " << new_route.stops[i]->node_type << endl;
  }

  cout << "Cost of Route: " << new_route.cost << endl;
  cout << "Is Route Feasible: " << new_route.is_feasible() << endl;

  return 0;
}