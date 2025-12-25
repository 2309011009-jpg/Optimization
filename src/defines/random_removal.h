#include "problem_definitions.h"
#include "alns_definitions.h"
#include <vector>
#include <tuple>
#include "../..//libraries/adaptive-large-neighbourhood-search/src/DestroyMethod.h"


using namespace mlpalns;

struct RandomRemoval: public DestroyMethod<PDPTWT_solution> {
    void destroy_solution(PDPTWT_solution& solution, std::mt19937& mt) {

        // Remove a randomly selected number of requests.
        int percentage = (rand() % 30) + 1;
        int request_amount_to_remove = (solution.problem->requests.size() * percentage) / 100;
        
        for(int i = 0; i < request_amount_to_remove; i++){
        
            // Be aware that this may remove the same request twice. Don't think it needs to be fixed though.
            int request_index = rand() % solution.problem->requests.size();


            solution.remove_request(&solution.problem->requests[request_index]);
            solution.unassigned[request_index] = true;

        }
    }

    std::unique_ptr<DestroyMethod<PDPTWT_solution>> clone() const {
        return std::make_unique<RandomRemoval>(*this);
    }
};