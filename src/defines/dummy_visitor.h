// A dummy visitor that does nothing, just to prevent the segfault
#include "../../libraries/adaptive-large-neighbourhood-search/src/AlgorithmVisitor.h"

#include "../../libraries/adaptive-large-neighbourhood-search/src/AlgorithmStatus.h"
#include <algorithm> // for std::clamp
#include <iostream>
#include "alns_definitions.h"

#ifndef ADAPTIVE_PENALTY_VISITOR_H
#define ADAPTIVE_PENALTY_VISITOR_H

// =========================================================
// 2. THE VISITOR
// Hooks into the ALNS loop to update the penalty
// =========================================================
namespace mlpalns {

template<class Solution>
class DummyVisitor : public AlgorithmVisitor<Solution> {
public:
    
    // --- 1. Algorithm Start ---
    // Initialize the manager when the algorithm begins
    void on_algorithm_start(std::vector<DestroyMethod<Solution>*>& destroy, 
                          std::vector<RepairMethod<Solution>*>& repair,
                          const std::vector<std::string>& destroy_methods_desc, 
                          const std::vector<std::string>& repair_methods_desc) override {
        

    }

    void on_iteration_end(AlgorithmStatus<Solution>& alg_status) override {
    }

    // --- 3. Prerun End ---
    // Called after calibration. We can reset or keep the penalty.
    void on_prerun_end(std::vector<DestroyMethod<Solution>*>& destroy, 
                       std::vector<RepairMethod<Solution>*>& repair) override {


    }

    void on_many_iters_without_improvement(std::vector<DestroyMethod<Solution>*>& destroy, 
                                         std::vector<RepairMethod<Solution>*>& repair) override {
        
    }
    
    virtual ~DummyVisitor() = default;
};

}

#endif // ADAPTIVE_PENALTY_VISITOR_H