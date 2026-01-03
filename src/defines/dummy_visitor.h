// A dummy visitor that does nothing, just to prevent the segfault
#include "../../libraries/adaptive-large-neighbourhood-search/src/AlgorithmVisitor.h"

#include "../../libraries/adaptive-large-neighbourhood-search/src/AlgorithmStatus.h"
#include <algorithm> // for std::clamp
#include <iostream>
#include "alns_definitions.h"

#ifndef ADAPTIVE_PENALTY_VISITOR_H
#define ADAPTIVE_PENALTY_VISITOR_H

// =========================================================
// 1. PENALTY MANAGER (Singleton)
// Acts as the bridge between the Visitor (which updates it)
// and the Solution (which reads it).
// =========================================================
class PenaltyManager {
public:
    // Access the single instance
    static PenaltyManager& getInstance() {
        static PenaltyManager instance;
        return instance;
    }

    // The dynamic multiplier
    double multiplier;

    PDPTWT_solution* sol;

    // Parameters for adjustment
    double growth_factor = 1.1;
    double relax_factor = 1.1;
    double min_penalty = 500.0;
    double max_penalty = 40000.0;

    void reset() {
        multiplier = 1.0; // Default starting value
    }

    void tighten() {
        multiplier *= growth_factor;
        sanitize();
    }

    void relax() {
        multiplier /= relax_factor;
        sanitize();
    }

private:
    PenaltyManager() : multiplier(1.0) {} // Private constructor
    
    void sanitize() {
        multiplier = std::clamp(multiplier, min_penalty, max_penalty);
    }

    // Delete copy constructors to ensure singleton uniqueness
    public:
        PenaltyManager(const PenaltyManager&) = delete;
        void operator=(const PenaltyManager&) = delete;
};


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
        
        PenaltyManager::getInstance().reset();
        std::cout << "[Visitor] Algorithm started. Penalty reset to " 
                  << PenaltyManager::getInstance().multiplier << std::endl;

    }

    // --- 2. Iteration End (THE CORE LOGIC) ---
    // Check if the current solution is feasible and adjust penalty
    void on_iteration_end(AlgorithmStatus<Solution>& alg_status) override {
        
        // Access the current solution from the status object.
        // NOTE: Depending on your library version, this might be a pointer or reference.
        // Usually alg_status.curr_solution is the one accepted in the current iteration.
        // We check the CANDIDATE (alg_status.best_solution or similar might be global best).
        // Let's assume we check the solution accepted/generated in this step.
        /*
        // Assuming alg_status.curr_solution is a pointer to the Solution object
        if (alg_status.best_solution.hard_feasible()) {
             PenaltyManager::getInstance().relax();
             alg_status.best_solution.penalty_multiplier = PenaltyManager::getInstance().multiplier;
        } else {
             PenaltyManager::getInstance().tighten();
             alg_status.best_solution.penalty_multiplier = PenaltyManager::getInstance().multiplier;
        }
        */
        // Optional: Print status every 100 iterations to debug
        // if (alg_status.iteration_number % 100 == 0) {
        //     std::cout << "Iter: " << alg_status.iteration_number 
        //               << " | Penalty: " << PenaltyManager::getInstance().multiplier << std::endl;
        // }
    }

    // --- 3. Prerun End ---
    // Called after calibration. We can reset or keep the penalty.
    void on_prerun_end(std::vector<DestroyMethod<Solution>*>& destroy, 
                       std::vector<RepairMethod<Solution>*>& repair) override {
        // Optional: Reset penalty after calibration phases if needed
    }

    // --- 4. Stagnation ---
    // If we are stuck, we might want to shake things up violently.
    void on_many_iters_without_improvement(std::vector<DestroyMethod<Solution>*>& destroy, 
                                         std::vector<RepairMethod<Solution>*>& repair) override {
        
        // If we are stuck, drastically reducing the penalty might help the algorithm 
        // "tunnel" through an infeasible region, OR increasing it might force it out.
        // For now, we leave it to the standard iteration logic.
    }
    
    virtual ~DummyVisitor() = default;
};

} // namespace mlpalns

#endif // ADAPTIVE_PENALTY_VISITOR_H