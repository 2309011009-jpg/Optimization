#pragma once
#include "../problem_definitions.h"
#include "../alns_definitions.h"
#include "../../../libraries/adaptive-large-neighbourhood-search/src/RepairMethod.h"
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

using namespace mlpalns;

// --- UNIQUE STRUCT NAMES TO AVOID COLLISIONS ---

struct TF_MoveStep {
    int v_idx = -1; 
    int p_idx = -1; 
    int d_idx = -1; 
};

struct TF_TransferMove {
    TF_MoveStep leg1; 
    TF_MoveStep leg2; 
    int t_node_idx = -1;
    double total_cost = std::numeric_limits<double>::max();
};

struct transfer_first_insertion : public RepairMethod<PDPTWT_solution> {

    void repair_solution(PDPTWT_solution& sol, std::mt19937& mt) override {
        
        // 1. Identify Unplanned Requests
        std::vector<const Request*> removed_requests;
        for(int i = 0; i < sol.problem->requests.size(); i++)
            if(sol.unassigned[i]) removed_requests.push_back(&sol.problem->requests[i]);

        auto rng = std::default_random_engine {};
        std::shuffle(std::begin(removed_requests), std::end(removed_requests), rng);

        // 2. Main Loop - Process requests one by one
        while(!removed_requests.empty()){

            int best_req_idx = -1;
            double global_min_cost = std::numeric_limits<double>::max();
            TF_TransferMove best_transfer_move_global;

            // Iterate over ALL unassigned requests to find the best insertion among them
            for(int r = 0; r < removed_requests.size(); r++){
                const Request* req = removed_requests[r];

                TF_TransferMove best_transfer_move;
                double best_transfer_cost = std::numeric_limits<double>::max();

                // --- 1. Iterate ALL Transfer Nodes ---
                for(int t_idx = 0; t_idx < sol.problem->transshipment_node_amount; ++t_idx) {
                    Node* trans_node = &sol.problem->transshipment_nodes[t_idx];

                    // --- 2. Iterate Leg 1 Vehicle (Origin -> Transfer) ---
                    for(int v1 = 0; v1 < sol.problem->vehicle_amount; v1++){
                        Route* r1 = &sol.routes[v1];
                        
                        // --- 3. Iterate Leg 1 Positions ---
                        for(int i = 0; i <= r1->stops.size(); i++){
                            r1->insert_stop(i, req->origin, req, true); 
                            for(int j = i + 1; j <= r1->stops.size(); j++){
                                r1->insert_stop(j, trans_node, req, false);

                                // --- 4. Iterate Leg 2 Vehicle (Transfer -> Dest) ---
                                for(int v2 = 0; v2 < sol.problem->vehicle_amount; v2++){
                                    
                                    // STRICTLY DISABLE SELF-TRANSFERS
                                    if(v1 == v2) continue; 
                                    
                                    Route* r2 = &sol.routes[v2];

                                    // --- 5. Iterate Leg 2 Positions ---
                                    for(int k = 0; k <= r2->stops.size(); k++){
                                        r2->insert_stop(k, trans_node, req, true);
                                        for(int l = k + 1; l <= r2->stops.size(); l++){
                                            r2->insert_stop(l, req->destination, req, false);

                                            // --- 6. Check Global Feasibility & Cost ---
                                            if(sol.is_feasible()){
                                                double total = sol.getCost();
                                                if(total < best_transfer_cost){
                                                    best_transfer_cost = total;
                                                    
                                                    best_transfer_move.leg1 = {v1, i, j};
                                                    best_transfer_move.leg2 = {v2, k, l};
                                                    best_transfer_move.t_node_idx = t_idx;
                                                    best_transfer_move.total_cost = total;
                                                }
                                            }
                                            r2->erase_stop(l);
                                        }
                                        r2->erase_stop(k);
                                    }
                                }
                                r1->erase_stop(j);
                            }
                            r1->erase_stop(i); 
                        }
                    }
                }

                // --- Update Global Best ---
                if(best_transfer_move.t_node_idx != -1 && best_transfer_cost < global_min_cost){
                    global_min_cost = best_transfer_cost;
                    best_req_idx = r;
                    best_transfer_move_global = best_transfer_move;
                }
            }

            // 3. Apply Best Global Move
            if(best_req_idx == -1) break; // No feasible transfers found

            const Request* req = removed_requests[best_req_idx];
            sol.unassigned[req->id] = false;

            if(best_transfer_move_global.t_node_idx != -1) {
                Node* t_node = &sol.problem->transshipment_nodes[best_transfer_move_global.t_node_idx];
                
                Route& r1 = sol.routes[best_transfer_move_global.leg1.v_idx];
                r1.insert_stop(best_transfer_move_global.leg1.p_idx, req->origin, req, 1);
                r1.insert_stop(best_transfer_move_global.leg1.d_idx, t_node, req, 0);
                
                Route& r2 = sol.routes[best_transfer_move_global.leg2.v_idx];
                r2.insert_stop(best_transfer_move_global.leg2.p_idx, t_node, req, 1);
                r2.insert_stop(best_transfer_move_global.leg2.d_idx, req->destination, req, 0);
            } 
            else {
                sol.unassigned[req->id] = true;
            }

            removed_requests.erase(removed_requests.begin() + best_req_idx);
        }
    }

    std::unique_ptr<RepairMethod<PDPTWT_solution>> clone() const override {
        return std::make_unique<transfer_first_insertion>(*this);
    }
};