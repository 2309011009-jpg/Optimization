#include "gtkmm/button.h"
#include <gtkmm.h>

class problem_load_button : public Gtk::Button {
public:

  sigc::signal<void(std::string)> m_signal_problem_loaded;

  void on_button_clicked() {
    auto root = this->get_root();
    auto window = dynamic_cast<Gtk::Window*>(root);

    auto dialog = Gtk::FileChooserNative::create(
        "Please pick a PDPTW problem file",
        *window, // Gets the parent window this button belongs to
        Gtk::FileChooser::Action::OPEN,
        "Load", 
        "Cancel"
    );

    dialog->set_modal(true);

    dialog->signal_response().connect([this, dialog](int response_id) {
            if (response_id == Gtk::ResponseType::ACCEPT) {
                auto file = dialog->get_file();
                if (file) {
                    problem_path = file->get_path();
                    this->set_label(file->get_basename());

                    m_signal_problem_loaded.emit(problem_path);
                }
            }
        });

        dialog->show();
  }


  problem_load_button(){
    set_label("Load Problem");
    signal_clicked().connect(sigc::mem_fun(*this, &problem_load_button::on_button_clicked));
  }

  sigc::signal<void(std::string)> signal_problem_loaded() {
        return m_signal_problem_loaded;
  }
  
  std::string problem_path;

};