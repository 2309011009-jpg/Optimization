#include <iostream>
#include <fstream>
#include <vector>
#include <cctype> // for std::isdigit
#include <string>
#include <chrono>
#include <algorithm>
#include <iomanip> // <--- Add this header

// Include your existing project headers
#include "../defines/problem_definitions.h"
#include "../defines/alns_definitions.h"
#include "../defines/lyu_and_yu_parser.h"
#include "../defines/initial_solution.h"
#include "../defines/greedy_insertion.h"
#include "../defines/random_removal.h"
#include "../defines/dummy_visitor.h"

// Include ALNS Library Headers
#include "../../libraries/adaptive-large-neighbourhood-search/src/PALNS.h"
#include "../../libraries/adaptive-large-neighbourhood-search/src/Parameters.h"

#include <boost/filesystem.hpp>
using namespace mlpalns;

//g++ ./benchmark.cpp -o solver -lboost_filesystem

namespace fs = boost::filesystem;

// --- Natural Sort Logic ---
// Breaks strings into chunks of Text vs Numbers and compares them intelligently.
bool natural_less(const fs::path& a_path, const fs::path& b_path) {
    std::string a = a_path.filename().string();
    std::string b = b_path.filename().string();

    size_t i = 0, j = 0;
    while (i < a.length() && j < b.length()) {
        // If both are digits, parse the full number and compare numerically
        if (std::isdigit(a[i]) && std::isdigit(b[j])) {
            char* end_a;
            char* end_b;
            
            // strtoul parses the number starting at current position
            unsigned long num_a = std::strtoul(&a[i], &end_a, 10);
            unsigned long num_b = std::strtoul(&b[j], &end_b, 10);

            if (num_a != num_b) {
                return num_a < num_b;
            }

            // If numbers are equal, advance pointers to end of the numbers
            i = end_a - a.data();
            j = end_b - b.data();
        } 
        else {
            // Standard character comparison
            if (a[i] != b[j]) {
                return a[i] < b[j];
            }
            i++;
            j++;
        }
    }
    
    // If we run out of characters, the shorter string comes first
    return a.length() < b.length();
}

// --- Recursive Scan ---
void scan_directory(const fs::path& dir_path, int depth = 0) {
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) return;

    std::vector<fs::path> entries;
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        entries.push_back(entry.path());
    }

    // USE THE CUSTOM COMPARATOR HERE
    std::sort(entries.begin(), entries.end(), natural_less);

    for (const auto& path : entries) {
        std::string indent(depth * 4, ' ');

        if (fs::is_directory(path)) {
            std::cout << indent << "[DIR] " << path.filename() << std::endl;
            scan_directory(path, depth + 1);
        } 
        else if (fs::is_regular_file(path)) {
            std::cout << indent << path.filename() << std::endl;
            
            // Call solver here
        }
    }
}

struct SolverResult {
    double objective_value;
    double computation_time_ms;
    bool is_feasible;
};

SolverResult run_solver(const fs::path& instance_path) {
    std::cout << "   -> Solving: " << instance_path.filename() << " ... " << std::flush;
    

    // START TIMER (Optional, if your solver doesn't track its own time)
    auto start = std::chrono::high_resolution_clock::now();

    PDPTWT problem = parse(instance_path.string());
    auto alns = PALNS<PDPTWT, PDPTWT_solution>{problem};

    PDPTWTSolutionCreator creator;
    std::mt19937 rng(42);
    PDPTWT_solution initial_sol = creator.create_initial_solution(problem, rng); 

    RandomRemoval random_removal;
    alns.add_destroy_method(random_removal, "Remove Randomly");

    insert_w_transfer insert_w_transfer;
    alns.add_repair_method(insert_w_transfer, "Insert With Transfer");

    mlpalns::Parameters params("Params.json");

    std::unique_ptr<mlpalns::AlgorithmVisitor<PDPTWT_solution>> visitor = 
    std::make_unique<DummyVisitor<PDPTWT_solution>>();
    alns.set_algorithm_visitor(visitor);

    PDPTWT_solution best_sol = alns.go(initial_sol, 16, params);


    // DUMMY DATA FOR DEMONSTRATION:
    SolverResult res;
    res.objective_value = best_sol.getCost(); 
    res.is_feasible = best_sol.is_feasible();

    // END TIMER
    auto end = std::chrono::high_resolution_clock::now();
    res.computation_time_ms = std::chrono::duration<double>(end - start).count();

    std::cout << "Done." << std::endl;
    return res;
}

// --- 4. Recursive Scan with CSV Writing ---
void scan_and_solve(const fs::path& dir_path, std::ofstream& csv_file) {
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) return;

    std::vector<fs::path> entries;
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        entries.push_back(entry.path());
    }

    std::sort(entries.begin(), entries.end(), natural_less);

    for (const auto& path : entries) {
        if (fs::is_directory(path)) {
            // Recurse into subfolder
            scan_and_solve(path, csv_file); 
        } 
        else if (fs::is_regular_file(path)) {
            // 1. Run the solver
            SolverResult result = run_solver(path);

            // 2. Write to CSV
            // Format: Instance Name, Feasible, Cost, Time(ms)
            csv_file << path.filename().string() << ","
              << (result.is_feasible ? "YES" : "NO") << ","
              << result.objective_value << ","
              << std::fixed << std::setprecision(6) << result.computation_time_ms << "\n";
            
            // Optional: Flush to save immediately (safer if crash happens)
            csv_file.flush();
        }
    }
}

int main() {
    fs::path rootPath("data/");
    
    // Open CSV file for writing
    std::ofstream csv_file("benchmark_results.csv");
    
    // Check if file opened successfully
    if (!csv_file.is_open()) {
        std::cerr << "Error: Could not create results.csv" << std::endl;
        return 1;
    }

    // Write CSV Header
    csv_file << "Instance,Feasible,Objective,Time_ms\n";

    std::cout << "Starting Benchmark..." << std::endl;
    std::cout << "Results will be saved to 'benchmark_results.csv'" << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    scan_and_solve(rootPath, csv_file);

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Benchmark Complete." << std::endl;
    csv_file.close();

    return 0;
}