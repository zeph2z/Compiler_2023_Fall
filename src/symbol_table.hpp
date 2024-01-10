#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class SymbolInfo {
    public:
        std::string type;
        std::string name;
        std::vector<std::string> param;
        bool is_const;
        int value;
        int level;
};

class SymbolTableNode;

extern std::shared_ptr<SymbolTableNode> CurrentSymbolTable, FuncSymbolTable, GlobalSymbolTable;
extern std::unordered_map<std::string, SymbolInfo> FuncTable;

class SymbolTableNode {
    public:
        std::unordered_map<std::string, SymbolInfo> table;
        std::shared_ptr<SymbolTableNode> parent = GlobalSymbolTable;
};