#include "problem_definitions.h"
#include "alns_definitions.h"
#include <vector>
#include <limits>
#include "../../libraries/adaptive-large-neighbourhood-search/src/RepairMethod.h"

using namespace mlpalns;

struct insert_w_transfer : public RepairMethod<PDPTWT_solution> {
    void repair_solution(PDPTWT_solution& sol, std::mt19937& mt) {

      for(int r = 0; r < sol.problem->requests.size(); r++){
        if(!sol.unassigned[r]) continue;

        const Request* request = &sol.problem->requests[r];

        double best_cost = std::numeric_limits<double>::max();
        
        // 0: Direct, 1: Transfer (Leg1 -> Leg2), 2: Transfer (Leg2 -> Leg1)
        int best_method = -1; 
        
        // We store the moves exactly as they were found to replay them safely
        struct MoveStep { int v; int i; int j; };
        MoveStep move1 = {-1, -1, -1};
        MoveStep move2 = {-1, -1, -1};
        int best_t_node = -1;

        // -------------------------------------------------------
        // 1. Best Insertion Without Transfer
        // -------------------------------------------------------
        for(int v = 0; v < sol.problem->vehicle_amount; v++){
            Route* route = &sol.routes[v];
            // FIX: Loop condition must be <= size(), not size() + 1
            for(int i = 0; i <= route->stops.size(); i++){
                // After insert at i, size increases by 1. 
                // j can go up to new_size (which is old_size + 1).
                // But route->stops.size() updates dynamically! 
                // So j <= route->stops.size() is correct. 
                // Removing the "+ 1" prevents the crash.
                for(int j = i + 1; j <= route->stops.size(); j++){
                    
                    route->insert_stop(i, request->origin, request, true);
                    route->insert_stop(j, request->destination, request, false);

                    if(sol.is_feasible()){
                        double cost = sol.getCost();
                        if(cost < best_cost){
                            best_cost = cost;
                            best_method = 0;
                            move1 = {v, i, j};
                        }
                    }
                    route->erase_stop(j);
                    route->erase_stop(i);
                }
            }
        }

        // -------------------------------------------------------
        // 2. Best Insertion With Transfer (Mitrovic-Minic & Laporte)
        // -------------------------------------------------------
        for(int t = 0; t < sol.problem->transshipment_node_amount; t++){
            Node* trans_node = &sol.problem->transshipment_nodes[t];

            // =================================================
            // METHOD (i): Best Leg 1 -> Then Best Leg 2
            // =================================================
            
            // Step A: Find Best Leg 1 (Origin -> Transfer)
            double best_leg1_cost = std::numeric_limits<double>::max();
            MoveStep best_l1 = {-1, -1, -1};

            for(int v = 0; v < sol.problem->vehicle_amount; v++){
                Route* route = &sol.routes[v];
                double start_cost = route->calculate_cost();

                for(int i = 0; i <= route->stops.size(); i++){
                    for(int j = i + 1; j <= route->stops.size(); j++){ // FIX: removed +1
                        
                        route->insert_stop(i, request->origin, request, true);
                        route->insert_stop(j, trans_node, request, false);

                        if(true){ 
                            double delta = route->calculate_cost() - start_cost;
                            if(delta < best_leg1_cost){
                                best_leg1_cost = delta;
                                best_l1 = {v, i, j};
                            }
                        }
                        route->erase_stop(j);
                        route->erase_stop(i);
                    }
                }
            }

            // Step B: Fix Best Leg 1, Search for Best Leg 2
            if(best_l1.v != -1){
                // Apply Leg 1 temporarily
                sol.routes[best_l1.v].insert_stop(best_l1.i, request->origin, request, true);
                sol.routes[best_l1.v].insert_stop(best_l1.j, trans_node, request, false);

                for(int v = 0; v < sol.problem->vehicle_amount; v++){
                    Route* route = &sol.routes[v];
                    for(int i = 0; i <= route->stops.size(); i++){
                        for(int j = i + 1; j <= route->stops.size(); j++){
                            
                            route->insert_stop(i, trans_node, request, true);
                            route->insert_stop(j, request->destination, request, false);

                            if(sol.is_feasible()){
                                double cost = sol.getCost();
                                if(cost < best_cost){
                                    best_cost = cost;
                                    best_method = 1; // Method 1: L1 then L2
                                    best_t_node = t;
                                    move1 = best_l1; // First move applied
                                    move2 = {v, i, j}; // Second move applied
                                }
                            }
                            route->erase_stop(j);
                            route->erase_stop(i);
                        }
                    }
                }
                // Revert Leg 1
                sol.routes[best_l1.v].erase_stop(best_l1.j);
                sol.routes[best_l1.v].erase_stop(best_l1.i);
            }

            // =================================================
            // METHOD (ii): Best Leg 2 -> Then Best Leg 1
            // =================================================

            // Step A: Find Best Leg 2 (Transfer -> Destination)
            double best_leg2_cost = std::numeric_limits<double>::max();
            MoveStep best_l2 = {-1, -1, -1};

            for(int v = 0; v < sol.problem->vehicle_amount; v++){
                Route* route = &sol.routes[v];
                double start_cost = route->calculate_cost();

                for(int i = 0; i <= route->stops.size(); i++){
                    for(int j = i + 1; j <= route->stops.size(); j++){ // FIX: removed +1
                        
                        route->insert_stop(i, trans_node, request, true);
                        route->insert_stop(j, request->destination, request, false);

                        if(true){
                            double delta = route->calculate_cost() - start_cost;
                            if(delta < best_leg2_cost){
                                best_leg2_cost = delta;
                                best_l2 = {v, i, j};
                            }
                        }
                        route->erase_stop(j);
                        route->erase_stop(i);
                    }
                }
            }

            // Step B: Fix Best Leg 2, Search for Best Leg 1
            if(best_l2.v != -1){
                // Apply Leg 2 temporarily
                sol.routes[best_l2.v].insert_stop(best_l2.i, trans_node, request, true);
                sol.routes[best_l2.v].insert_stop(best_l2.j, request->destination, request, false);

                for(int v = 0; v < sol.problem->vehicle_amount; v++){
                    Route* route = &sol.routes[v];
                    for(int i = 0; i <= route->stops.size(); i++){
                        for(int j = i + 1; j <= route->stops.size(); j++){ // FIX: removed +1
                            
                            route->insert_stop(i, request->origin, request, true);
                            route->insert_stop(j, trans_node, request, false);

                            if(sol.is_feasible()){
                                double cost = sol.getCost();
                                if(cost < best_cost){
                                    best_cost = cost;
                                    best_method = 2; // Method 2: L2 then L1
                                    best_t_node = t;
                                    move1 = best_l2;   // First move applied
                                    move2 = {v, i, j}; // Second move applied
                                }
                            }
                            route->erase_stop(j);
                            route->erase_stop(i);
                        }
                    }
                }
                // Revert Leg 2
                sol.routes[best_l2.v].erase_stop(best_l2.j);
                sol.routes[best_l2.v].erase_stop(best_l2.i);
            }
        }

        // -------------------------------------------------------
        // 3. Apply Best Move Safely
        // -------------------------------------------------------
        if(best_method == 0){
            sol.routes[move1.v].insert_stop(move1.i, request->origin, request, true);
            sol.routes[move1.v].insert_stop(move1.j, request->destination, request, false);
            sol.unassigned[r] = false;
        }
        else if(best_method == 1){
            // Apply in Order: Leg 1 then Leg 2 (Indices are valid in this order)
            Node* t_node = &sol.problem->transshipment_nodes[best_t_node];
            
            sol.routes[move1.v].insert_stop(move1.i, request->origin, request, true);
            sol.routes[move1.v].insert_stop(move1.j, t_node, request, false);

            sol.routes[move2.v].insert_stop(move2.i, t_node, request, true);
            sol.routes[move2.v].insert_stop(move2.j, request->destination, request, false);
            
            sol.unassigned[r] = false;
        }
        else if(best_method == 2){
            // Apply in Order: Leg 2 then Leg 1 (Indices are valid in this order)
            Node* t_node = &sol.problem->transshipment_nodes[best_t_node];
            
            // move1 here is Leg 2 (Transfer->Dest)
            sol.routes[move1.v].insert_stop(move1.i, t_node, request, true);
            sol.routes[move1.v].insert_stop(move1.j, request->destination, request, false);

            // move2 here is Leg 1 (Origin->Transfer)
            sol.routes[move2.v].insert_stop(move2.i, request->origin, request, true);
            sol.routes[move2.v].insert_stop(move2.j, t_node, request, false);
            
            sol.unassigned[r] = false;
        }
      }
    }

    std::unique_ptr<RepairMethod<PDPTWT_solution>> clone() const {
        return std::make_unique<insert_w_transfer>(*this);
    }
};