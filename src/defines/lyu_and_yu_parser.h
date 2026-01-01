#include "problem_definitions.h" 
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#ifndef  PARSE
#define PARSE

PDPTWT parse(std::string path) {

    std::ifstream problem_file(path);
    if (!problem_file.is_open()) {
        std::cerr << "CRITICAL ERROR: Could not find file: " << path << std::endl;
        exit(1);
    }

    int number_of_requests;
    int number_of_vehicles;
    int number_of_transshipment_nodes;
    int capacity_of_vehicles;

    // Skip the first line
    problem_file.ignore(1000, '\n');

    // Read 2nd line metadata
    problem_file >> number_of_requests
                 >> number_of_vehicles
                 >> number_of_transshipment_nodes
                 >> capacity_of_vehicles;

    // Skip 3 lines of headers
    problem_file.ignore(1000, '\n');
    problem_file.ignore(1000, '\n');
    problem_file.ignore(1000, '\n');

    int number_of_nodes = number_of_requests * 2 +
                          number_of_vehicles * 2 +
                          number_of_transshipment_nodes;

    // Allocate raw arrays
    Node* nodes = new Node[number_of_nodes];
    Node* t_nodes = new Node[number_of_transshipment_nodes];
    Vehicle* vehicles = new Vehicle[number_of_vehicles];

    // Temporary storage 
    // We still need this because Node class does not store 'load'
    std::vector<int> node_loads(number_of_nodes); 
    std::vector<Node*> pickup_ptrs;
    std::vector<Node*> delivery_ptrs;
    
    int id = 0;
    std::string node_type;
    int x, y, early_tw, late_tw, load;
    int t_node_idx = 0;
    int v_start_idx = 0;
    int v_end_idx = 0;

    // 1. READ FILE LOOP
    while (problem_file >> node_type >> x >> y >> early_tw >> late_tw >> load) {
        
        if(id >= number_of_nodes) break; 

        // Initialize Node using your new default-arg constructor
        nodes[id] = Node(id, x, y, node_type[0], early_tw, late_tw);
        
        // Store load temporarily in parallel vector
        node_loads[id] = load; 

        char type = node_type[0];

        if (type == 'p') {
            pickup_ptrs.push_back(&nodes[id]);
        } 
        else if (type == 'd') {
            delivery_ptrs.push_back(&nodes[id]);
        }
        else if (type == 't') {
            if (t_node_idx < number_of_transshipment_nodes) {
                t_nodes[t_node_idx] = nodes[id];
                t_node_idx++;
            }
        }
        else if (type == 'o') {
            if (v_start_idx < number_of_vehicles) {
                vehicles[v_start_idx].origin = &nodes[id];
                vehicles[v_start_idx].capacity = capacity_of_vehicles;
                v_start_idx++;
            }
        }
        else if (type == 'e') {
            if (v_end_idx < number_of_vehicles) {
                vehicles[v_end_idx].destination = &nodes[id];
                v_end_idx++;
            }
        }

        id++;
    }

    // 2. CONSTRUCT REQUESTS
    std::vector<Request> requests;
    requests.reserve(pickup_ptrs.size());

    if (pickup_ptrs.size() != delivery_ptrs.size()) {
        std::cerr << "WARNING: Unbalanced Pickups and Deliveries!" << std::endl;
    }

    for (int i = 0; i < pickup_ptrs.size(); i++) {
        int request_load = node_loads[pickup_ptrs[i]->id];
        
        // UPDATED LINE:
        // Request(int id, Node* origin, int load, Node* destination)
        requests.emplace_back(
            i,              // ID
            pickup_ptrs[i],      // Origin
            request_load,        // Load
            delivery_ptrs[i]     // Destination
        );
    }

    // 3. RETURN OBJECT
    return PDPTWT(
        nodes, 
        number_of_nodes, 
        vehicles, 
        number_of_vehicles, 
        requests, 
        t_nodes, 
        number_of_transshipment_nodes
    );
}

#endif