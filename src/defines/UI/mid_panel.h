#include "gtkmm/box.h"
#include "problem_load_button.h"
#include "problem_box.h"
#include "bottom_row.h"

#ifndef MID
#define MID

class mid_panel : public Gtk::Box{
public:

  mid_panel() : Gtk::Box(Gtk::Orientation::VERTICAL, 5){
    load_btn = Gtk::make_managed<problem_load_button>();
    append(*load_btn);

    problem_box = Gtk::make_managed<ProblemCanvas>();
    append(*problem_box);
    
    problem_box->set_hexpand(true);
    problem_box->set_vexpand(true);

    append(bottom_box);
  }

    bottom_row bottom_box;
    ProblemCanvas* problem_box;
    problem_load_button* load_btn;

};

#endif