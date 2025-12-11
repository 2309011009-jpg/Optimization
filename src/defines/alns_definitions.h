#ifndef ALNSDEFINITIONS_H
#define ALNSDEFINITIONS_H

#include "problem_definitions.h"
#include <vector>

class PDPTWT_solution{
  public:
    const PDPTWT* problem;
    std::vector<Route> routes;
    // Route n corresponds to vehicle n

    // Array of bools, signifying if the problem->request[i] is assigned or not.
    // true == unassigned, false == assigned.
    bool* unassigned;

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

      unassigned = new bool[problem->requests.size()];
      for(int i = 0; i < problem->requests.size(); i++){
        unassigned[i] = true;
      }
    }

    float getCost(){
      float total_cost = 0;
      for(int i = 0; i < routes.size(); i++){
        total_cost += routes[i].calculate_cost();
      }

      return total_cost;
    }

    bool is_feasible(){
      if(_timing_check() == false) return false;
      if(_structure_check() == false) return false;
      if(_precedence_check() == false) return false;
      if(_capacity_check() == false) return false;
      if(_request_check() == false) return false;

      return true;
    }


  private:

    bool _timing_check(){ // Check if timing is satisfied.
      for(int i = 0; i < routes.size(); i++){
        if(routes[i]._check_timing() == false) 
          return false;
      }

      return true;
    }


    bool _structure_check(){ // Check if vehicle starts from it's depot and ends in it's destination.
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

    bool _precedence_check(){ // Check if pickup was visited before delivery.
      for(int i = 0; i < routes.size(); i++){
        if(routes[i]._check_precedence() == false) 
          return false;
      }

      return true;

    }


    bool _capacity_check(){ // Check if any vehicle's load is bigger than it's capacity at any point.
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


  bool _request_check(){ // Check if all requests have been assigned.
    for(int i = 0; i < problem->requests.size(); i++){
      if(unassigned[i] == true) return false;
    }

    return true;
  }


};

#endif