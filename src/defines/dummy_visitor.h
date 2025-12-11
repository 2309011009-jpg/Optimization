// A dummy visitor that does nothing, just to prevent the segfault
#include "../../libraries/adaptive-large-neighbourhood-search/src/AlgorithmVisitor.h"

template<class Solution>
class DummyVisitor : public mlpalns::AlgorithmVisitor<Solution> {
public:
    // CORRECT SIGNATURE:
    // 1. Destroy/Repair lists are NOT const (mutable)
    // 2. String descriptions ARE const
    void on_algorithm_start(std::vector<mlpalns::DestroyMethod<Solution>*>& destroy_methods,
                            std::vector<mlpalns::RepairMethod<Solution>*>& repair_methods,
                            const std::vector<std::string>& destroy_desc,
                            const std::vector<std::string>& repair_desc) override {
        // Do nothing
    }

    void on_iteration_end(mlpalns::AlgorithmStatus<Solution>& status) override {
        // Do nothing
    }

    void on_many_iters_without_improvement(std::vector<mlpalns::DestroyMethod<Solution>*>& destroy_methods,
                                           std::vector<mlpalns::RepairMethod<Solution>*>& repair_methods) override {
        // Do nothing
    }

    void on_prerun_end(std::vector<mlpalns::DestroyMethod<Solution>*>& destroy_methods,
                       std::vector<mlpalns::RepairMethod<Solution>*>& repair_methods) override {
        // Do nothing
    }
};