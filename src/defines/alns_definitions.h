#include "problem_definitions.h"

class Route{
  public:

    PDPTWT* problem;
    Vehicle* vehicle;
    Node** stops; // 2* because we want an array of pointers to the PDPTW's nodes.
    int stop_amount;
    // Node 1 = Transshipment Node.
    // Node 2 = Origin of the request.
    // if bool = 1, then pickup. if bool = 0, then dropoff.
    std::tuple<Node*, Node*, bool>* transshipment_actions;    
    int transshipment_amount;

    float cost;
    

    Route(
      PDPTWT* problem_inst,
      Vehicle* vehicle_t,
      Node** node_arr,
      int stop_cnt,
      std::tuple<Node*, Node*, bool>* tr_actions = nullptr,
      int transshipment_cnt = 0
    ){
      problem = problem_inst;
      vehicle = vehicle_t;
      stops = node_arr;
      stop_amount = stop_cnt;
      transshipment_actions = tr_actions;
      transshipment_amount = transshipment_cnt;

      cost = calculate_cost();
    }

    bool is_feasible(){

      if(_check_structure() == false){return false;}
      if(_check_capacity() == false){return false;}
      if(_check_timing() == false){return false;}
      if(_check_precedence() == false){return false;}

      return true;
    }


    private:
    
      float calculate_cost(){
        float total = 0;
        for (int i = 0; i < stop_amount - 1; i++) {
            total += problem->get_distance(stops[i], stops[i+1]);
        }
        
        return total;
      }


      bool _check_structure(){
        if(stop_amount < 2){return false;}
        if(stops[0] != vehicle->origin){return false;}
        if(stops[stop_amount - 1] != vehicle->destination){return false;}

        return true;
      }


      bool _check_capacity(){
        int current_load = 0;
        for(int i = 0; i < stop_amount; i++){
          current_load += _get_load_change(stops[i]);

          if(current_load > vehicle->capacity)
            return false;
        }

        return true;
      }


      bool _check_timing(){
        float current_time = 0;
        for (int i = 0; i < stop_amount - 1; i++) {
            current_time += problem->get_distance(stops[i], stops[i+1]);

            if(current_time < stops[i+1]->earliest_tw || current_time > stops[i+1]->latest_tw)
              return false;
        }
        
        return true;
      }


      bool _check_precedence(){
        for(int i = 0; i < stop_amount; i++){
          if(stops[i]->node_type == 'd'){
            Node* pickup = stops[i]->pair_node;
            bool pickup_was_visited = false;

            // Check if Pickup node was visited previously.
            for(int j = 0; j < i; j++){
              if(stops[j] == pickup){
                pickup_was_visited = true;
                break;
              }
            }

            if(pickup_was_visited){continue;}

            // Check if the load has been picked up from a transshipment node previously.
            bool transshipment_was_visited = false;
            for(int j = 0; j < transshipment_amount; j++){
              if(
                std::get<1>(transshipment_actions[j]) == pickup &&
                std::get<2>(transshipment_actions[j]) == true
              ){
                for(int k = 0; k < i; k++){
                  if(std::get<0>(transshipment_actions[j]) == stops[k])
                    transshipment_was_visited = true;
                    break;
                }

                break;
              }
            }

              if(!transshipment_was_visited){return false;}
          }
        }

        return true;
      }


      // Get the value of load change visiting a certain node will cause.
      int _get_load_change(Node* stop){
        if(stop->node_type != 't'){return stop->load;}

        else{

          for(int i = 0; i < transshipment_amount; i++){
            if(std::get<0>(transshipment_actions[i]) == stop){

              if(std::get<2>(transshipment_actions[i]))
                return std::get<1>(transshipment_actions[i])->load;

              return std::get<1>(transshipment_actions[i])->pair_node->load;
            }
          }
        }
        
        return -99999;
      }

};

class PDPTWT_solution{
  public:
    PDPTWT* problem;
    Route* routes;

    PDPTWT_solution(
      PDPTWT* problem_instance,
      Route* route_arr = nullptr
    ){
      problem = problem_instance;
      routes = route_arr;
    }


    float getCost(){
      float total_cost = 0;
      for(int i = 0; i < problem->vehicle_amount; i++){
        total_cost += routes[i].cost;
      }

      return total_cost;
    }


};