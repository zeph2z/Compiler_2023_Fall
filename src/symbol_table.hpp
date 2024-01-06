#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

class SymbolInfo {
    public:
        std::string type;
        bool is_const;
        int value;
};

extern std::unordered_map<std::string, SymbolInfo> SymbolTable;