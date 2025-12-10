#include "../../libraries/adaptive-large-neighbourhood-search/src/InitialSolutionCreator.h"
#include "problem_definitions.h"
#include "alns_definitions.h"
#include <vector>
#include <tuple>


using namespace mlpalns;
// Popülasyon kullanmak local minimumdan kaçmak için düşünülebilir.
 
PDPTWT_solution Alg2(Request& request, PDPTWT_solution& solution){

float best_cost;
float current_cost;
PDPTWT_solution best_solution = solution;
PDPTWT_solution current_solution = solution;
best_cost = 999999; // +infinity

for(int v = 0; v < solution.problem->vehicle_amount; v++){ // Per Vehicle
  Route* route_of_v = &solution.routes[v];

  for(int i = 0; i <= route_of_v->stops.size(); i++){ // Try Every Combination
    for(int j = i + 1; j <= route_of_v->stops.size(); j++){

      route_of_v->stops.insert(route_of_v->stops.begin() + i, request.origin);
      route_of_v->stops.insert(route_of_v->stops.begin() + j, request.destination);

      current_solution = solution;
      current_cost = current_solution.getCost();

      if(current_cost < best_cost){
        best_cost = current_cost;
        best_solution = current_solution;
      }

      route_of_v->remove_request(request);
    }
  }
}

return best_solution;
}


PDPTWT_solution Alg3(Request& request, PDPTWT_solution& solution){

float best_cost;
float current_cost;
PDPTWT_solution best_solution = solution;
best_cost = 999999; // +infinity

for(int t = 0; t < solution.problem->transshipment_node_amount; t++){
  Node* trans_node = &solution.problem->transshipment_nodes[t];

  for(int v1 = 0; v1 <= solution.problem->vehicle_amount; v1++){
    Vehicle* vehicle1 = &solution.problem->vehicles[v1];

    for(int v2 = 0; v2 <= solution.problem->vehicle_amount; v2++){
      Vehicle* vehicle2 = &solution.problem->vehicles[v2];

      // Make sure they're not the same vehicle.
      if(vehicle1 != vehicle2){ 

        solution.routes[v1].remove_request(request);
        solution.routes[v2].remove_request(request);

        for(int i = 0; i <= solution.routes[v1].stops.size(); i++){
          for(int j = i + 1; j <= solution.routes[v1].stops.size(); j++){

            for(int i2 = 0; i2 <= solution.routes[v1].stops.size(); i2++){
              for(int j2 = i2 + 1; j2 <= solution.routes[v1].stops.size(); j2++){
                
                solution.routes[v1].stops.insert(solution.routes[v1].stops.begin() + i, request.origin);
                solution.routes[v1].stops.insert(solution.routes[v1].stops.begin() + j, trans_node);
                solution.routes[v1].stops.insert(solution.routes[v2].stops.begin() + i2, trans_node);
                solution.routes[v1].stops.insert(solution.routes[v2].stops.begin() + j2, request.destination);

                current_cost = solution.getCost();
                if(current_cost < best_cost){
                  best_cost = current_cost;
                  best_solution = solution;
                }

                solution.routes[v1].stops.erase(solution.routes[v1].stops.begin() + i);
                solution.routes[v1].stops.erase(solution.routes[v1].stops.begin() + j);
                solution.routes[v1].stops.erase(solution.routes[v2].stops.begin() + i2);
                solution.routes[v1].stops.erase(solution.routes[v2].stops.begin() + j2);

              }
            }


          }
        }
        
      }
    }

  }
}

return best_solution;
}


struct PDPTWTSolutionCreator : InitialSolutionCreator<PDPTWT_solution, PDPTWT> {

    PDPTWT_solution create_initial_solution(const PDPTWT& instance, std::mt19937& mt) {
      
      std::vector<Request> singles;
      std::vector<Request> doubles;

      PDPTWT_solution initial_solution(instance);

      float max_dist = 0;
      for(int i = 0; i < instance.node_amount * instance.node_amount; i++){
        if(instance.distance_matrix[i] > max_dist) max_dist = instance.distance_matrix[i];
      }

      for(int i = 0; i < instance.requests.size(); i++){
        Request current_request = instance.requests[i];

        if(instance.get_distance(current_request.origin, current_request.destination) < max_dist / 2){
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
