#include "problem_definitions.h"
#include <fstream>
#include <iostream>

PDPTWT* parse(std::string path){

  std::ifstream problem_file(path);
  if (!problem_file.is_open()) {
    std::cerr << "CRITICAL ERROR: Could not find file: " << path << std::endl;
    std::cerr << "Current working directory may not be what you expect." << std::endl;
    exit(1); 
    }


  int number_of_requests;
  int number_of_vehicles;
  int number_of_transshipment_nodes;
  int capacity_of_vehicles;

  problem_file.ignore(1000, '\n');

  // Read 2nd line.
  problem_file 
  >> number_of_requests
  >> number_of_vehicles
  >> number_of_transshipment_nodes
  >> capacity_of_vehicles;
    
  problem_file.ignore(1000, '\n');
  problem_file.ignore(1000, '\n');
  problem_file.ignore(1000, '\n');

  int number_of_nodes = number_of_requests * 2 + 
        number_of_vehicles * 2 + 
        number_of_transshipment_nodes;

  Node* nodes = new Node[number_of_nodes];

  Vehicle* vehicles = new Vehicle[number_of_vehicles];

  int id = 0;
  std::string node_type;
  int x;
  int y;
  int a;
  int b;
  int load;

  while(problem_file >> node_type >> x >> y >> a >> b >> load){

    nodes[id] = Node(id, x, y, node_type[0], load, a, b);
    
    id++;
  }


  int v_start_idx = 0;
  int v_end_idx = 0;
  for(int i = 0; i < number_of_nodes; i++) {
      if (nodes[i].node_type == 'o') {
          if (v_start_idx < number_of_vehicles) {
              vehicles[v_start_idx].origin = &nodes[i];
              vehicles[v_start_idx].capacity = capacity_of_vehicles;
              v_start_idx++;
          }
      }
      else if (nodes[i].node_type == 'e') {
          if (v_end_idx < number_of_vehicles) {
              vehicles[v_end_idx].destination = &nodes[i];
              v_end_idx++;
            }
        }
    }

    Node** pickups = new Node*[number_of_requests];
    Node** deliveries = new Node*[number_of_requests];
    int p_count = 0;
    int d_count = 0;

    for(int i = 0; i < number_of_nodes; i++) {
        if (nodes[i].node_type == 'p') {
            pickups[p_count++] = &nodes[i];
        } 
        else if (nodes[i].node_type == 'd') {
            deliveries[d_count++] = &nodes[i];
        }
    }

    // Link them (assuming Index 0 of P matches Index 0 of D)
    for(int i = 0; i < number_of_requests; i++) {
        if (i < p_count && i < d_count) {
            pickups[i]->add_pair(deliveries[i]);
        }
    }

    // Clean up temp helper arrays (not the nodes themselves!)
    delete[] pickups;
    delete[] deliveries;

  return new PDPTWT(nodes, number_of_nodes, vehicles, number_of_vehicles, number_of_requests);
}
