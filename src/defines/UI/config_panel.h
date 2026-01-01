#include "gtkmm/label.h"
#include "repair_operators.h"
#include "destroy_operators.h"
#include "gtkmm/spinner.h"

#ifndef CONFIG
#define CONFIG

class config_panel : public Gtk::Box{
public:

  config_panel() : Gtk::Box(Gtk::Orientation::VERTICAL, 25) {

    append(title);
    
    append(repair_operator_boxa);
    append(destroy_operator_boxa);

    m_spinner.set_halign(Gtk::Align::CENTER);
    m_spinner.set_valign(Gtk::Align::END);
    m_spinner.set_size_request(32, 32);
    m_spinner.set_visible(false);
    m_spinner.set_vexpand(true);
    append(m_spinner);
  }

  void set_busy_state(bool is_busy) {
      if (is_busy) {
          m_spinner.set_visible(true); // Show it
          m_spinner.start();           // Start the animation
      } else {
          m_spinner.stop();            // Stop animation
          m_spinner.set_visible(false);// Hide it again
      }
  }

  Gtk::Label title{"Configuration"};
  repair_operator_box repair_operator_boxa;
  destroy_operator_box destroy_operator_boxa;
  Gtk::Spinner m_spinner;

};


#endif