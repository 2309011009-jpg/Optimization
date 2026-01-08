#include "../alns_definitions.h"
#include "../problem_definitions.h"
#include "gtkmm/box.h"
#include "gtkmm/label.h"
#include <string>
#include "gtkmm/button.h"

#ifndef INFO
#define INFO

class info_panel : public Gtk::Box{
public:

  void load_problem(const PDPTWT& p){
    vehicle_info.set_label("Vehicle Amount: " + to_string(p.vehicle_amount));
    request_info.set_label("Request Amount: " + to_string(p.requests.size()));
    node_info.set_label("Node Amount: " + to_string(p.node_amount));
    transshipment_info.set_label("Transshipment Amount: " + to_string(p.transshipment_node_amount));
    cost_info.set_label("Cost: Unknown");
    feasibility_info.set_label("Feasible: Unknown");

    export_solution.set_visible(false);
  }

  void load_solution(const PDPTWT_solution& sol){
    cost_info.set_label("Cost: " + to_string(sol.getCost()));
    feasibility_info.set_label(std::string("Feasible: ") + (sol.hard_feasible() ? "Yes!" : "No :("));
    export_solution.set_visible(true);
  }

  void on_button_clicked(){
    m_signal_solution_export.emit();
  }

  sigc::signal<void(void)> m_signal_solution_export;

  sigc::signal<void(void)> signal_solution_export() {
        return m_signal_solution_export;
  }

  info_panel() : Gtk::Box(Gtk::Orientation::VERTICAL, 15){
    append(vehicle_info);
    append(request_info);
    append(node_info);
    append(transshipment_info);

    append(cost_info);
    append(feasibility_info);

    export_solution.set_visible(false);
    append(export_solution);
    export_solution.signal_clicked().connect(sigc::mem_fun(*this, &info_panel::on_button_clicked));
  }

  Gtk::Label vehicle_info;
  Gtk::Label request_info;
  Gtk::Label node_info;
  Gtk::Label transshipment_info;

  Gtk::Label cost_info;
  Gtk::Label feasibility_info;
  Gtk::Button export_solution{"Export Solution"};
};

#endif