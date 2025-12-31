#include "problem_definitions.h"
#include "alns_definitions.h"
#include <cstddef>
#include <tuple>
#include <vector>
#include <limits>
#include <algorithm>
#include <utility>

#include "../../libraries/adaptive-large-neighbourhood-search/src/RepairMethod.h"


// Regret-K
#define K 10

using namespace mlpalns;

struct regret_k_insertion : public RepairMethod<PDPTWT_solution> {
    void repair_solution(PDPTWT_solution& sol, std::mt19937& mt) {

      vector<const Request*> removed_requests;

      // Get all removed requests.
      for(int i = 0; i < sol.problem->requests.size(); i++)
        if(sol.unassigned[i] == true) removed_requests.push_back(&sol.problem->requests[i]);
      
      // While not empty
      while(!removed_requests.empty()){

        vector<vector<pair<tuple<int, int, int>, double>>> all_requests;


        for(int r = 0; r < removed_requests.size(); r++){
          const Request* request = removed_requests[r];

          // All feasible solutions and their costs for this request.
          vector<pair<tuple<int, int, int>, double>> costs;

          // Find ever feasible solution.
          for(int v = 0; v < sol.problem->vehicle_amount; v++){
            Route* route = &sol.routes[v];
            
            for(int i = 0; i <= route->stops.size(); i++){
                for(int j = i + 1; j <= route->stops.size(); j++){
                    
                  route->insert_stop(i, request->origin, request, true);
                  route->insert_stop(j, request->destination, request, false);

                  if(sol.is_feasible()){
                    pair<tuple<int, int, int>, double> cost = {{i,j,v},sol.getCost()};
                    costs.push_back(cost);
                  }

                  route->erase_stop(j);
                  route->erase_stop(i);
              }
            }
          }

          // Sort from lowest cost to highest cost.
          std::sort(costs.begin(), costs.end(), 
          [](const auto& a, const auto& b) {
              return a.second < b.second;
          }
          );
          
          all_requests.push_back(costs);
        }

        const Request* request = NULL;

        double highest_regret_k = 0;
        tuple <int, int, int> reconstructor;
        for(int i = 0; i < all_requests.size(); i++){
          if(all_requests[i].empty()) continue;

          double best_cost = all_requests[i].front().second;
          double regret_k = 0;

          for(int k = 0; k <= K && k < all_requests[i].size() - 1; k++)
            regret_k += all_requests[i][k + 1].second - best_cost;
          
          // If higher regret_k, save.
          if(regret_k > highest_regret_k){
            highest_regret_k = regret_k;
            reconstructor = all_requests[i].front().first;
            request = removed_requests[i];
          }

        }

        // If none of the requests can be placed.
        if(request == NULL) return;


        // Insert Solution
        Route* route = &sol.routes[std::get<2>(reconstructor)];
        route->insert_stop(std::get<0>(reconstructor), request->origin, request, true);
        route->insert_stop(std::get<1>(reconstructor), request->destination, request, false);

        sol.unassigned[request->id] = false;

        // Remove it from the removed_requests vector.
        removed_requests.erase(
            std::remove(removed_requests.begin(), removed_requests.end(), request),
            removed_requests.end()
        );

      }


    }

    std::unique_ptr<RepairMethod<PDPTWT_solution>> clone() const {
        return std::make_unique<regret_k_insertion>(*this);
    }
};