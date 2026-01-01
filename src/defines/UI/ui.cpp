#include "main_panel.h"


class MainWindow : public Gtk::Window {
public:
  MainWindow() {
    set_title("PDPTW-T");
    set_default_size(1200, 900);
    
    main = Gtk::make_managed<main_box>();

    main->set_spacing(10);
    main->set_margin(10); // set_margin replaces margin_top/bottom/etc in GTK4 helpers

    set_child(*main);
  }

  main_box* main;

};



int main(int argc, char *argv[]) {
    // GTK4 Application::create signature is different (no argc/argv)
    auto app = Gtk::Application::create("org.gtkmm.example.pdptwt");
    
    return app->make_window_and_run<MainWindow>(argc, argv);
}