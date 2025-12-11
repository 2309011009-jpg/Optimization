/* 
TODO: Random Removal

This heuristic randomly selects
r% of the requests to be removed from the current
solution.
*/

#include "problem_definitions.h"
#include "alns_definitions.h"
#include <vector>
#include <tuple>

PDPTWT_solution random_removal(PDPTWT_solution& solution){
  
  int request_index = rand() % solution.problem->requests.size();


}