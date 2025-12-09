#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <cmath>
#include <tuple>

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


class PDPTWT{
  public:

    Node* nodes;
    int node_amount;
    Vehicle* vehicles;
    int vehicle_amount;

    int request_amount;
    float* distance_matrix;


    PDPTWT(
      Node* node_arr,
      int node_cnt,
      Vehicle* vehicle_arr,
      int vehicle_cnt,
      int request_cnt
    ){
      nodes = node_arr;
      node_amount = node_cnt;
      vehicles = vehicle_arr;
      vehicle_amount = vehicle_cnt;
      request_amount = request_cnt;

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
    delete[] distance_matrix;
    }


    inline float get_distance(Node* from_node, Node* to_node) const {
      return distance_matrix[from_node->id* node_amount + to_node->id];

    }


    // Required by ALNS library
    int getInstanceSize(){return node_amount;} 

    
};

#endif