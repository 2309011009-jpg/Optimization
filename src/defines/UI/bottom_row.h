#include "gtkmm/box.h"
#include "gtkmm/button.h"

#ifndef BOTTOM
#define BOTTOM

class bottom_row : public Gtk::Box{
public:

  bottom_row() : Gtk::Box(Gtk::Orientation::HORIZONTAL, 5) {
    append(left_button);
    append(run_button);
    append(right_button);

    run_button.set_hexpand(true);

    run_button.signal_clicked().connect(sigc::mem_fun(*this, &bottom_row::on_button_clicked));
  }

  void on_button_clicked(){
    m_signal_problem_run.emit();
  }

  sigc::signal<void(void)> m_signal_problem_run;

  sigc::signal<void(void)> signal_problem_run() {
        return m_signal_problem_run;
  }

  Gtk::Button left_button{"<"};
  Gtk::Button run_button{"Run"};
  Gtk::Button right_button{">"};
};


#endif