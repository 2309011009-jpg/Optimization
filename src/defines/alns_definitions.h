#ifndef ALNSDEFINITIONS_H
#define ALNSDEFINITIONS_H

#include "problem_definitions.h"
#include <vector>

class PDPTWT_solution{
  public:
    const PDPTWT* problem;
    std::vector<Route> routes;
    // Route n corresponds to vehicle n

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
    }

    float getCost(){
      float total_cost = 0;
      for(int i = 0; i < routes.size(); i++){
        total_cost += routes[i].calculate_cost();
      }

      return total_cost;
    }

};

#endif