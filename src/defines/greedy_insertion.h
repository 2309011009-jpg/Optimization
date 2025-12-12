// TODO: Greedy Insertion
// Best Insertion with Transfer.

#include "problem_definitions.h"
#include "alns_definitions.h"
#include <vector>
#include <tuple>
#include "../..//libraries/adaptive-large-neighbourhood-search/src/RepairMethod.h"

using namespace mlpalns;

struct insert_w_transfer : public RepairMethod<PDPTWT_solution> {
    void repair_solution(PDPTWT_solution& sol, std::mt19937& mt) {

      // For each unassigned request.
      for(int r = 0; r < sol.problem->requests.size(); r++){
        bool assigned = false;
        if(sol.unassigned[r]){
          const Request* request = &sol.problem->requests[r];

          float best_without_transfer = std::numeric_limits<float>::max();
          PDPTWT_solution best_without_transfer_sol = sol;
          float best_with_transfer = std::numeric_limits<float>::max();
          PDPTWT_solution best_with_transfer_sol = sol;

          PDPTWT_solution current_solution = sol;
          float current_cost;

          // Find the best solution without transfers.

          for(int v = 0; v < sol.problem->vehicle_amount; v++){ // Per Vehicle
          Route* route_of_v = &current_solution.routes[v];

          for(int i = 1; i <= route_of_v->stops.size(); i++){ // Try Every Combination
            for(int j = i + 1; j <= route_of_v->stops.size(); j++){

              route_of_v->stops.insert(route_of_v->stops.begin() + i, request->origin);
              route_of_v->stops.insert(route_of_v->stops.begin() + j, request->destination);

              current_cost = current_solution.getCost();

              if(current_cost < best_without_transfer && current_solution.is_feasible()){
                best_without_transfer = current_cost;
                best_without_transfer_sol = current_solution;
                assigned = true;
              }

              route_of_v->stops.erase(route_of_v->stops.begin() + j);
              route_of_v->stops.erase(route_of_v->stops.begin() + i);
            }
          }
          
          // Find the best solution WITH transfers.

          
          for(int t = 0; t < sol.problem->transshipment_node_amount; t++){ // Per Transshipment Node.
            Node* trans_node = &sol.problem->transshipment_nodes[t];

            for(int v1 = 0; v1 < sol.problem->vehicle_amount; v1++){ // Per Vehicle Combination.
              Vehicle* vehicle1 = &sol.problem->vehicles[v1];

              for(int v2 = 0; v2 < sol.problem->vehicle_amount; v2++){
                Vehicle* vehicle2 = &sol.problem->vehicles[v2];

                // Make sure they're not the same vehicle.
                if(vehicle1 != vehicle2){ 
                  for(int i = 1; i <= sol.routes[v1].stops.size(); i++){
                    for(int j = i + 1; j <= sol.routes[v1].stops.size(); j++){

                      for(int i2 = 1; i2 <= sol.routes[v2].stops.size(); i2++){
                        for(int j2 = i2 + 1; j2 <= sol.routes[v2].stops.size(); j2++){


                          int v1_action_index = 0;
                          for(int k = 0; k < j; k++){
                              // Count how many transfer nodes appear before our insertion point 'j'
                              if(current_solution.routes[v1].stops[k]->node_type == 't') v1_action_index++;
                          }


                          int v2_action_index = 0;
                          for(int k = 0; k < i2; k++){
                              if(current_solution.routes[v2].stops[k]->node_type == 't') v2_action_index++;
                          }


                          // TODO: Make this more readable.
                          current_solution.routes[v1].stops.insert(current_solution.routes[v1].stops.begin() + i, request->origin);
                          current_solution.routes[v1].stops.insert(current_solution.routes[v1].stops.begin() + j, trans_node);
                          current_solution.routes[v1].transshipment_actions.insert(
                              current_solution.routes[v1].transshipment_actions.begin() + v1_action_index, 
                              std::make_tuple(request->origin, trans_node, 0)
                          );

                          current_solution.routes[v2].stops.insert(current_solution.routes[v2].stops.begin() + i2, trans_node);
                          current_solution.routes[v2].stops.insert(current_solution.routes[v2].stops.begin() + j2, request->destination);
                          current_solution.routes[v2].transshipment_actions.insert(
                              current_solution.routes[v2].transshipment_actions.begin() + v2_action_index, 
                              std::make_tuple(request->origin, trans_node, 1)
                          );


                          current_cost = current_solution.getCost();

                          if(current_cost < best_with_transfer && current_solution.is_feasible()){
                            best_with_transfer = current_cost;
                            best_with_transfer_sol = current_solution;
                            assigned = true;
                          }

                          current_solution.routes[v1].stops.erase(current_solution.routes[v1].stops.begin() + j);
                          current_solution.routes[v1].stops.erase(current_solution.routes[v1].stops.begin() + i);

                          current_solution.routes[v2].stops.erase(current_solution.routes[v2].stops.begin() + j2);
                          current_solution.routes[v2].stops.erase(current_solution.routes[v2].stops.begin() + i2);

                          current_solution.routes[v1].transshipment_actions.erase(current_solution.routes[v1].transshipment_actions.begin() + v1_action_index);
                          current_solution.routes[v2].transshipment_actions.erase(current_solution.routes[v2].transshipment_actions.begin() + v2_action_index);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }

        if(best_with_transfer < best_without_transfer)
          sol = best_with_transfer_sol;
        else sol = best_without_transfer_sol;

        if(assigned)
          sol.unassigned[r] = false;
      }
    }
  }

    std::unique_ptr<RepairMethod<PDPTWT_solution>> clone() const {
        return std::make_unique<insert_w_transfer>(*this);
    }
};