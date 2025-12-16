#include "defines/lyu_and_yu_parser.h"
#include "defines/problem_definitions.h"
#include "defines/alns_definitions.h"
#include "defines/initial_solution.h"
#include "defines/random_removal.h"
#include "defines/greedy_insertion.h"
#include "defines/dummy_visitor.h"

// ALNS Library Headers
#include "../libraries/adaptive-large-neighbourhood-search/src/PALNS.h"
#include "../libraries/adaptive-large-neighbourhood-search/src/Parameters.h"

#include <iostream>
#include <chrono>

using namespace std::chrono;

using namespace std;
using namespace mlpalns;
int main(){
  // ../data/PDPT/PDPT-R25-K2-T2/PDPT-R25-K2-T2-Q100-2.txt
  PDPTWT problem = parse("../data/PDPT/PDPT-R25-K2-T2/PDPT-R25-K2-T2-Q100-2.txt");
  /* STUCK ON:

  ../data/PDPT/PDPT-R5-K2-T1/PDPT-R5-K2-T1-Q100-2.txt
  
  */

  cout << "Problem Information:" << endl;
  cout << "Node Amount: " << problem.node_amount << endl;
  cout << "Vehicle Amount: " << problem.vehicle_amount << endl;
  cout << "Vehicle Capacity: " << problem.vehicles[0].capacity << endl;

  for(int i = 0; i < problem.node_amount; i++){
    cout << problem.nodes[i].id << " --- " << problem.nodes[i].type << endl;
  }

  auto alns = PALNS<PDPTWT, PDPTWT_solution>{problem};

  PDPTWTSolutionCreator creator;
  std::mt19937 rng(42);

  PDPTWT_solution initial_sol = creator.create_initial_solution(problem, rng);
  initial_sol.print_solution();
  
  RandomRemoval random_removal;
  alns.add_destroy_method(random_removal, "Remove Randomly");

  insert_w_transfer insert_w_transfer;
  alns.add_repair_method(insert_w_transfer, "Insert With Transfer");

  mlpalns::Parameters params("../libraries/adaptive-large-neighbourhood-search/Params.json");

  // 1. Explicitly define the variable as the BASE class type (AlgorithmVisitor)
std::unique_ptr<mlpalns::AlgorithmVisitor<PDPTWT_solution>> visitor = 
    std::make_unique<DummyVisitor<PDPTWT_solution>>();

// 2. Now pass it (it matches the function signature perfectly)
alns.set_algorithm_visitor(visitor);

  cout << endl << "SOLUTION STARTING" << endl;
  auto start = high_resolution_clock::now();
  PDPTWT_solution best_sol = alns.go(initial_sol, 16, params);
  auto stop = high_resolution_clock::now();
  cout << "--- Finished ---" << endl;
  cout << "Best Cost: " << best_sol.getCost() << endl;
  auto duration = duration_cast<microseconds>(stop - start);
  cout << "Time Spent: " << duration.count() << " Microseconds" << endl;
  // Print details
  best_sol.print_solution();
  return 0;
}