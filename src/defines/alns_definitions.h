#ifndef ALNSDEFINITIONS_H
#define ALNSDEFINITIONS_H

#include "problem_definitions.h"
#include <vector>
#include <algorithm>
#include <map>
using namespace std;

class PDPTWT_solution{
  public:
    const PDPTWT* problem;
    std::vector<Route> routes;
    // Route n corresponds to vehicle n

    // Array of bools, signifying if the problem->request[i] is assigned or not.
    // true == unassigned, false == assigned.
    std::vector<bool> unassigned;

    PDPTWT_solution(
      const PDPTWT& problem_instance
    ){
      problem = &problem_instance;

      for(int i = 0; i < problem->vehicle_amount; i++){
        // Initiate a route per vehicle with only the origin & destination.
        std::vector<Node*> route_stops;
        route_stops.push_back(problem->vehicles[i].origin);
        route_stops.push_back(problem->vehicles[i].destination);

        routes.push_back(Route(problem_instance, route_stops)); 
      }

      for(int i = 0; i < problem->requests.size(); i++){
        unassigned.push_back(true);
      }
    }

    float getCost() const{
      float total_cost = 0;
      for(int i = 0; i < routes.size(); i++){
        total_cost += routes[i].calculate_cost();
      }

      float penalty = 0;

      for(int i = 0; i < problem->requests.size(); i++){
        if(unassigned[i]) penalty += 10000;
      }

      if(is_feasible() == false)
        penalty += 10000;

      return total_cost + penalty;
    }

    bool is_feasible() const{
      if(_timing_check() == false) return false;
      if(_structure_check() == false) return false;
      if(_precedence_check() == false) return false;
      if(_capacity_check() == false) return false;
      //if(_request_check() == false) return false;

      return true;
    }


    void remove_request(const Request& request){ // TODO: AI generated, fix asap.

  for(int i = 0; i < routes.size(); i++){
    Route* current_route = &routes[i];

    // 1. Remove Origin and Destination from stops (Standard logic)
    // Iterate backwards to avoid index skipping issues
    for(int j = current_route->stops.size() - 1; j >= 0; j--){
      if(current_route->stops[j] == request.origin || current_route->stops[j] == request.destination){
        current_route->stops.erase(current_route->stops.begin() + j);
      }
    }

    // 2. Remove Transshipment Actions AND their corresponding Stops
    // Iterate backwards to safely erase
    for(int j = current_route->transshipment_actions.size() - 1; j >= 0; j--){
      
      // Check if this action belongs to the request being removed
      // (Index 0 of tuple is always the Request Origin / ID)
      if(std::get<0>(current_route->transshipment_actions[j]) == request.origin){

        // Identify the node used in this action
        Node* trans_node = std::get<1>(current_route->transshipment_actions[j]);

        // A. Remove EXACTLY ONE instance of this node from the stops.
        // We search for the first occurrence. Since your logic duplicates pointers (55 - 55),
        // removing the first matching pointer is perfectly fine.
        auto it = std::find(current_route->stops.begin(), current_route->stops.end(), trans_node);
        
        if (it != current_route->stops.end()) {
            current_route->stops.erase(it);
        }

        // B. Remove the action itself
        current_route->transshipment_actions.erase(current_route->transshipment_actions.begin() + j);
      }
    }
  }
}


    void print_solution(){
      for(int i = 0; i < problem->vehicle_amount; i++){
      cout << endl << "Route of Vehicle " << i << " : ";

      for(int j = 0; j < routes[i].stops.size(); j++){
        cout << routes[i].stops[j]->id << " - "; 
      }

      cout << "COST: " << routes[i].calculate_cost() << endl;

    }

    cout << "TOTAL COST: " << getCost() << endl;

    cout << "Solution Feasibility: " << is_feasible() << endl;

    }

  private:

    bool _timing_check() const{ // Check if timing is satisfied.
      for(int i = 0; i < routes.size(); i++){
        if(routes[i]._check_timing() == false) 
          return false;
      }

      return true;
    }


    bool _structure_check() const{ // Check if vehicle starts from it's depot and ends in it's destination.
      for(int i = 0; i < routes.size(); i++){
        if(
          routes[i].stops.front() != problem->vehicles[i].origin
          ||
          routes[i].stops.back() != problem->vehicles[i].destination
      ){
        return false;
      }
      }

      return true;
    }

bool _precedence_check() const{ // Check if pickup was visited before delivery.
      for(int i = 0; i < routes.size(); i++){
        if(routes[i]._check_precedence() == false) 
          return false;
      }

      // Check if request picked up from transshipment node was delivered there before arrival.
      for(int i = 0; i < routes.size(); i++){ // For every route.
        for(int j = 0; j < routes[i].transshipment_actions.size(); j++){ // For every action
          if(std::get<2>(routes[i].transshipment_actions[j])){ // If action was a pickup
            // Check if another route dropped it before.
            int index_of_visit = 0;
            
            // ERROR FIX 1: The 'for' loop body needs brackets
            for(int k = 0; k <= j; k++){
              // Find the transshipmen node's index.
              if(std::get<1>(routes[i].transshipment_actions[k]) == std::get<1>(routes[i].transshipment_actions[j])){
                index_of_visit++;
              } // Correct closing bracket for if(k)
            } // Correct closing bracket for for(k)

            // NOTE: Assuming routes[i]._check_timing is a function that returns float
            // and has been defined to accept (Node*, int index).
            float arrival_time = routes[i]._check_timing(std::get<1>(routes[i].transshipment_actions[j]), index_of_visit - 1); // Pass 0-based index
            
            // tuple<Node*, Node*, bool> dropoff_action; // Removed tuple declaration here to avoid syntax error
            float drop_time = -1.0;
            bool drop_found = false;

            for(int v = 0; v < routes.size(); v++){ // Check every other route.
              if(v == i) continue;

              int drop_action_index = -1;
              
              for(int k = 0; k < routes[v].transshipment_actions.size(); k++){ // Find the associated dropoff action.
                if(
                  std::get<0>(routes[v].transshipment_actions[k]) == std::get<0>(routes[i].transshipment_actions[j]) &&
                  std::get<2>(routes[v].transshipment_actions[k]) == 0
                ){
                  // We don't need to store the action, just its index is enough to proceed
                  drop_action_index = k;
                  drop_found = true;
                  break;
                }
              }

              if(drop_found){
                // Calculate drop time using the found index
                int drop_visit_index = 0;
                Node* drop_node = std::get<1>(routes[v].transshipment_actions[drop_action_index]);
                
                // ERROR FIX 2: Loop to count drop visits
                for(int k = 0; k <= drop_action_index; k++){
                    // Find the transshipmen node's index.
                    if(std::get<1>(routes[v].transshipment_actions[k]) == drop_node){
                      drop_visit_index++;
                    }
                } // Correct closing bracket

                // NOTE: Assuming routes[v]._check_timing is available
                drop_time = routes[v]._check_timing(drop_node, drop_visit_index - 1);
                
                // The structure for finding drop time is complex; this break is necessary to prevent recalculation.
                break;
              }
            } // Correct closing bracket for for(v)

            // ERROR FIX 3: Bracket added around the final check
            if(drop_found){
                if(arrival_time < drop_time){
                    return false; 
                }
            } // Correct closing bracket
          } // Correct closing bracket for if(action was a pickup)
        } // Correct closing bracket for for(j)
      } // Correct closing bracket for for(i)

      return true;

    }


    bool _capacity_check() const{ // Check if any vehicle's load is bigger than it's capacity at any point.
      int current_load;
        for(int i = 0; i < routes.size(); i++){
          current_load = 0;
          std::map<Node*, int> transshipment_node_count;
          for(int j = 0; j < routes[i].stops.size(); j ++){
            if(routes[i].stops[j]->node_type == 't'){
              if(transshipment_node_count.find(routes[i].stops[j]) != transshipment_node_count.end()){
                transshipment_node_count.at(routes[i].stops[j])++;}
              else{
                transshipment_node_count.insert({routes[i].stops[j], 0});
              }

              current_load += routes[i]._get_load_change(routes[i].stops[j], transshipment_node_count[routes[i].stops[j]]);
            }
            else{
              current_load += routes[i]._get_load_change(routes[i].stops[j]);
            }
            
          if(current_load > problem->vehicles[0].capacity)
            return false;
          }
    }
    return true;
  }


  bool _request_check() const{ // Check if all requests have been assigned.
    for(int i = 0; i < problem->requests.size(); i++){
      if(unassigned[i] == true) return false;
    }

    return true;
  }



};

#endif