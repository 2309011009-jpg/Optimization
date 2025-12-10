#include "defines/lyu_and_yu_parser.h"
#include "defines/problem_definitions.h"
#include "defines/alns_definitions.h"
#include "defines/initial_solution.h"
#include "../libraries/adaptive-large-neighbourhood-search/src/PALNS.h"

#include <iostream>
using namespace std;

int main(){

  PDPTWT problem = parse("/home/inthezone/Codes/Optimization/input_data/example3.txt");

  cout << "Problem Information:" << endl;
  cout << "Node Amount: " << problem.node_amount << endl;
  cout << "Vehicle Amount: " << problem.vehicle_amount << endl;
  cout << "Vehicle Capacity: " << problem.vehicles[0].capacity << endl;

  for(int i = 0; i < problem.node_amount; i++){
    cout << problem.nodes[i].id << " --- " << problem.nodes[i].node_type << endl;
  }

  auto alns = PALNS<PDPTWT, PDPTWT_solution>{problem};

  PDPTWTSolutionCreator creator;
  std::mt19937 rng(42);

  PDPTWT_solution initial_sol = creator.create_initial_solution(problem, rng);

  for(int i = 0; i < problem.vehicle_amount; i++){
    cout << endl << "Route of Vehicle " << i << " : ";

    for(int j = 0; j < initial_sol.routes[i].stops.size(); j++){
      cout << initial_sol.routes[i].stops[j]->id << " - "; 
    }

    cout << "COST: " << initial_sol.routes[i].calculate_cost();

  }

  cout << "TOTAL COST: " << initial_sol.getCost();


  return 0;
}