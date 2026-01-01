#ifndef PROBLEM_BOX_H
#define PROBLEM_BOX_H

#include <gtkmm.h>
#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>          // Required for Images
#include <gdkmm/general.h>         // Required for set_source_pixbuf
#include "../lyu_and_yu_parser.h"
#include "../problem_definitions.h"
#include "../alns_definitions.h"
#include <gtkmm/tooltip.h>
#include <iomanip>
#include <string>
#include <iostream>

class ProblemCanvas : public Gtk::DrawingArea {
public:
    const PDPTWT* problem = nullptr;
    const PDPTWT_solution* solution = nullptr;
    const Node* m_nodes = nullptr;

    // --- ICON STORAGE ---
    Glib::RefPtr<Gdk::Pixbuf> icon_depot;
    Glib::RefPtr<Gdk::Pixbuf> icon_pickup;
    Glib::RefPtr<Gdk::Pixbuf> icon_delivery;
    Glib::RefPtr<Gdk::Pixbuf> icon_transfer;

    double min_x = 0, max_x = 100;
    double min_y = 0, max_y = 100;

    ProblemCanvas() {
        set_draw_func(sigc::mem_fun(*this, &ProblemCanvas::on_draw));
        set_has_tooltip(true);
        signal_query_tooltip().connect(
            [this](int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
                return on_tooltip(x, y, keyboard_tooltip, tooltip);
            }, false
        );

        // --- LOAD ICONS (Scale to 20x20 pixels) ---
        int icon_size = 20; 
        string root_directory = "Assets/";
        icon_depot    = load_icon(root_directory + "Depot.png", icon_size);
        icon_pickup   = load_icon(root_directory + "Pickup.png", icon_size);
        icon_delivery = load_icon(root_directory + "Delivery.png", icon_size);
        icon_transfer = load_icon(root_directory + "Transfer.png", icon_size);
    }

    // Helper: Safely load and then TINT to white
    Glib::RefPtr<Gdk::Pixbuf> load_icon(std::string filename, int size) {
        try {
            auto pixbuf = Gdk::Pixbuf::create_from_file(filename, size, size, true);
            
            // --- NEW: Convert Black Icons to White ---
            tint_pixbuf_white(pixbuf);
            
            return pixbuf;
        } catch (...) {
            std::cerr << "Warning: Could not load " << filename << std::endl;
            return Glib::RefPtr<Gdk::Pixbuf>(); 
        }
    }

    // --- NEW FUNCTION: PIXEL MANIPULATION ---
    // Turns any colored pixels to Pure White, keeping Alpha transparency
    void tint_pixbuf_white(Glib::RefPtr<Gdk::Pixbuf>& pixbuf) {
        if (!pixbuf) return;
        
        // Ensure it has an alpha channel (RGBA)
        if (pixbuf->get_n_channels() != 4) return;

        int width = pixbuf->get_width();
        int height = pixbuf->get_height();
        int rowstride = pixbuf->get_rowstride();
        guint8* pixels = pixbuf->get_pixels();

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Get pointer to pixel (R, G, B, A)
                guint8* p = pixels + y * rowstride + x * 4;
                
                // If the pixel is not completely transparent
                if (p[3] > 0) {
                    p[0] = 255; // Red -> Max
                    p[1] = 255; // Green -> Max
                    p[2] = 255; // Blue -> Max
                    // p[3] (Alpha) stays the same
                }
            }
        }
    }

    void load_data(const PDPTWT& p) {
        problem = &p;
        m_nodes = p.nodes;
        solution = nullptr; 

        min_x = m_nodes[0].x; max_x = m_nodes[0].x;
        min_y = m_nodes[0].y; max_y = m_nodes[0].y;

        for (int i = 1; i < p.node_amount; i++) {
            if (m_nodes[i].x < min_x) min_x = m_nodes[i].x;
            if (m_nodes[i].x > max_x) max_x = m_nodes[i].x;
            if (m_nodes[i].y < min_y) min_y = m_nodes[i].y;
            if (m_nodes[i].y > max_y) max_y = m_nodes[i].y;
        }
        queue_draw();
    }

    void load_solution(const PDPTWT_solution& s) {
        solution = &s;
        queue_draw();
    }

protected:
    std::tuple<double, double, double> get_vehicle_base_color(int index) {
        static std::vector<std::tuple<double, double, double>> colors = {
            {0.9, 0.2, 0.2}, {0.2, 0.9, 0.2}, {0.3, 0.5, 1.0}, 
            {1.0, 0.7, 0.0}, {0.8, 0.2, 0.8}, {0.2, 0.9, 0.9} 
        };
        return colors[index % colors.size()];
    }

    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
        // 1. Background
        cr->set_source_rgb(0.2, 0.2, 0.2); 
        cr->paint();

        if(problem == nullptr || m_nodes == nullptr) return;

        // --- SCALING MATH ---
        double padding = 20.0;
        double usable_width = width - 2 * padding;
        double usable_height = height - 2 * padding;
        double data_width = max_x - min_x;
        double data_height = max_y - min_y;
        
        if (data_width <= 0) data_width = 1;
        if (data_height <= 0) data_height = 1;

        double scale_x = usable_width / data_width;
        double scale_y = usable_height / data_height;
        double scale = std::min(scale_x, scale_y);

        double offset_x = padding + (usable_width - (data_width * scale)) / 2.0;
        double offset_y = padding + (usable_height - (data_height * scale)) / 2.0;

        auto to_screen_x = [&](double val) { return offset_x + (val - min_x) * scale; };
        auto to_screen_y = [&](double val) { return offset_y + (val - min_y) * scale; };

        // --- 2. DRAW ROUTES (GRADIENT LINES) ---
        if (solution != nullptr) {
            cr->set_line_width(2.5);
            cr->set_line_cap(Cairo::Context::LineCap::ROUND);

            int vehicle_idx = 0;
            for (const auto& route : solution->routes) {
                if (route.stops.size() < 2) {
                    vehicle_idx++;
                    continue;
                }

                auto [r, g, b] = get_vehicle_base_color(vehicle_idx++);
                size_t total_segments = route.stops.size() - 1;

                for (size_t i = 0; i < total_segments; i++) {
                    const Node* n1 = route.stops[i].node;
                    const Node* n2 = route.stops[i+1].node;

                    double x1 = to_screen_x(n1->x);
                    double y1 = to_screen_y(n1->y);
                    double x2 = to_screen_x(n2->x);
                    double y2 = to_screen_y(n2->y);

                    // Gradient alpha for directionality
                    double t_start = (double)i / total_segments;
                    double t_end   = (double)(i + 1) / total_segments;
                    double alpha_start = 0.3 + (0.7 * t_start);
                    double alpha_end   = 0.3 + (0.7 * t_end);

                    auto pattern = Cairo::LinearGradient::create(x1, y1, x2, y2);
                    pattern->add_color_stop_rgba(0.0, r, g, b, alpha_start);
                    pattern->add_color_stop_rgba(1.0, r, g, b, alpha_end);

                    cr->set_source(pattern);
                    cr->move_to(x1, y1);
                    cr->line_to(x2, y2);
                    cr->stroke();
                }
            }
        }

        // --- 3. DRAW NODES (ICONS OR SHAPES) ---
        for (int i = 0; i < problem->node_amount; i++) {
            double sx = to_screen_x(m_nodes[i].x);
            double sy = to_screen_y(m_nodes[i].y);

            // Determine correct icon
            Glib::RefPtr<Gdk::Pixbuf> current_icon;
            switch(m_nodes[i].type) {
                case 'p': current_icon = icon_pickup; break;
                case 'd': current_icon = icon_delivery; break;
                case 't': current_icon = icon_transfer; break;
                case 'o': 
                case 'e': current_icon = icon_depot; break;
            }

            if (current_icon) {
                // DRAW ICON (Centered)
                int w = current_icon->get_width();
                int h = current_icon->get_height();
                Gdk::Cairo::set_source_pixbuf(cr, current_icon, sx - (w/2.0), sy - (h/2.0));
                cr->paint();
            }
        }
    }

    bool on_tooltip(int mouse_x, int mouse_y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
        if (problem == nullptr || m_nodes == nullptr) return false;

        // --- Coordinate Math (Exact Copy) ---
        double width = get_width(); double height = get_height();
        double padding = 20.0;
        double usable_width = width - 2 * padding;
        double usable_height = height - 2 * padding;
        double data_width = max_x - min_x;
        double data_height = max_y - min_y;
        if (data_width <= 0) data_width = 1; if (data_height <= 0) data_height = 1;
        double scale = std::min(usable_width / data_width, usable_height / data_height);
        double offset_x = padding + (usable_width - (data_width * scale)) / 2.0;
        double offset_y = padding + (usable_height - (data_height * scale)) / 2.0;
        auto to_screen_x = [&](double val) { return offset_x + (val - min_x) * scale; };
        auto to_screen_y = [&](double val) { return offset_y + (val - min_y) * scale; };

        // --- 1. CHECK NODES ---
        for (int i = 0; i < problem->node_amount; i++) {
            double sx = to_screen_x(m_nodes[i].x);
            double sy = to_screen_y(m_nodes[i].y);
            double dx = mouse_x - sx;
            double dy = mouse_y - sy;

            if ((dx*dx + dy*dy) < 100.0) { 
                auto get_type_name = [](char type) -> std::string {
                  switch(type) {
                      case 'd': return "Delivery"; case 'p': return "Pickup";
                      case 'o': return "Origin";   case 'e': return "Destination";
                      default:  return "Transshipment";
                  }
                };
                
                std::stringstream ss;
                ss << "ID: " << m_nodes[i].id << 
                      "\nType: " << get_type_name(m_nodes[i].type) <<
                      "\nPosition: (" << m_nodes[i].x << ", " << m_nodes[i].y << ")" <<
                      "\nTime Window: " << m_nodes[i].earliest_tw << " - " << m_nodes[i].latest_tw;

                if (solution != nullptr) {
                    bool found = false;
                    for (int v = 0; v < solution->routes.size(); ++v) {
                        const auto& route = solution->routes[v];
                        for (const auto& stop : route.stops) {
                            if (stop.node->id == m_nodes[i].id) {
                                if(!found) ss << "\n----------------"; 
                                double cost = route.get_arrival_time(stop);
                                ss << "\nVehicle " << v << ": " << std::fixed << std::setprecision(2) << cost;
                                found = true;
                            }
                        }
                    }
                    if (!found && !solution->routes.empty()) ss << "\n(Unvisited)";
                }
                tooltip->set_text(ss.str());
                tooltip->set_tip_area(Gdk::Rectangle(sx - 5, sy - 5, 10, 10));
                return true; 
            }
        }

        // --- 2. CHECK LINES ---
        if (solution == nullptr) return false;

        for (int v = 0; v < solution->routes.size(); ++v) {
            const auto& route = solution->routes[v];
            if (route.stops.size() < 2) continue;

            for (size_t i = 0; i < route.stops.size() - 1; ++i) {
                double x1 = to_screen_x(route.stops[i].node->x);
                double y1 = to_screen_y(route.stops[i].node->y);
                double x2 = to_screen_x(route.stops[i+1].node->x);
                double y2 = to_screen_y(route.stops[i+1].node->y);

                double px = x2 - x1; double py = y2 - y1;
                double norm = px*px + py*py;
                if (norm == 0) continue;

                double u = ((mouse_x - x1) * px + (mouse_y - y1) * py) / norm;

                if (u > 0.0 && u < 1.0) { 
                    double ix = x1 + u * px;
                    double iy = y1 + u * py;
                    double dist_sq = (mouse_x - ix)*(mouse_x - ix) + (mouse_y - iy)*(mouse_y - iy);
                    
                    if (dist_sq < 25.0) { 
                        double cost = problem->get_distance(route.stops[i].node, route.stops[i+1].node);
                        std::stringstream ss;
                        ss << "Veh: " << v << "\n" 
                           << route.stops[i].node->id << " -> " << route.stops[i+1].node->id << "\n"
                           << "Cost: " << std::fixed << std::setprecision(2) << cost;
                        
                        tooltip->set_text(ss.str());
                        tooltip->set_tip_area(Gdk::Rectangle(mouse_x - 2, mouse_y - 2, 4, 4));
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

#endif