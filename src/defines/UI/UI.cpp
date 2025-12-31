#include <gtkmm.h>
#include "../lyu_and_yu_parser.h"
#include "../problem_definitions.h"
#include "../alns_definitions.h"

//g++ main.cpp -o a.out `pkg-config --cflags --libs gtkmm-4.0`


class MainWindow : public Gtk::Window {
public:
    MainWindow() {
        // 1. Window Setup
        set_title("PDPTW-T Solver");
        set_default_size(800, 600);

        // 2. Layout Container (Vertical Box)
        // GTK4 uses specific namespaces for enums (Orientation::VERTICAL)
        m_box.set_orientation(Gtk::Orientation::VERTICAL);
        m_box.set_spacing(10);
        m_box.set_margin_top(20);
        m_box.set_margin_bottom(20);
        m_box.set_margin_start(20);
        m_box.set_margin_end(20);
        
        // In GTK4, Window has a 'set_child' method instead of 'add'
        set_child(m_box);

        // 3. The Button to trigger File Selection
        file_select.set_label("Select Input File");
        
        // Connect the button click to our handler
        file_select.signal_clicked().connect(
            sigc::mem_fun(*this, &MainWindow::on_button_clicked)
        );

        // 4. The Label (To display the path)
        m_label.set_text("No file selected yet.");
        m_box.append(m_label);

        // In GTK4, we use 'append' instead of 'pack_start'
        m_box.append(file_select);

        

        // 5. Prepare the File Chooser Dialog (Native System Dialog)
        // We create it now, but we only show it when the button is clicked.
        m_file_dialog = Gtk::FileChooserNative::create(
            "Please choose a file",
            *this,
            Gtk::FileChooser::Action::OPEN,
            "Open",
            "Cancel"
        );

        // Connect the dialog response (User clicked Open or Cancel)
        m_file_dialog->signal_response().connect(
            sigc::mem_fun(*this, &MainWindow::on_file_dialog_response)
        );
    }

protected:
    // Triggered when the "Select Input File" button is clicked
    void on_button_clicked() {
        m_file_dialog->show();
    }

    // Triggered when the user finishes with the File Dialog
    void on_file_dialog_response(int response_id) {
        // Check if the user clicked "Open" (ACCEPT)
        if (response_id == Gtk::ResponseType::ACCEPT) {
            auto file = m_file_dialog->get_file();
            if (file) {
                std::string filename = file->get_path();
                m_label.set_text("Selected: " + filename);
                current_problem = parse(filename);
            }
        }
        // Hide the dialog after use
        m_file_dialog->hide();
    }

    // Member widgets
    Gtk::Box m_box;
    Gtk::Label m_label;
    Gtk::Button file_select;
    Glib::RefPtr<Gtk::FileChooserNative> m_file_dialog;
    PDPTWT current_problem;

};

int main(int argc, char *argv[]) {
    // GTK4 Application::create signature is different (no argc/argv)
    auto app = Gtk::Application::create("org.gtkmm.example.pdptwt");
    
    return app->make_window_and_run<MainWindow>(argc, argv);
}