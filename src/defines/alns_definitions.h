#ifndef ALNSDEFINITIONS_H
#define ALNSDEFINITIONS_H

#include "problem_definitions.h"
#include <vector>

class PDPTWT_solution{
  public:
    PDPTWT* problem;
    Vehicle** vehicles; // An array of pointers

    PDPTWT_solution(
      PDPTWT* problem_instance
    ){
      problem = problem_instance;

      for(int i = 0; i < problem->vehicle_amount; i++){
        // Initiate a route per vehicle with only the origin & destination.
        std::vector<Node*> route_stops;
        route_stops.push_back(problem->vehicles[i].origin);
        route_stops.push_back(problem->vehicles[i].destination);

        vehicles[i]->route = new Route(problem, route_stops); 
      }
    }

    ~PDPTWT_solution(){
      for (int i = 0; problem->vehicle_amount; i++){
        delete vehicles[i]->route;
      }
    }


    float getCost(){
      float total_cost = 0;
      for(int i = 0; i < problem->vehicle_amount; i++){
        total_cost += vehicles[i]->route->calculate_cost();
      }

      return total_cost;
    }


};

#endif