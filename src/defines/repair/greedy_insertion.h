#ifndef GREEDY_INSERTION_H
#define GREEDY_INSERTION_H

#include "../alns_definitions.h"
#include "../../../libraries/adaptive-large-neighbourhood-search/src/RepairMethod.h"
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>
#include <iostream>

class GreedyInsertion : public mlpalns::RepairMethod<PDPTWT_solution> {
public:
    std::unique_ptr<mlpalns::RepairMethod<PDPTWT_solution>> clone() const override {
        return std::make_unique<GreedyInsertion>(*this);
    }

    void repair_solution(PDPTWT_solution& solution, std::mt19937& mt) override {
        (void)mt;

        while (true) {
            int best_req_id = -1;
            int best_route_idx = -1;
            int best_p_idx = -1;
            int best_d_idx = -1;
            double min_total_cost = std::numeric_limits<double>::max();

            for (int i = 0; i < (int)solution.unassigned.size(); i++) {
                if (solution.unassigned[i] == true) { 
                    const Request* req = &solution.problem->requests[i];

                    for (int r = 0; r < (int)solution.routes.size(); r++) {
                        Route& route = solution.routes[r];

                        // Güzergahın başı ve sonu (depolar) arasına yerleştirme denemeleri
                        for (int p = 1; p < (int)route.stops.size(); p++) {
                            for (int d = p; d < (int)route.stops.size(); d++) {
                                
                                Route test_route = route;
                                // DOĞRU İSİMLER: origin ve destination
                                Stop p_stop(req->origin, req, true);
                                Stop d_stop(req->destination, req, false);
                                
                                test_route.stops.insert(test_route.stops.begin() + p, p_stop);
                                test_route.stops.insert(test_route.stops.begin() + d + 1, d_stop);

                                // Zaman pencereleri kontrolü
                                if (test_route.check_timing()) {
                                    // Kapasite kontrolü
                                    int load = 0;
                                    bool cap_ok = true;
                                    for(auto& s : test_route.stops) {
                                        load += s.get_load_change();
                                        if(load > solution.problem->vehicles[r].capacity) { 
                                            cap_ok = false; 
                                            break; 
                                        }
                                    }

                                    if(cap_ok) {
                                        double cost = test_route.calculate_cost();
                                        if (cost < min_total_cost) {
                                            min_total_cost = cost;
                                            best_req_id = i;
                                            best_route_idx = r;
                                            best_p_idx = p;
                                            best_d_idx = d;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (best_req_id != -1) {
                const Request* req = &solution.problem->requests[best_req_id];
                Stop p_stop(req->origin, req, true);
                Stop d_stop(req->destination, req, false);

                // Gerçek rotaya ekleme yapıyoruz
                solution.routes[best_route_idx].stops.insert(solution.routes[best_route_idx].stops.begin() + best_p_idx, p_stop);
                solution.routes[best_route_idx].stops.insert(solution.routes[best_route_idx].stops.begin() + best_d_idx + 1, d_stop);
                solution.unassigned[best_req_id] = false;

                std::cout << "[Greedy] Request " << best_req_id << " -> Vehicle " << best_route_idx 
                          << " (New Cost: " << min_total_cost << ")" << std::endl;
            } else {
                break; 
            }
        }
    }
};

#endif