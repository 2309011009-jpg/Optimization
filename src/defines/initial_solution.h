#include "../../libraries/adaptive-large-neighbourhood-search/src/InitialSolutionCreator.h"
#include "problem_definitions.h"
#include "alns_definitions.h"

using namespace mlpalns;

struct PDPTWTSolutionCreator : InitialSolutionCreator<PDPTWT_solution, PDPTWT> {

    PDPTWT_solution create_initial_solution(const PDPTWT instance, std::mt19937& mt) {
      
      return NULL; // RETURN INITIAL SOLUTION.
    }
};
 