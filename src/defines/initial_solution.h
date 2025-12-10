#include "../../libraries/adaptive-large-neighbourhood-search/src/InitialSolutionCreator.h"
#include "problem_definitions.h"
#include "alns_definitions.h"
#include <vector>
#include <tuple>


using namespace mlpalns;
// Popülasyon kullanmak local minmumdan kaçmak için düşünülebilir.
 
void Alg2(Request& request, PDPTWT_solution& solution){

  for(int v = 0; v <= solution.problem->vehicle_amount; v++){ // Per Car.
    Route* current_route = solution.problem->vehicles[v].route;
    float best_cost = 9999999; // Infinitely high number.
    Route best_route(solution.problem, current_route->stops);

    for(int i = 0; i <= current_route->stops.size(); i++){ // Try every position.
      for(int j = 0; j < current_route->stops.size() && j > i + 1; j++){

        current_route->stops.insert(current_route->stops.begin() + i, request.origin);
        current_route->stops.insert(current_route->stops.begin() + j, request.destination);

        if(current_route->calculate_cost() < best_cost){
          best_cost = current_route->calculate_cost();
          best_route.stops = current_route->stops;
        }

        current_route->remove_request(request);
      }
    }

    // Update route with the best one found.
    current_route->stops = best_route.stops;
  }
}


void Alg3(Request& request, PDPTWT_solution& solution){


  float best_cost = 999999;


  // Per Transshipment
  for(int t = 0; t < solution.problem->transshipment_node_amount; t++){
    Node* transshipment_node = &solution.problem->transshipment_nodes[t];

    // Per Vehicle 1
    for(int v1 = 0; v1 < solution.problem->vehicle_amount; v1++){
      Vehicle vehicle1 = solution.problem->vehicles[v1]; 

      // Per Vehicle 2
      for(int v2 = 0; v2 < solution.problem->vehicle_amount; v2++){
        Vehicle vehicle2 = solution.problem->vehicles[v2];
        
        if(&vehicle1 != &vehicle2){ // Make sure they're not the same vehicle.

          // Remove transshipment node if it's already in route.
          for(int a = 0; a < vehicle1.route->stops.size(); a++){
            if(vehicle1.route->stops[a] == transshipment_node){
              vehicle1.route->stops.erase(vehicle1.route->stops.begin() + a);
            }
          }

          for(int a = 0; a < vehicle2.route->stops.size(); a++){
            if(vehicle2.route->stops[a] == transshipment_node){
              vehicle2.route->stops.erase(vehicle2.route->stops.begin() + a);
            }
          }

          for(int i1 = 0; i1 <= vehicle1.route->stops.size(); i1++){
            for(int j1 = 0; j1 <= vehicle2.route->stops.size() && j1 >= i1+1; j1++){

              for(int i2 = 0; i2 <= vehicle2.route->stops.size(); i2++){
                for(int j2 = 0; j2 <= vehicle2.route->stops.size() && j2 >= i2 + 1; j2++){

                  // Pickup to Transshipment Node.
                  vehicle1.route->stops.insert(vehicle1.route->stops.begin() + i1, request.origin);
                  vehicle1.route->stops.insert(vehicle1.route->stops.begin() + j1, transshipment_node);
                  int v1_transshipment_cnt = vehicle1.route->transshipment_actions.size();
                  vehicle1.route->transshipment_actions[v1_transshipment_cnt] = std::make_tuple(request.origin, transshipment_node, 0);

                  // Transshipment to Drop-off Node.
                  vehicle2.route->stops.insert(vehicle2.route->stops.begin() + i2, transshipment_node);
                  vehicle2.route->stops.insert(vehicle2.route->stops.begin() + j2, request.destination);
                  int v2_transshipment_cnt = vehicle2.route->transshipment_actions.size();
                  vehicle2.route->transshipment_actions[v2_transshipment_cnt] = std::make_tuple(request.origin, transshipment_node, 1);
                  
                  float total_cost = vehicle1.route->calculate_cost() + vehicle2.route->calculate_cost();

                  if(total_cost < best_cost){


                    
                  }
                  
                }

              }

            }
          }


        }
    }
    }
  }


}


struct PDPTWTSolutionCreator : InitialSolutionCreator<PDPTWT_solution, PDPTWT> {

    PDPTWT_solution create_initial_solution(PDPTWT instance, std::mt19937& mt) {
      
      std::vector<Request> singles;
      std::vector<Request> doubles;

      PDPTWT_solution initial_solution(&instance);

      int max_dist = 0;
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
        Alg2(singles[i], initial_solution);
      }

      for (int i = 0; i < doubles.size(); i++){
        Alg3(doubles[i], initial_solution);
      }

      return initial_solution;
    }
};
