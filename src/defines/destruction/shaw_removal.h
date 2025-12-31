#include "../problem_definitions.h"
#include "../alns_definitions.h"
#include <vector>
#include "../../..//libraries/adaptive-large-neighbourhood-search/src/DestroyMethod.h"


using namespace mlpalns;

inline double calculate_relativity(const PDPTWT* problem, const Request* R, const Request* S){
  double distance = problem->get_distance(R->origin, S->origin) + problem->get_distance(R->destination, S->destination);
  double time     = 
      abs(R->origin->earliest_tw - S->origin->earliest_tw) 
      + 
      abs(R->destination->earliest_tw - S->destination->earliest_tw);
  int load     = abs(R->load + S->load);

  double distance_weight = 1;
  double time_weight     = 5;
  double load_weight     = 1;

  return distance * distance_weight + time * time_weight + load * load_weight;
}


struct ShawRemoval: public DestroyMethod<PDPTWT_solution> {
    void destroy_solution(PDPTWT_solution& solution, std::mt19937& mt) {

      const std::vector<Request>* requests = &solution.problem->requests;

      // Select a request randomly as seed. 
      int percentage = (rand() % 30) + 1;
      int request_amount_to_remove = ((requests->size() * percentage) / 100);
      const Request* seed_request = &(*requests)[rand() % requests->size()];

      std::vector<const Request*> removed_requests;
      removed_requests.reserve(request_amount_to_remove);

      // Remove seed request and put it into the vector.
      solution.remove_request(seed_request);
      removed_requests.push_back(seed_request);

      // -1 is for the seed request already removed.
      while(removed_requests.size() < request_amount_to_remove){

        const Request* referance_request = removed_requests[rand() % removed_requests.size()];

        // Store all relativity scores.
        std::vector<pair<const Request*, double>> scores;
        scores.reserve(requests->size() - removed_requests.size());
        
        for(int i = 0; i < requests->size(); i++){

          // Skip if request has already been removed.
          if(solution.unassigned[i] == true) continue;
          else{
            // Score relativity of request.
            scores.push_back({
                  &(*requests)[i], 
                  calculate_relativity(solution.problem, referance_request, &(*requests)[i])
                });
          }
        }

        if(scores.empty()) break;

      

        // Sort scores from from smallest to biggest.
        std::sort(scores.begin(), scores.end(), [](auto &left, auto &right) {
            return left.second < right.second;
        });

        // This variable determines the range which the smallest requests can be chosen from.
        int range_limit = std::max(1, (int)scores.size() / 5);


        const Request* request_to_remove = std::get<0>(scores.at((rand() % range_limit)));

        solution.remove_request(request_to_remove);
        removed_requests.push_back(request_to_remove);

      }
    }

    std::unique_ptr<DestroyMethod<PDPTWT_solution>> clone() const {
        return std::make_unique<ShawRemoval>(*this);
    }
};