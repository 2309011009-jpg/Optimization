#include "gtkmm/adjustment.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/frame.h"
#include "gtkmm/label.h"

#include <map>
#include "../../../libraries/adaptive-large-neighbourhood-search/src/DestroyMethod.h"
#include "../alns_definitions.h"

// Destroy Operators
#include "../destruction/random_removal.h"
#include "../destruction/shaw_removal.h"
#include "gtkmm/menubutton.h"
#include "gtkmm/spinbutton.h"
#include "gtkmm/togglebutton.h"


#ifndef DESTROY
#define DESTROY

class destroy_operator_box : public Gtk::Box{
  public:


    std::map<std::string, mlpalns::DestroyMethod<PDPTWT_solution>* > destroy_operators;
    std::map<std::string, Gtk::ToggleButton*> toggle_buttons;

    void load_operators() {
      destroy_operators["Random Removal"] = new struct RandomRemoval;
      destroy_operators["Shaw Removal"] = new struct ShawRemoval;
    }

    destroy_operator_box(): Gtk::Box(Gtk::Orientation::VERTICAL, 5){
        load_operators();
        append(destroy_title);

        // 1. Loop through the map
        for (auto& destroy_operator : destroy_operators) {
            
            // 2. DEFINE op_ptr HERE
            std::string op_name = destroy_operator.first;
            auto* op_ptr = destroy_operator.second; // <--- This was missing!

            auto operator_frame = Gtk::make_managed<Gtk::Frame>();
            auto operator_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);

            auto config_button = Gtk::make_managed<Gtk::MenuButton>();
            config_button->set_icon_name("preferences-system");

            // 3. Now op_ptr exists for this check
            Configurable* config_ptr = dynamic_cast<Configurable*>(op_ptr);

            if (config_ptr && !config_ptr->params.empty()) {
                auto popover = Gtk::make_managed<Gtk::Popover>();
                auto pop_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
                pop_box->set_margin(10);

                for (auto& param_pair : config_ptr->params) {
                    std::string p_name = param_pair.first;
                    Parameter& p = param_pair.second;

                    auto label = Gtk::make_managed<Gtk::Label>(p.name);
                    label->set_halign(Gtk::Align::START);
                    pop_box->append(*label);

                    auto adjustment = Gtk::Adjustment::create(p.value, p.min, p.max, p.step);
                    auto spin_btn = Gtk::make_managed<Gtk::SpinButton>(adjustment);
                    spin_btn->set_digits(p.step < 1.0 ? 2 : 0);

                    // Connect signal
                    spin_btn->signal_value_changed().connect(
                        [config_ptr, p_name, spin_btn]() {
                            config_ptr->set_param_value(p_name, spin_btn->get_value());
                        }
                    );

                    pop_box->append(*spin_btn);
                }
                popover->set_child(*pop_box);
                config_button->set_popover(*popover);
            } 
            else {
                config_button->set_sensitive(false);
                config_button->set_opacity(0.5);
            }

            auto operator_title = Gtk::make_managed<Gtk::ToggleButton>(op_name);
            toggle_buttons[op_name] = operator_title;

            operator_frame->set_child(*operator_box);
            operator_title->set_hexpand(true);
            operator_frame->set_hexpand(false);
            
            operator_box->append(*config_button);
            operator_box->append(*operator_title);

            this->append(*operator_frame);
        }
    }

    std::map<std::string, mlpalns::DestroyMethod<PDPTWT_solution>*> get_selected_operators() {
        
        std::map<std::string, mlpalns::DestroyMethod<PDPTWT_solution>*> selected_ops;
        
        for (auto const& [name, btn] : toggle_buttons) {
            // 1. Check if the button is pressed
            if (btn->get_active()) {
                
                // 2. Retrieve the pointer from your main map
                // We use .at() for safety (throws if key missing), or [] if you are sure
                selected_ops[name] = destroy_operators.at(name);
            }
        }
        
        return selected_ops;
    }

    ~destroy_operator_box() {
        for (auto& pair : destroy_operators) {
            delete pair.second;
        }
    }

    Gtk::Label destroy_title{"Destroy Operators"};

};

#endif