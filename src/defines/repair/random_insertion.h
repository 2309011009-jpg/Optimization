#ifndef RANDOM_INSERTION_H
#define RANDOM_INSERTION_H

#include "../alns_definitions.h"
#include "../../../libraries/adaptive-large-neighbourhood-search/src/RepairMethod.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <random>
#include <iostream>

class RandomInsertion : public mlpalns::RepairMethod<PDPTWT_solution> {
public:
    std::unique_ptr<mlpalns::RepairMethod<PDPTWT_solution>> clone() const override {
        return std::make_unique<RandomInsertion>(*this);
    }

    void repair_solution(PDPTWT_solution& solution, std::mt19937& mt) override {
        // 1. Atanmamış tüm requestleri topla
        std::vector<int> pending_reqs;
        for (int i = 0; i < (int)solution.unassigned.size(); i++) {
            if (solution.unassigned[i] == true) {
                pending_reqs.push_back(i);
            }
        }

        // 2. Requestleri karıştır (rastgelelik için)
        std::shuffle(pending_reqs.begin(), pending_reqs.end(), mt);

        for (int req_id : pending_reqs) {
            const Request* req = &solution.problem->requests[req_id];
            
            // Rotaları karıştır
            std::vector<int> route_indices(solution.routes.size());
            std::iota(route_indices.begin(), route_indices.end(), 0);
            std::shuffle(route_indices.begin(), route_indices.end(), mt);

            bool inserted = false;
            for (int r : route_indices) {
                Route& route = solution.routes[r];
                
                // Uygun tüm pickup-delivery pozisyonlarını topla
                struct Pos { int p, d; };
                std::vector<Pos> valid_positions;

                for (int p = 1; p < (int)route.stops.size(); p++) {
                    for (int d = p; d < (int)route.stops.size(); d++) {
                        Route test_route = route;
                        test_route.stops.insert(test_route.stops.begin() + p, Stop(req->origin, req, true));
                        test_route.stops.insert(test_route.stops.begin() + d + 1, Stop(req->destination, req, false));

                        if (test_route.check_timing()) {
                            int load = 0; bool cap_ok = true;
                            for(auto& s : test_route.stops) {
                                load += s.get_load_change();
                                if(load > solution.problem->vehicles[r].capacity) { cap_ok = false; break; }
                            }
                            if(cap_ok) valid_positions.push_back({p, d});
                        }
                    }
                }

                if (!valid_positions.empty()) {
                    std::uniform_int_distribution<int> dist(0, valid_positions.size() - 1);
                    Pos selected = valid_positions[dist(mt)];

                    route.stops.insert(route.stops.begin() + selected.p, Stop(req->origin, req, true));
                    route.stops.insert(route.stops.begin() + selected.d + 1, Stop(req->destination, req, false));
                    solution.unassigned[req_id] = false;
                    
                    std::cout << "[Random] Request " << req_id << " -> Vehicle " << r << std::endl;
                    inserted = true;
                    break; 
                }
            }
        }
    }
};

#endif