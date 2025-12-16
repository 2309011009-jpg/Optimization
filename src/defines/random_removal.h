#include "problem_definitions.h"
#include "alns_definitions.h"
#include <vector>
#include <tuple>
#include "../..//libraries/adaptive-large-neighbourhood-search/src/DestroyMethod.h"


using namespace mlpalns;

struct RandomRemoval: public DestroyMethod<PDPTWT_solution> {
    void destroy_solution(PDPTWT_solution& solution, std::mt19937& mt) {

        int request_index = rand() % solution.problem->requests.size();

        solution.remove_request(&solution.problem->requests[request_index]);
        solution.unassigned[request_index] = true;

    }

    std::unique_ptr<DestroyMethod<PDPTWT_solution>> clone() const {
        return std::make_unique<RandomRemoval>(*this);
    }
};