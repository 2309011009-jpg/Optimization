#include "../problem_definitions.h"
#include "../alns_definitions.h"
#include "../../../libraries/adaptive-large-neighbourhood-search/src/RepairMethod.h"
#include <vector>
#include <algorithm>
#include <random>
#include <limits>

using namespace mlpalns;

// Structure to hold a single leg move (Pickup -> Delivery)
struct MoveStep {
    int v_idx = -1;  // Vehicle Index
    int p_idx = -1;  // Pickup Index
    int d_idx = -1;  // Delivery Index
    double cost_delta = std::numeric_limits<double>::max();
};

// Structure to hold a complete Transfer Move (Leg 1 + Leg 2)
struct TransferMove {
    MoveStep leg1; // Origin -> Transfer
    MoveStep leg2; // Transfer -> Destination
    int t_node_idx = -1;
    double total_cost = std::numeric_limits<double>::max();
};

struct regret_w_transfer : public RepairMethod<PDPTWT_solution> {

    void repair_solution(PDPTWT_solution& sol, std::mt19937& mt) {
        
        // 1. Identify Unplanned Requests
        std::vector<const Request*> removed_requests;
        for(int i = 0; i < sol.problem->requests.size(); i++)
            if(sol.unassigned[i]) removed_requests.push_back(&sol.problem->requests[i]);

        auto rng = std::default_random_engine {};
        std::shuffle(std::begin(removed_requests), std::end(removed_requests), rng);

        while(!removed_requests.empty()){

            int best_req_idx = -1;
            double max_regret = -std::numeric_limits<double>::max();
            
            MoveStep best_direct_move_global;
            TransferMove best_transfer_move_global;
            bool use_transfer_global = false;

            // ------------------------------------------------------------------
            // EVALUATE ALL REQUESTS TO FIND MAX REGRET
            // ------------------------------------------------------------------
            for(int r = 0; r < removed_requests.size(); r++){
                const Request* req = removed_requests[r];

                // --- A. Calculate Best Direct Insertion ---
                double best_direct_cost = std::numeric_limits<double>::max();
                MoveStep best_direct_move;

                for(int v = 0; v < sol.problem->vehicle_amount; v++){
                    Route* route = &sol.routes[v];
                    double start_cost = route->calculate_cost(); // Cache start cost

                    for(int i = 0; i <= route->stops.size(); i++){
                        route->insert_stop(i, req->origin, req, true);
                        
                        for(int j = i + 1; j <= route->stops.size(); j++){
                            route->insert_stop(j, req->destination, req, false);

                            if(sol.is_feasible()){
                                double total_cost = sol.getCost(); 
                                if(total_cost < best_direct_cost){
                                    best_direct_cost = total_cost;
                                    best_direct_move = {v, i, j, total_cost};
                                }
                            }
                            route->erase_stop(j);
                        }
                        route->erase_stop(i); 
                    }
                }

                // --- B. Calculate Best Transfer Insertion (Sequential Leg Method) ---
                double best_transfer_cost = std::numeric_limits<double>::max();
                TransferMove best_transfer_move;

                // Optimization: Select only closest transfer point (m=1)
                int best_t_candidate = -1;
                double min_dist_t = std::numeric_limits<double>::max();
                for(int t=0; t<sol.problem->transshipment_node_amount; ++t) {
                   double d = sol.problem->get_distance(req->origin, &sol.problem->transshipment_nodes[t]) + 
                              sol.problem->get_distance(&sol.problem->transshipment_nodes[t], req->destination);
                   if(d < min_dist_t) { min_dist_t = d; best_t_candidate = t; }
                }

                std::vector<int> t_candidates;
                if(best_t_candidate != -1) t_candidates.push_back(best_t_candidate);
                
                for(int t_idx : t_candidates){
                    Node* trans_node = &sol.problem->transshipment_nodes[t_idx];
                    
                    // =========================================================
                    // Method (i): Leg 1 First (Origin -> Transfer)
                    // =========================================================
                    MoveStep leg1_candidate;
                    for(int v = 0; v < sol.problem->vehicle_amount; v++){
                        Route* route = &sol.routes[v];
                        for(int i = 0; i <= route->stops.size(); i++){
                            route->insert_stop(i, req->origin, req, true); 
                            for(int j = i + 1; j <= route->stops.size(); j++){
                                route->insert_stop(j, trans_node, req, false);

                                if(sol.is_feasible()){
                                    double c = sol.getCost(); 
                                    if(c < leg1_candidate.cost_delta) leg1_candidate = {v, i, j, c};
                                }
                                route->erase_stop(j);
                            }
                            route->erase_stop(i); 
                        }
                    }

                    // If Leg 1 found, fix it temporarily and find Leg 2
                    if(leg1_candidate.v_idx != -1){
                        Route* r1 = &sol.routes[leg1_candidate.v_idx];
                        r1->insert_stop(leg1_candidate.p_idx, req->origin, req, true);
                        r1->insert_stop(leg1_candidate.d_idx, trans_node, req, false);

                        for(int v = 0; v < sol.problem->vehicle_amount; v++){
                            if(v == leg1_candidate.v_idx) continue;
                            Route* r2 = &sol.routes[v];
                            for(int i = 0; i <= r2->stops.size(); i++){
                                r2->insert_stop(i, trans_node, req, true); 
                                for(int j = i + 1; j <= r2->stops.size(); j++){
                                    r2->insert_stop(j, req->destination, req, false);

                                    if(sol.is_feasible()){
                                        double total = sol.getCost();
                                        if(total < best_transfer_cost){
                                            best_transfer_cost = total;
                                            best_transfer_move = {leg1_candidate, {v, i, j, 0.0}, t_idx, total};
                                        }
                                    }
                                    r2->erase_stop(j);
                                }
                                r2->erase_stop(i);
                            }
                        }
                        // Revert Leg 1
                        r1->erase_stop(leg1_candidate.d_idx);
                        r1->erase_stop(leg1_candidate.p_idx);
                    }
                    
                    // =========================================================
                    // Method (ii): Leg 2 First (Transfer -> Destination)
                    // =========================================================
                    MoveStep leg2_candidate;
                    for(int v = 0; v < sol.problem->vehicle_amount; v++){
                        Route* route = &sol.routes[v];
                        for(int i = 0; i <= route->stops.size(); i++){
                            route->insert_stop(i, trans_node, req, true);
                            for(int j = i + 1; j <= route->stops.size(); j++){
                                route->insert_stop(j, req->destination, req, false);

                                if(sol.is_feasible()){
                                    double c = sol.getCost();
                                    if(c < leg2_candidate.cost_delta) leg2_candidate = {v, i, j, c};
                                }
                                route->erase_stop(j);
                            }
                            route->erase_stop(i);
                        }
                    }
                    
                    // If Leg 2 found, fix it temporarily and find Leg 1
                    if(leg2_candidate.v_idx != -1){
                        Route* r2 = &sol.routes[leg2_candidate.v_idx];
                        r2->insert_stop(leg2_candidate.p_idx, trans_node, req, true);
                        r2->insert_stop(leg2_candidate.d_idx, req->destination, req, false);

                        for(int v = 0; v < sol.problem->vehicle_amount; v++){
                            if(v == leg2_candidate.v_idx) continue;
                            Route* r1 = &sol.routes[v];
                            for(int i = 0; i <= r1->stops.size(); i++){
                                r1->insert_stop(i, req->origin, req, true);
                                for(int j = i + 1; j <= r1->stops.size(); j++){
                                    r1->insert_stop(j, trans_node, req, false);

                                    if(sol.is_feasible()){
                                        double total = sol.getCost();
                                        if(total < best_transfer_cost){
                                            best_transfer_cost = total;
                                            // Leg 1 is the one we just found (r1), Leg 2 is fixed (r2)
                                            best_transfer_move = {{v, i, j, 0.0}, leg2_candidate, t_idx, total};
                                        }
                                    }
                                    r1->erase_stop(j);
                                }
                                r1->erase_stop(i);
                            }
                        }
                        // Revert Leg 2
                        r2->erase_stop(leg2_candidate.d_idx);
                        r2->erase_stop(leg2_candidate.p_idx);
                    }
                }

                // --- C. Regret Calculation & FORCED TRANSFER DECISION ---
                double regret = best_direct_cost - best_transfer_cost;

                if(regret > max_regret){
                    max_regret = regret;
                    best_req_idx = r;

                    // FORCE TRANSFER logic: 
                    // Always prefer transfer if a valid one exists.
                    if (best_transfer_move.t_node_idx != -1) {
                        use_transfer_global = true;
                        best_transfer_move_global = best_transfer_move;
                    } 
                    else {
                        use_transfer_global = false;
                        best_direct_move_global = best_direct_move;
                    }
                }
            }

            // ------------------------------------------------------------------
            // APPLY BEST MOVE
            // ------------------------------------------------------------------
            if(best_req_idx == -1) break; 

            const Request* req = removed_requests[best_req_idx];
            
            sol.unassigned[req->id] = false;
            
            if(use_transfer_global && best_transfer_move_global.t_node_idx != -1) {
                // Apply Leg 1
                Node* t_node = &sol.problem->transshipment_nodes[best_transfer_move_global.t_node_idx];
                Route& r1 = sol.routes[best_transfer_move_global.leg1.v_idx];
                r1.insert_stop(best_transfer_move_global.leg1.p_idx, req->origin, req, 1);
                r1.insert_stop(best_transfer_move_global.leg1.d_idx, t_node, req, 0);
                
                // Apply Leg 2
                Route& r2 = sol.routes[best_transfer_move_global.leg2.v_idx];
                r2.insert_stop(best_transfer_move_global.leg2.p_idx, t_node, req, 1);
                r2.insert_stop(best_transfer_move_global.leg2.d_idx, req->destination, req, 0);
            } else if (!use_transfer_global && best_direct_move_global.v_idx != -1) {
                // Apply Direct
                Route& r = sol.routes[best_direct_move_global.v_idx];
                r.insert_stop(best_direct_move_global.p_idx, req->origin, req, 1);
                r.insert_stop(best_direct_move_global.d_idx, req->destination, req, 0);
            }
            else sol.unassigned[req->id] = true;

          
            removed_requests.erase(removed_requests.begin() + best_req_idx);
        }
    }

    std::unique_ptr<RepairMethod<PDPTWT_solution>> clone() const override {
        return std::make_unique<regret_w_transfer>(*this);
    }
};