#include "mid_panel.h"
#include "config_panel.h"
#include "info_panel.h"
#include "../lyu_and_yu_parser.h"
#include "../alns_definitions.h"
#include "../problem_definitions.h"
#include "../../../libraries/adaptive-large-neighbourhood-search/src/PALNS.h"
#include "../../defines/initial_solution.h"
#include "../../defines/dummy_visitor.h"

#include <thread>
#include <glibmm/dispatcher.h>

#include <gtkmm/filechoosernative.h>
#include <filesystem>

class main_box : public Gtk::Box {
public:
    // Threading Members
    Glib::Dispatcher m_dispatcher;
    std::thread m_worker_thread;
    PDPTWT_solution m_final_solution; // Temp storage for result

    // GUI Components
    PDPTWT problem;
    config_panel* config_box;
    mid_panel* mid_box;
    info_panel* info_box;

    // Destructor: Must join thread to prevent crash on exit
    ~main_box() {
        if (m_worker_thread.joinable()) {
            m_worker_thread.join();
        }
    }

    main_box() : Gtk::Box(Gtk::Orientation::HORIZONTAL, 5) {
        config_box = Gtk::make_managed<config_panel>();
        append(*config_box);

        mid_box = Gtk::make_managed<mid_panel>();
        append(*mid_box);
        mid_box->set_hexpand(true);

        info_box = Gtk::make_managed<info_panel>();
        append(*info_box);

        // Connect the "File Loaded" signal
        mid_box->load_btn->signal_problem_loaded().connect(
            sigc::mem_fun(*this, &main_box::update_problem)
        );

        mid_box->bottom_box.signal_problem_run().connect(
            sigc::mem_fun(*this, &main_box::solve_problem)
        );

        info_box->signal_solution_export().connect(
            sigc::mem_fun(*this, &main_box::export_solution)
        );

        // Connect the "Thread Finished" signal
        m_dispatcher.connect(sigc::mem_fun(*this, &main_box::on_solver_finished));

        config_box->set_busy_state(false);
    }

    // 1. Triggered when file is loaded
    void update_problem(std::string path) {
        // Parse on main thread (fast enough usually)
        problem = parse(path);
        
        // Update GUI
        info_box->load_problem(problem);
        mid_box->problem_box->load_data(problem);
    }

    // 2. Prepares data and launches thread
    void solve_problem() {
        // Lock UI
        config_box->set_busy_state(true);

        // DATA EXTRACTION (Must be done on Main Thread)
        // We copy the pointers/values we need so the thread doesn't touch widgets
        auto selected_repair = config_box->repair_operator_boxa.get_selected_operators();
        auto selected_destroy = config_box->destroy_operator_boxa.get_selected_operators();

        if(selected_repair.empty() || selected_destroy.empty()){
          cerr<< "No Operation selected";
          config_box->set_busy_state(false);
          return;
        }

        // Pointer to the CURRENT problem data
        const PDPTWT* prob_ptr = &problem; 

        // Join previous thread if it's still running (safety)
        if (m_worker_thread.joinable()) {
            m_worker_thread.join();
        }

        // LAUNCH WORKER
        m_worker_thread = std::thread(
            [this, prob_ptr, selected_repair, selected_destroy]() {
                this->run_solver_thread(prob_ptr, selected_repair, selected_destroy);
            }
        );
    }

   void export_solution() {
    // 1. Create the dialog
    // Use FileChooserNative for better OS integration
    auto dialog = Gtk::FileChooserNative::create(
        "Select Output Folder",
        *dynamic_cast<Gtk::Window*>(this->get_root()), 
        Gtk::FileChooser::Action::SELECT_FOLDER,
        "Select",
        "Cancel"
    );

    // 2. Connect the response signal
    dialog->signal_response().connect(
        [this, dialog](int response_id) {
            if (response_id == Gtk::ResponseType::ACCEPT) {
                auto file = dialog->get_file();
                
                if (file) {
                    // Use std::filesystem for safe path manipulation
                    std::filesystem::path folder_path(file->get_path());
                    std::filesystem::path full_path = folder_path / "solution_output.txt";

                    std::cout << "Exporting to: " << full_path.string() << std::endl;
                    
                    std::ofstream out_file(full_path.string());
                    if (out_file.is_open()) {
                        // --- HEADER INFO ---
                        // Replicating the data extraction from your functional 'run_solver'
                        out_file << "Instance: " << mid_box->load_btn->get_label() << "\n";
                        out_file << "Objective Cost: " << m_final_solution.getCost() << "\n";
                        out_file << "Feasible: " << (m_final_solution.hard_feasible() ? "YES" : "NO") << "\n";
                        out_file << "--------------------------------------------------\n\n";

                        // --- ROUTES ---
                        int v_id = 1;
                        for (size_t i = 0; i < m_final_solution.routes.size(); i++) {
                            // 1. Get Transfer Info (from functional code)
                            std::string transfer_info = m_final_solution.routes[i].get_transfer_str();
                            
                            // 2. Get Stops (returns vector<string> based on your SolverResult logic)
                            auto stops = m_final_solution.routes[i].get_route_str();

                            out_file << "Vehicle " << v_id++ << ": " << transfer_info << "\n";
                            out_file << "Path: ";

                            // 3. Iterate the strings directly
                            for (size_t j = 0; j < stops.size(); j++) {
                                out_file << stops[j];
                                if (j != stops.size() - 1) out_file << " -> ";
                            }
                            out_file << "\n\n";
                        }

                        out_file.close();
                    } else {
                        std::cerr << "Failed to open file for writing: " << full_path << std::endl;
                    }
                }
            }
        }
    );

    // 3. Show the dialog
    dialog->show();
}

    // 3. The Background Worker (NO GUI ACCESS HERE!)
    void run_solver_thread(const PDPTWT* prob, 
                           std::map<std::string, mlpalns::RepairMethod<PDPTWT_solution>*> repair_ops,
                           std::map<std::string, mlpalns::DestroyMethod<PDPTWT_solution>*> destroy_ops) 
    {
        PDPTWTSolutionCreator creator;
        std::mt19937 rng(42);
        
        // Use 'prob' pointer, not 'this->problem' directly to be safe
        PDPTWT_solution initial_sol = creator.create_initial_solution(*prob, rng);

        mlpalns::Parameters params("Params.json");
        std::unique_ptr<mlpalns::AlgorithmVisitor<PDPTWT_solution>> visitor = 
    std::make_unique<DummyVisitor<PDPTWT_solution>>();
        auto alns = mlpalns::PALNS<PDPTWT, PDPTWT_solution>{*prob};

        alns.set_algorithm_visitor(visitor);

        for (auto& oper : repair_ops) {
            alns.add_repair_method(*oper.second, oper.first);
        }
        for (auto& oper : destroy_ops) {
            alns.add_destroy_method(*oper.second, oper.first);
        }

        // HEAVY CALCULATION
        m_final_solution = alns.go(initial_sol, 15, params);
        // Signal completion
        m_dispatcher.emit();
    }

    // 4. Called on Main Thread when dispatcher emits
    void on_solver_finished() {
        config_box->set_busy_state(false);
        
        m_final_solution.print_solution();
        
        // Now it's safe to update the GUI
        info_box->load_solution(m_final_solution);
        mid_box->problem_box->load_solution(m_final_solution);
    }
};