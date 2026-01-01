#include "../problem_definitions.h"
#include "../alns_definitions.h"
#include <vector>
#include "../../../libraries/adaptive-large-neighbourhood-search/src/DestroyMethod.h"
#include "../configurable.h"

using namespace mlpalns;

struct RandomRemoval: public DestroyMethod<PDPTWT_solution> , public Configurable{

    RandomRemoval(){
        add_parameter("Max Removal Percentage", 50.0, 10.0, 80.0, 5.0);
    }

    void destroy_solution(PDPTWT_solution& solution, std::mt19937& mt) {

        int max_removal_percentage = get_int_param("Max Removal Percentage");

        // Remove a randomly selected number of requests.
        int percentage = (rand() % max_removal_percentage) + 1;
        int request_amount_to_remove = (solution.problem->requests.size() * percentage) / 100;
        
        for(int i = 0; i < request_amount_to_remove; i++){
        
            // Be aware that this may remove the same request twice. Don't think it needs to be fixed though.
            int request_index = rand() % solution.problem->requests.size();

            solution.remove_request(&solution.problem->requests[request_index]);

        }
    }

    std::unique_ptr<DestroyMethod<PDPTWT_solution>> clone() const {
        return std::make_unique<RandomRemoval>(*this);
    }
};