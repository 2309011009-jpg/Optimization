#include "defines/lyu_and_yu_parser.h"
#include "defines/problem_definitions.h"
#include "defines/alns_definitions.h"
#include "../libraries/adaptive-large-neighbourhood-search/src/PALNS.h"

#include <iostream>
using namespace std;

int main(){

  PDPTWT* problem = parse("/home/inthezone/Codes/Optimization/input_data/example1.txt");

  cout << "Problem Information:" << endl;
  cout << "Node Amount: " << problem->node_amount << endl;
  cout << "Vehicle Amount: " << problem->vehicle_amount << endl;
  cout << "Vehicle Capacity: " << problem->vehicles[0].capacity << endl;

  Node* request_pickups[problem->request_amount];
  int request_cnt = 0;
  for(int i = 0; i < problem->node_amount; i++){
    if(problem->nodes[i].node_type == 'p'){
      request_pickups[request_cnt] = &problem->nodes[i];
      request_cnt++;
    }
  }

  PDPTWT_solution solution(problem);

  return 0;
}