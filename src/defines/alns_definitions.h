#ifndef ALNSDEFINITIONS_H
#define ALNSDEFINITIONS_H

#include "problem_definitions.h"
#include <vector>
#include <algorithm>

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


    void remove_request(const Request& request){

      Route* current_route;
      for(int i = 0; i < routes.size(); i++){
        current_route = &routes[i];

        // Check all stops and remove the origin and destination of the request.
        for(int j = 1; j < current_route->stops.size() - 1;){

          if(current_route->stops[j] == request.origin || current_route->stops[j] == request.destination){
            current_route->stops.erase(current_route->stops.begin() + j);
          }
          else j++;
        }

        // Remove any related transshipment node that doesn't serve an alternative purpose.
        for(int j = 0; j < current_route->transshipment_actions.size(); j++){
          // This transshipment node was used for this request, though we need to check if it's in use by another.
          if(std::get<0>(current_route->transshipment_actions[j]) == request.origin){

            bool another_user = false;

            for(int k = 0; k < current_route->transshipment_actions.size(); k++){

              // There's another request being picked up from & delivered to this transshipment node.
              if(std::get<1>(current_route->transshipment_actions[j]) == std::get<0>(current_route->transshipment_actions[k]))
                if(std::get<0>(current_route->transshipment_actions[k]) != request.origin){
                  another_user = true;
                  break;
                }
            }


            // If no other request going through the transshipment node is found:
            if(another_user == false){ // Remove the Transshipment Node
              current_route->stops.erase(
                std::remove(current_route->stops.begin(),
                current_route->stops.end(),
                std::get<1>(current_route->transshipment_actions[j])
              ), current_route->stops.end());

              // Remove the transshipment action.
              current_route->transshipment_actions.erase(current_route->transshipment_actions.begin() + j);
            }
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

      return true;

    }


    bool _capacity_check() const{ // Check if any vehicle's load is bigger than it's capacity at any point.
      int current_load;
        for(int i = 0; i < routes.size(); i++){
          current_load = 0;
          for(int j = 0; j < routes[i].stops.size(); j ++){
            current_load += routes[i]._get_load_change(routes[i].stops[j]);
          
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