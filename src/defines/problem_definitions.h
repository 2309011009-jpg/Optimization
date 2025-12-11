#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <cmath>
#include <tuple>
#include <vector>

class Route;
class PDPTWT;
class Vehicle;
class Node;
class Request;

class Node{
  public:
    int id;
    int x;
    int y;
    char node_type; // o, e, p, d, t
    int load;
    int earliest_tw;
    int latest_tw;
    Node* pair_node;

    Node(
      int id_t = 0,
      int horizontal = 0,
      int vertical = 0,
      char type = '!',
      int load_amount = 0,
      int earliest = 0,
      int latest = 0,
      Node* pair = nullptr
    ){
      id = id_t;
      x = horizontal;
      y = vertical;
      node_type = type;
      load = load_amount;
      earliest_tw = earliest;
      latest_tw = latest;
      pair_node = pair;
    }

    void add_pair(Node* pair){
      pair_node = pair;
      pair_node->pair_node = this;
    }
};


class Vehicle{
  public:
    int capacity;
    Node* origin;
    Node* destination;

    Vehicle(
      int max = 0, 
      Node* origin_node = nullptr, 
      Node* destination_node = nullptr
    ){
      capacity = max;
      origin = origin_node;
      destination = destination_node;
    }
};


class Request{
  public:
    Node* origin;
    Node* destination;
  
    Request(
      Node* origin_node = nullptr, 
      Node* destination_node = nullptr
    ){
      origin = origin_node;
      destination = destination_node;
    }
};


class PDPTWT{
  public:

    Node* nodes;
    int node_amount;
    Vehicle* vehicles;
    int vehicle_amount;

    std::vector<Request> requests;
    float* distance_matrix;

    Node* transshipment_nodes;
    int transshipment_node_amount;

    PDPTWT(
      Node* node_arr,
      int node_cnt,
      Vehicle* vehicle_arr,
      int vehicle_cnt,
      std::vector<Request> request_arr,
      Node* t_points,
      int t_amount
    ){
      nodes = node_arr;
      node_amount = node_cnt;
      vehicles = vehicle_arr;
      vehicle_amount = vehicle_cnt;
      requests = request_arr;
      transshipment_nodes = t_points;
      transshipment_node_amount = t_amount;

      // Access as distances[i*node_amount+j]
      float *distances = new float[node_amount * node_amount];

      for(int i = 0; i < node_amount; i++){
        for(int j = 0; j < node_amount; j++){
          float temp1 = nodes[j].x - nodes[i].x;
          float temp2 = nodes[i].y - nodes[j].y;
          float distance = sqrt(temp1 * temp1 + temp2 * temp2);
          distances[i*node_amount + j] = distance;

        }
      }

      distance_matrix = distances;
    }
    
    ~PDPTWT() {
    //delete[] distance_matrix; TODO: Double free here for unknown reasons
    }


    inline float get_distance(Node* from_node, Node* to_node) const {
      return distance_matrix[from_node->id* node_amount + to_node->id];

    }


    // Required by ALNS library
    int getInstanceSize(){return node_amount;} 

    
};


class Route{
  public:

    const PDPTWT* problem;
    std::vector<Node*> stops;
    std::vector<std::tuple<Node*, Node*, bool>> transshipment_actions;    

    Route(
      const PDPTWT& problem_inst,
      std::vector<Node*> node_arr,
      std::vector<std::tuple<Node*, Node*, bool>> tr_actions = {}
    ){
      problem = &problem_inst;
      stops = node_arr;
      transshipment_actions = tr_actions;
    }

    /*bool is_feasible(){

      if(_check_structure() == false){return false;}
      if(_check_capacity() == false){return false;}
      if(_check_timing() == false){return false;}
      if(_check_precedence() == false){return false;}

      return true;
    }*/


    void remove_request(Request& request){
      for(int i = 0; i < stops.size(); i++){
        if(stops[i] == request.origin || stops[i] == request.destination){
          stops.erase(stops.begin() + i);
          //i--; // adjust index since vector shrank
        }
      }
    }

    float calculate_cost(){
        float total = 0;

        for (int i = 0; i < stops.size() - 1; i++) {
            total += problem->get_distance(stops[i], stops[i+1]);
        }
        
        return total;
    }


      bool _check_timing(){
        float current_time = 0;
        // Added safety check
        if(stops.size() == 0) return true;

        for (int i = 0; i < stops.size() - 1; i++) {
            current_time += problem->get_distance(stops[i], stops[i+1]);

            if(current_time < stops[i+1]->earliest_tw || current_time > stops[i+1]->latest_tw)
              return false;
        }
        
        return true;
      }


      bool _check_precedence(){
        for(int i = 0; i < stops.size(); i++){
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
            
            for(int j = 0; j < transshipment_actions.size(); j++){
              if(
                std::get<1>(transshipment_actions[j]) == pickup &&
                std::get<2>(transshipment_actions[j]) == true
              ){
                for(int k = 0; k < i; k++){
                  if(std::get<0>(transshipment_actions[j]) == stops[k]) {
                    transshipment_was_visited = true;
                    break;
                  }
                }
                
                if (transshipment_was_visited) break;
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
          for(int i = 0; i < transshipment_actions.size(); i++){
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

#endif