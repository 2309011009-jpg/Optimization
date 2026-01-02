#if defined(__INCLUDE_LEVEL__) && __INCLUDE_LEVEL__ == 0
#error "Do not compile this header directly; include it from a .cpp file and use the Makefile or build.sh to build the project."
#endif

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
    std::vector<char> unassigned;



    PDPTWT_solution(){}

    PDPTWT_solution(
      const PDPTWT& problem_instance
    ){
      problem = &problem_instance;

      for(int i = 0; i < problem->vehicle_amount; i++){

        // Initiate a route per vehicle with only the origin & destination.
        Stop origin(problem->vehicles[i].origin, nullptr, true);
        Stop destination(problem->vehicles[i].destination, nullptr, true);
        
        Route new_route = Route(problem_instance);

        new_route.stops.push_back(origin);
        new_route.stops.push_back(destination);

        routes.push_back(new_route);
      }

      for(size_t i = 0; i < problem->requests.size(); i++){
        unassigned.push_back(true);
      }

    }

    float getCost() const{
      float total_cost = 0;
      for(int i = 0; i < routes.size(); i++){
        total_cost += routes[i].calculate_cost();
      }

      double penalty = 0;

      for(size_t i = 0; i < problem->requests.size(); i++){
        if(unassigned[i]) penalty += 10000;
      }

      if(is_feasible() == false)
        penalty += 1000000;

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

    // TODO : Erase-Remove maybe?
    void remove_request(const Request* request) {
    
      // Check all stops and remove any associated stops.
      for(size_t i = 0; i < routes.size(); i++){
        for(size_t j = 0; j < routes[i].stops.size();){
          
          if(routes[i].stops[j].request == request){
            routes[i].stops.erase(routes[i].stops.begin() + j);
          }
          else j++;
        }
      }
      unassigned[request->id] = true;
    }


    void print_solution(){
      for(size_t i = 0; i < static_cast<size_t>(problem->vehicle_amount); i++){
      cout << endl << "Route of Vehicle " << i << " : ";

      for(size_t j = 0; j < routes[i].stops.size(); j++){
        cout << routes[i].stops[j].node->id << " - "; 
      }

      cout << "COST: " << routes[i].calculate_cost() << endl;

    }
 
    cout << "TRANSFERS:" << endl;

    for(size_t i = 0; i < static_cast<size_t>(problem->vehicle_amount); i++){
      for(size_t j = 0; j < routes[i].stops.size(); j++){
        if(routes[i].stops[j].node->type == 't'){
          if(routes[i].stops[j].pickup_or_dropoff){
            cout << "Vehicle " << i << " Pickup " << routes[i].stops[j].request->id << " at " << routes[i].stops[j].node->id << " at time point: " << routes[i].get_arrival_time(routes[i].stops[j]) << endl;
          }
          else cout << "Vehicle " << i << " Dropoff " << routes[i].stops[j].request->id << " at " << routes[i].stops[j].node->id << " at time point: " << routes[i].get_arrival_time(routes[i].stops[j]) << endl;
        }
      }
    }

    cout << "TOTAL COST: " << getCost() << endl;

    cout << "Solution Feasibility: " << is_feasible() << endl;

    }

  private:

    bool _timing_check() const{ // Check if timing is satisfied.
      for(size_t i = 0; i < routes.size(); i++){
        if(routes[i].check_timing() == false) 
          return false;
      }

      return true;
    }


    bool _structure_check() const{ // Check if vehicle starts from it's depot and ends in it's destination.
      for(size_t i = 0; i < routes.size(); i++){
        if(
          routes[i].stops.front().node != problem->vehicles[i].origin
          ||
          routes[i].stops.back().node != problem->vehicles[i].destination
      ){
        return false;
      }
      }

      return true;
    }

    bool _precedence_check() const { 

      for(size_t i = 0; i < routes.size(); i++){
        auto current_stops = routes[i].stops;

        for(size_t j = 0; j < current_stops.size(); j++){

          // If dropoff, an earlier pickup must've occured.
          if(current_stops[j].pickup_or_dropoff == false){
            bool pickup_found = false;

            // Check all previous stops.
            for(size_t k = 0; k < j; k++){

              // Find where it was picked up
              if(current_stops[k].request == current_stops[j].request){
                pickup_found = true;

                // If transshipment, find it's dropoff counterpart.
                if(current_stops[k].node->type == 't'){
                  double arrival_at_t = routes[i].get_arrival_time(current_stops[k]);

                  // Check all other route's stops.
                  for(size_t r = 0; r < routes.size(); r++){
                    auto temp = routes[r].stops;
                    for(size_t s = 0; s < temp.size(); s++){

                      // If the stop is for the same request
                      if(
                        temp[s].node == current_stops[k].node
                        &&
                        temp[s].request == current_stops[k].request
                        &&
                        temp[s].pickup_or_dropoff == false
                      )
                      {
                        // If dropoff counterpart arrived later than pickup.
                        if(routes[r].get_arrival_time(temp[s]) > arrival_at_t) return false;
                      }
                    }
                  }
                }
              }
            }

            if(pickup_found == false) return false;
          }

        }
      }

      return true;
    }


    bool _capacity_check() const{ // Check if any vehicle's load is bigger than it's capacity at any point.
      for(int i = 0; i < routes.size(); i++){
        int current_load = 0;
        int capacity = problem->vehicles[i].capacity;

        auto current_stops = routes[i].stops;
        for(size_t j = 0; j < current_stops.size(); j++){
          current_load += current_stops[j].get_load_change();

          if(current_load > capacity)
            return false;

        }
      }

      return true;
    }


    bool _request_check() const{ // Check if all requests have been assigned.
      for(size_t i = 0; i < problem->requests.size(); i++){
        if(unassigned[i] == true) return false;
      }

      return true;
    }



};

#endif