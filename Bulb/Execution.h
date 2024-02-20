#pragma once

#include <vector>
#include <string>

struct Execution {
    std::string instruction;
    union {
        std::vector<Execution> parameters;
        std::string value;
    };
};