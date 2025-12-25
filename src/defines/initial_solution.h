#include "../../libraries/adaptive-large-neighbourhood-search/src/InitialSolutionCreator.h"
#include "problem_definitions.h"
#include "alns_definitions.h"
#include <vector>
#include <tuple>


using namespace mlpalns;
// Popülasyon kullanmak local minimumdan kaçmak için düşünülebilir.
// Request'leri shuffle'lamak ve oluşan sonuçlardan en iyisini seçmek bir seçenek.

// IMPORTANT: The results obtained by the initial solutions are not neccessarily feasible.
// IMPORTANT: I made a modification to the algorithm in the paper, i now starts from 1 rather than 0.
// This is to stop the origin-destination nodes from being displaced.
// IMPORTANT: Another modification is that if there are no transshipment nodes in the problem, all will be placed in singles.


PDPTWT_solution Alg2(const Request* request, PDPTWT_solution& solution){

  double best_cost;
  double current_cost;
  PDPTWT_solution best_solution = solution;
  PDPTWT_solution current_solution = solution;
  best_cost = std::numeric_limits<double>::max(); // +infinity

  for(int v = 0; v < solution.routes.size(); v++){ // Per Route
    Route* route_of_v = &solution.routes[v];

    for(int i = 1; i <= route_of_v->stops.size(); i++){ // Try Every Combination
      for(int j = i + 1; j <= route_of_v->stops.size(); j++){

        route_of_v->insert_stop(i, request->origin, request, true);
        route_of_v->insert_stop(j, request->destination, request, false);

        current_solution = solution;
        current_cost = current_solution.getCost();

        if(current_cost < best_cost){
          best_cost = current_cost;
          best_solution = current_solution;

        }

        route_of_v->erase_stop(j);
        route_of_v->erase_stop(i);
      }
    }
  }

  best_solution.unassigned[request->id] = false;
  return best_solution;
  }


PDPTWT_solution Alg3(const Request* request, PDPTWT_solution& solution){

double best_cost;
double current_cost;
PDPTWT_solution best_solution = solution;
best_cost = std::numeric_limits<double>::max(); // +infinity

for(int t = 0; t < solution.problem->transshipment_node_amount; t++){ // Per Transshipment Node
  Node* trans_node = &solution.problem->transshipment_nodes[t];

  for(int v1 = 0; v1 < solution.problem->vehicle_amount; v1++){ // Per Vehicle Combination
    Vehicle* vehicle1 = &solution.problem->vehicles[v1];
    Route* route1 = &solution.routes[v1];

    for(int v2 = 0; v2 < solution.problem->vehicle_amount; v2++){
      Vehicle* vehicle2 = &solution.problem->vehicles[v2];
      Route* route2 = &solution.routes[v2];

      // Make sure they're not the same vehicle.
      if(vehicle1 != vehicle2){ 

        for(int i = 1; i <= route1->stops.size(); i++){
          for(int j = i + 1; j <= route1->stops.size(); j++){

            for(int i2 = 1; i2 <= route2->stops.size(); i2++){
              for(int j2 = i2 + 1; j2 <= route2->stops.size(); j2++){

                route1->insert_stop(i, request->origin, request, 1);
                route1->insert_stop(j, trans_node, request, 0);

                route2->insert_stop(i2, trans_node, request, 1);
                route2->insert_stop(j2, request->destination, request, 0);


                current_cost = solution.getCost();
                if(current_cost < best_cost){
                  best_cost = current_cost;
                  best_solution = solution;
                }

                route1->erase_stop(j);
                route1->erase_stop(i);

                route2->erase_stop(j2);
                route2->erase_stop(i2);
              }
            }
          }
        }
        
      }
    }

  }
}

best_solution.unassigned[request->id] = false;
return best_solution;
}


struct PDPTWTSolutionCreator : InitialSolutionCreator<PDPTWT_solution, PDPTWT> {

    PDPTWT_solution create_initial_solution(const PDPTWT& instance, std::mt19937& mt) {
      
      std::vector<const Request*> singles;
      std::vector<const Request*> doubles;

      PDPTWT_solution initial_solution(instance);

      double max_dist = 0;
      for(int i = 0; i < instance.node_amount * instance.node_amount; i++){
        if(instance.distance_matrix[i] > max_dist) max_dist = instance.distance_matrix[i];
      }

      for(int i = 0; i < instance.requests.size(); i++){
        const Request* current_request = &instance.requests[i];

        // If the distance inbetween the origin and destination of the request is too much, use transshipment.
        if(instance.get_distance(current_request->origin, current_request->destination) < max_dist / 2  || instance.transshipment_node_amount == 0){
          singles.push_back(current_request);
        }
        else{
          doubles.push_back(current_request);
        }
      }

      for (int i = 0; i < singles.size(); i++){
        initial_solution = Alg2(singles[i], initial_solution);
      }

      for (int i = 0; i < doubles.size(); i++){
        initial_solution = Alg3(doubles[i], initial_solution);
      }

      return initial_solution;
    }
};
