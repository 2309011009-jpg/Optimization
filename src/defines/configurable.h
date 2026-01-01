// Configurable.h
#ifndef CONFIGURABLE_H
#define CONFIGURABLE_H

#include <string>
#include <map>

struct Parameter {
    std::string name;
    double value;
    double min;
    double max;
    double step;
    
    // Helper to get integer value easily
    int as_int() const { return static_cast<int>(value); }
};

class Configurable {
public:
    // Store parameters by name
    std::map<std::string, Parameter> params;

    void add_parameter(std::string name, double default_val, double min, double max, double step = 1.0) {
        params[name] = {name, default_val, min, max, step};
    }

    // Helper for algorithms to read values
    double get_param(std::string name) {
        if (params.count(name)) return params[name].value;
        return 0.0; // Should not happen if configured correctly
    }
    
    int get_int_param(std::string name) {
        return static_cast<int>(get_param(name));
    }

    // Called by GUI to update value
    void set_param_value(std::string name, double val) {
        if (params.count(name)) {
            params[name].value = val;
        }
    }
    
    // Virtual destructor usually good practice
    virtual ~Configurable() = default;
};

#endif