#include "defines/lyu_and_yu_parser.h"
#include "defines/problem_definitions.h"
#include "defines/alns_definitions.h"
#include "defines/initial_solution.h"
#include "../libraries/adaptive-large-neighbourhood-search/src/PALNS.h"

#include <iostream>
using namespace std;

int main(){

  PDPTWT* problem = parse("/home/inthezone/Codes/Optimization/input_data/example1.txt");

  cout << "Problem Information:" << endl;
  cout << "Node Amount: " << problem->node_amount << endl;
  cout << "Vehicle Amount: " << problem->vehicle_amount << endl;
  cout << "Vehicle Capacity: " << problem->vehicles[0].capacity << endl;

  //PDPTWT_solution solution(problem);

  return 0;
}