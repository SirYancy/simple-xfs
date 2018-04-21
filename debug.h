#include <iostream>

#define DEBUG true

#define debug_print(var) do { \
    if (DEBUG) { std::cout << "DBG: " << __FILE__ << "(" << __LINE__ << ") "\
        << var << std::endl;}} while (0)
