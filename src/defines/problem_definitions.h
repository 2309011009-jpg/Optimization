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
    char type; // o, e, p, d, t
    int earliest_tw;
    int latest_tw;

    Node(
      int id = 0,
      int x = 0,
      int y = 0,
      char type = '!',
      int earliest_tw = 0,
      int latest_tw = 0
    ) : id(id), x(x), y(y), type(type), earliest_tw(earliest_tw), latest_tw(latest_tw)
    {}

};


class Vehicle{
  public:
    int capacity;
    Node* origin;
    Node* destination;

    Vehicle(
      int capacity = 0, 
      Node* origin = nullptr, 
      Node* destination = nullptr
    ) : capacity(capacity), origin(origin), destination(destination)
    {}

};


class Request{
  public:
    int id;
    Node* origin;
    Node* destination;
    int load; // Always positive.
  
    Request(
      int id,
      Node* origin, 
      int load,
      Node* destination = nullptr
    ): id(id), origin(origin), destination(destination), load(load)
    {}

};


class PDPTWT{
  public:

    Node* nodes;
    int node_amount;
    Vehicle* vehicles;
    int vehicle_amount;

    std::vector<Request> requests;
    double* distance_matrix;

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
      double *distances = new double[node_amount * node_amount];

      for(int i = 0; i < node_amount; i++){
        for(int j = 0; j < node_amount; j++){
          double temp1 = nodes[j].x - nodes[i].x;
          double temp2 = nodes[i].y - nodes[j].y;
          double distance = sqrt(temp1 * temp1 + temp2 * temp2);
          distances[i*node_amount + j] = distance;

        }
      }

      distance_matrix = distances;
    }

    inline double get_distance(Node* from_node, Node* to_node) const {
      return distance_matrix[from_node->id* node_amount + to_node->id];

    }


    // Required by ALNS library
    int getInstanceSize() const {return node_amount;} 

    
};


class Stop{
  public:
    Node* node;
    const Request* request;
    bool pickup_or_dropoff; // 1 if pickup, 0 if dropoff

  Stop(
    Node* node,
    const Request* request,
    bool  pickup_or_dropoff
  ) : node(node), request(request), pickup_or_dropoff(pickup_or_dropoff)
  {}

  int get_load_change(){
    if(request != nullptr){
      if(pickup_or_dropoff == true) return request->load;
      return -request->load;
    }

    return 0;
  }
  
  
  bool operator==(Stop stop) const {

    if(
      node == stop.node 
      && 
      request == stop.request 
      && 
      pickup_or_dropoff == stop.pickup_or_dropoff
    ) return true;

    return false;
  }

};


class Route{
  public:

    const PDPTWT* problem;
    std::vector<Stop> stops;

    Route(
      const PDPTWT& problem
    ) : problem(&problem)
    {}


    void insert_stop(int index, Node* node, const Request* request, bool pickup_or_dropoff){
      stops.insert(stops.begin() +  index, Stop(node, request, pickup_or_dropoff));
    }

    void erase_stop(int index){
      stops.erase(stops.begin() + index);
    }

    double calculate_cost() const{
        

      double total = 0;

      for (int i = 0; i < stops.size() - 1; i++) {
        total += problem->get_distance(stops[i].node, stops[i+1].node);
      }
      
      return total;
    }


    double get_arrival_time(const Stop& stop) const{

      if(stops[0] == stop) return 0;

      double current_time = 0.0;
      for(int i = 0; i < stops.size() - 1; i++){

        current_time += problem->get_distance(stops[i].node, stops[i + 1].node);

        if(stops[i + 1] == stop)
          return current_time;

      }

      //
      return 9999999;
    }


    bool check_timing() const {

      double current_time = 0.0;
      for(int i = 0; i < stops.size() - 1; i++){

        current_time += problem->get_distance(stops[i].node, stops[i + 1].node);

        if(
          current_time < stops[i].node->earliest_tw
          ||
          current_time > stops[i].node->latest_tw 
        )
          return false;

      }
      return true;
    }



};


#endif