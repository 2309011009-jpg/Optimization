#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem> // Requires C++17
#include <map>
#include <chrono>
#include <iomanip>

// Include your existing project headers
#include "../defines/problem_definitions.h"
#include "../defines/alns_definitions.h"
#include "../defines/lyu_and_yu_parser.h"
#include "../defines/initial_solution.h"
#include "../defines/greedy_insertion.h"
#include "../defines/random_removal.h"
#include "../defines/dummy_visitor.h"

// Include ALNS Library Headers
#include "../../libraries/adaptive-large-neighbourhood-search/src/PALNS.h"
#include "../../libraries/adaptive-large-neighbourhood-search/src/Parameters.h"

namespace fs = std::filesystem;
using namespace mlpalns;
