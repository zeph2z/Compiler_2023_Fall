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
        int level;
};

class SymbolTableNode {
    public:
        std::unordered_map<std::string, SymbolInfo> table;
        std::shared_ptr<SymbolTableNode> parent;
};

extern std::shared_ptr<SymbolTableNode> CurrentSymbolTable;