#pragma once

#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <algorithm>
#include "symbol_table.hpp"

extern bool must_return, branch, func_is_void, has_left, global, need_load, as_para;
extern int cnt, level, block_cnt, logic_cnt, array_cnt, loc, depth;
extern std::string kstr, last_br, true_block_name, false_block_name;
extern std::stack<int> while_stack, current_depth, array_depth;
extern std::vector<int> array_temp, array_shape;
extern std::stack<std::string> current_ptr;

extern std::ostream& operator<<(std::ostream& os, const SymbolInfo& info);
extern bool last_is_br();
extern void array_generate_loc(int _depth), array_generate_glb(int _depth);

class BaseAST {
    public:
        bool _ptr = false;
        std::string label, rabel, pass_ident;
        int value;
        std::unique_ptr<BaseAST> next = nullptr;

        virtual void Generate(bool write = true) = 0;
        virtual void AddtoSymbolTable(std::string str, bool is_const) = 0;

        virtual ~BaseAST() = default;
};

// CompUnit ::= (ConstDecl | FuncType IDENT RestCompUnit) [CompUnit];
class CompUnitAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> const_decl;
        std::unique_ptr<BaseAST> func_type;
        std::string ident;
        std::unique_ptr<BaseAST> rest_comp_unit;

        void Generate(bool write = true) override {
            if (type == 0) {
                global = true;
                CurrentSymbolTable = GlobalSymbolTable;
                const_decl->Generate(false);
                global = false;
                CurrentSymbolTable = nullptr;
            }
            else if (type == 1) {
                rest_comp_unit->label = func_type->label;
                rest_comp_unit->pass_ident = ident;
                rest_comp_unit->Generate(write);
                kstr += "\n";
            }
            if (next) next->Generate(write);
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// RestCompUnit ::= "(" [FuncFParams] ")" Block | [ConstArray] ["=" InitVal] {"," VarDef} ";";
class RestCompUnitAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> init_val;
        std::unique_ptr<BaseAST> func_f_params;
        std::unique_ptr<BaseAST> block;
        std::unique_ptr<BaseAST> var_def;
        std::unique_ptr<BaseAST> const_array;

        void Generate(bool write = true) override {
            if (type == 0) {
                FuncSymbolTable.reset(new SymbolTableNode());
                CurrentSymbolTable.reset();
                level = 0;
                last_br.clear();
                kstr += "fun @" + pass_ident + "(";
                if (func_f_params) func_f_params->Generate(write);
                kstr += ")";

                if (label == "void") func_is_void = true;
                else func_is_void = false;
                if (label == "int") kstr += ": i32";

                FuncTable[pass_ident].type = label;
                for (const auto& pair : FuncSymbolTable->table) {
                    std::cout << pair.first << " "  << pair.second << std::endl;
                }

                kstr += " {\n%entry:\n";
                block->Generate(write);
                if (func_is_void && !last_is_br()) kstr += "    ret\n"; 
                kstr += "}\n\n";
            }
            else if (type == 1) {
                if (const_array) {
                    CurrentSymbolTable = GlobalSymbolTable;
                    SymbolInfo info;
                    info.name = pass_ident;

                    kstr += "global @" + info.name + " = alloc ";
                    const_array->Generate(true);
                    kstr += const_array->label + ", ";

                    info.type = const_array->label;
                    info.value = 0;
                    info.is_const = false;
                    info.level = 0;

                    GlobalSymbolTable->table[pass_ident] = info;

                    array_temp.clear();
                    array_shape.clear();
                    int size = 1;
                    loc = 0, depth = -1;

                    while (!array_depth.empty()) {
                        size *= array_depth.top();
                        array_shape.push_back(array_depth.top());
                        array_depth.pop();
                    }
                    array_temp = std::vector<int>(size, 0);
                    std::reverse(array_shape.begin(), array_shape.end());

                    if (init_val) {
                        kstr += "{";
                        init_val->Generate(false);
                        loc = 0;
                        array_generate_glb(0);
                        kstr += "}";
                    }
                    else kstr += "zeroinit";
                    kstr += "\n";

                    global = true;
                    if (var_def) var_def->AddtoSymbolTable(label, false);
                    global = false;
                    CurrentSymbolTable = nullptr;
                    kstr += "\n";
                }
                else {
                    CurrentSymbolTable = GlobalSymbolTable;
                    SymbolInfo info;
                    info.type = label;
                    info.value = 0;
                    info.is_const = false;
                    info.level = 0;
                    info.name = pass_ident;
                    if (init_val) {
                        has_left = true;
                        init_val->Generate(false);
                        info.value = init_val->value;
                        has_left = false;
                    }
                    GlobalSymbolTable->table[pass_ident] = info;
                    kstr += "global @" + info.name + " = alloc ";
                    if (info.type == "int")
                        kstr += "i32";
                    if (init_val) kstr += ", " + std::to_string(init_val->value) + "\n";
                    else kstr += ", zeroinit\n";

                    global = true;
                    if (var_def) var_def->AddtoSymbolTable(label, false);
                    global = false;
                    CurrentSymbolTable = nullptr;
                    kstr += "\n";
                }
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// FuncType ::= BType;
class FuncTypeAST : public BaseAST {
    public:
        void Generate(bool write = true) override {
            if (label == "int") kstr += "i32 ";
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// Block ::= "{" {BlockItem} "}";
class BlockAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> block_item;

        void Generate(bool write = true) override {
            level++;
            if (CurrentSymbolTable == nullptr) {
                CurrentSymbolTable = std::make_shared<SymbolTableNode>();
                for (auto& pair : FuncSymbolTable->table) {
                    if (pair.second.type == "int") {
                        kstr += "    @" + pair.first + "_1" + " = alloc " + pair.second.type + "\n";
                        kstr += "    store @" + pair.first + ", @" + pair.first + "_1" + "\n";
                    }
                    else {
                        kstr += "    %" + pair.first + " = alloc " + pair.second.type + "\n";
                        kstr += "    store @" + pair.first + ", %" + pair.first + "\n";
                        kstr += "    @" + pair.first + "_1" + " = load %" + pair.first + "\n";
                        pair.second.is_const = true;
                    }
                    CurrentSymbolTable->table.insert(pair);
                }
                FuncSymbolTable.reset();
            }
            else {
                std::shared_ptr<SymbolTableNode> new_table = std::make_shared<SymbolTableNode>();
                new_table->parent = CurrentSymbolTable;
                CurrentSymbolTable = new_table;
            }
            if (block_item) block_item->Generate(write);
            CurrentSymbolTable = CurrentSymbolTable->parent;
            must_return = false;
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// BlockItem ::= Decl | Stmt;
class BlockItemAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> decl = nullptr;
        std::unique_ptr<BaseAST> stmt = nullptr;

        void Generate(bool write = true) override {
            if (!last_is_br()) {
                if (decl)
                    decl->Generate(write);
                else if (stmt)
                    stmt->Generate(write);
                if (next) {
                    next->Generate(write);
                }
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// Stmt ::= "return" [Exp] ";" | LVal "=" Exp ";" | [Exp] ';' | Block;
// Stmt ::= "if" "(" Exp ")" Stmt ["else" Stmt];
// Stmt ::= "while" "(" Exp ")" Stmt | break ";" | continue ";";
class StmtAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> l_val;
        std::unique_ptr<BaseAST> exp;
        std::unique_ptr<BaseAST> block;
        std::unique_ptr<BaseAST> stmt;
        std::unique_ptr<BaseAST> else_stmt;

        void Generate(bool write = true) override {
            if (type == 0) {
                if (exp) {
                    has_left = true;
                    exp->Generate(true);
                    has_left = false;
                    kstr += "    ret " + exp->label + "\n";
                }
                else
                    kstr += "    ret\n";
                last_br = "ret";
            }
            else if (type == 1) {
                has_left = true;
                exp->Generate(true);
                has_left = false;
                auto it = CurrentSymbolTable;
                while (it) {
                    auto it2 = it->table.find(l_val->label);
                    if (it2 != it->table.end()) {
                        need_load = false;
                        l_val->Generate(true);
                        need_load = true;
                        kstr += "    store " + exp->label + ", " + l_val->rabel + "\n";
                        break;
                    }
                    it = it->parent;
                }
            }
            else if (type == 2) {
                if (exp) {
                    exp->Generate(true);
                }
                else
                    kstr += "\n";
            }
            else if (type == 3) {
                block->Generate(write);
            }
            else if (type == 4) {
                branch = true;
                int _block_cnt = block_cnt++;
                if (else_stmt) {
                    // br %0, %then_0, %else_0
                    true_block_name = "%then_" + std::to_string(_block_cnt);
                    false_block_name = "%else_" + std::to_string(_block_cnt);
                    exp->Generate(true);
                    branch = false;

                    kstr += "    br " + exp->label + ", " + "%then_" + std::to_string(_block_cnt) + ", " + "%else_" + std::to_string(_block_cnt) + "\n\n";
                    
                    kstr += "%then_" + std::to_string(_block_cnt) + ":\n";
                    last_br.clear();
                    stmt->Generate(true);
                    bool first_is_ret = last_br == "ret";
                    if (!last_is_br()) kstr += "    jump %end_" + std::to_string(_block_cnt) + "\n"; 
                    
                    kstr += "\n%else_" + std::to_string(_block_cnt) + ":\n";
                    last_br.clear();
                    else_stmt->Generate(true);
                    bool second_is_ret = last_br == "ret";
                    if (!last_is_br()) kstr += "    jump %end_" + std::to_string(_block_cnt) + "\n";
                    
                    must_return = first_is_ret && second_is_ret;
                    if (!must_return) kstr += "\n%end_" + std::to_string(_block_cnt) + ":\n";
                }
                else {
                    true_block_name = "%then_" + std::to_string(_block_cnt);
                    false_block_name = "%end_" + std::to_string(_block_cnt);
                    exp->Generate(true);
                    branch = false;

                    kstr += "    br " + exp->label + ", " + "%then_" + std::to_string(_block_cnt) + ", " + "%end_" + std::to_string(_block_cnt) + "\n\n";
                    kstr += "%then_" + std::to_string(_block_cnt) + ":\n";
                    last_br.clear();
                    stmt->Generate(true);
                    if (!last_is_br()) kstr += "    jump %end_" + std::to_string(_block_cnt) + "\n\n"; 
                    
                    kstr += "\n%end_" + std::to_string(_block_cnt) + ":\n";
                    last_br.clear();
                }
            }
            else if (type == 5) {
                int _block_cnt = block_cnt++;
                while_stack.push(_block_cnt);
                kstr += "    jump %while_entry_" + std::to_string(_block_cnt) + "\n\n";
            
                kstr += "%while_entry_" + std::to_string(_block_cnt) + ":\n";
                true_block_name = "%while_body_" + std::to_string(_block_cnt);
                false_block_name = "%end_" + std::to_string(_block_cnt);
                branch = true;
                exp->Generate(true);
                branch = false;
                kstr += "    br " + exp->label + ", " + "%while_body_" + std::to_string(_block_cnt) + ", " + "%end_" + std::to_string(_block_cnt) + "\n\n";
            
                kstr += "%while_body_" + std::to_string(_block_cnt) + ":\n";
                last_br.clear();
                stmt->Generate(true);
                if (!last_is_br()) kstr += "    jump %while_entry_" + std::to_string(_block_cnt) + "\n\n";

                kstr += "%end_" + std::to_string(_block_cnt) + ":\n";
                last_br.clear();
                while_stack.pop();
            }
            else if (type == 6) {
                int _block_cnt = while_stack.top();
                kstr += "    jump %end_" + std::to_string(_block_cnt) + "\n\n";
                last_br = "jump";
            }
            else if (type == 7) {
                int _block_cnt = while_stack.top();
                kstr += "    jump %while_entry_" + std::to_string(_block_cnt) + "\n\n";
                last_br = "jump";
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// Exp ::= LOrExp;
class ExpAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> lor_exp;

        void Generate(bool write = true) override {
            lor_exp->Generate(write);
            if (write) label = lor_exp->label;
            value = lor_exp->value;
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp;
// UnaryExp ::= IDENT "(" [FuncRParams] ")";
class UnaryExpAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> primary_exp;
        char op;
        std::unique_ptr<BaseAST> unary_exp;
        std::unique_ptr<BaseAST> func_r_params;
        std::string ident;

        void Generate(bool write = true) override {
            if (type == 0) {
                primary_exp->Generate(write);
                if (write) label = primary_exp->label;
                value = primary_exp->value;
            }
            else if (type == 1) {
                unary_exp->Generate(write);
                switch (op) {
                    case '+' : {
                        if (write) label = unary_exp->label; 
                        value = unary_exp->value;
                        break;
                    }
                    case '-' : {
                        if (write) {
                            label = "%" + std::to_string(cnt++);
                            // %2 = sub 0, %1
                            kstr += "    " + label + " = sub 0, " + unary_exp->label + "\n";
                        }
                        value = -unary_exp->value;
                        break;
                    }
                    case '!' : {
                        if (write) {
                            label = "%" + std::to_string(cnt++);
                            // %0 = eq 6, 0
                            kstr += "    " + label + " = eq " + unary_exp->label + ", 0\n";
                        }
                        value = !unary_exp->value;
                        break;
                    }
                }
            }
            else if (type == 2) {
                as_para = true;
                if (func_r_params) func_r_params->Generate(write);
                as_para = false;
                if (write) {
                    std::string func_type = FuncTable[ident].type;
                    if (has_left || func_type == "int") {
                        label = "%" + std::to_string(cnt++);
                        kstr += "    " + label + " = call @" + ident + "(";
                    }
                    else
                        kstr += "    call @" + ident + "(";
                    if (func_r_params) kstr += func_r_params->label;
                    kstr += ")\n";
                }
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// PrimaryExp ::= "(" Exp ")" | LVal | Number;
class PrimaryExpAST : public BaseAST {
    public:
        int type;
        int number;
        std::unique_ptr<BaseAST> exp, l_val;

        void Generate(bool write = true) override {
            if (type == 0) {
                exp->Generate(write);
                if (write) label = exp->label;
                value = exp->value;
            }
            else if (type == 1) {
                l_val->Generate(write);
                if (write) label = l_val->label;
                value = l_val->value;
            }
            else if (type == 2) {
                if (write) label = std::to_string(number);
                value = number;
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// AddExp ::= MulExp | AddExp ("+" | "-") MulExp;
class AddExpAST : public BaseAST {
    public:
        int type;
        char op;
        std::unique_ptr<BaseAST> mul_exp;
        std::unique_ptr<BaseAST> add_exp;

        void Generate(bool write = true) override {
            if (type == 0) {
                mul_exp->Generate(write);
                if (write) label = mul_exp->label;
                value = mul_exp->value;
            }
            else if (type == 1) {
                add_exp->Generate(write);
                mul_exp->Generate(write);
                if (write) label = "%" + std::to_string(cnt++);
                switch (op) {
                    case '+' : {
                        if (write) {
                            // %1 = add %0, %2
                            kstr += "    " + label + " = add " + add_exp->label + ", " + mul_exp->label + "\n";
                        }
                        value = add_exp->value + mul_exp->value;
                        break;
                    }
                    case '-' : {
                        if (write) {
                            // %1 = sub %0, %2
                            kstr += "    " + label + " = sub " + add_exp->label + ", " + mul_exp->label + "\n";
                        }
                        value = add_exp->value - mul_exp->value;
                        break;
                    }
                }
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// MulExp ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
class MulExpAST : public BaseAST {
    public:
        int type;
        char op;
        std::unique_ptr<BaseAST> unary_exp;
        std::unique_ptr<BaseAST> mul_exp;

        void Generate(bool write = true) override {
            if (type == 0) {
                unary_exp->Generate(write);
                if (write) label = unary_exp->label;
                value = unary_exp->value;
            }
            else if (type == 1) {
                mul_exp->Generate(write);
                unary_exp->Generate(write);
                if (write) label = "%" + std::to_string(cnt++);
                switch (op) {
                    case '*' : {
                        if (write) {
                            // %1 = mul %0, %2
                            kstr += "    " + label + " = mul " + mul_exp->label + ", " + unary_exp->label + "\n";
                        }
                        value = mul_exp->value * unary_exp->value;
                        break;
                    }
                    case '/' : {
                        if (write) {
                            // %1 = div %0, %2
                            kstr += "    " + label + " = div " + mul_exp->label + ", " + unary_exp->label + "\n";
                        }
                        if (unary_exp->value == 0) value = 0;
                        else value = mul_exp->value / unary_exp->value;
                        break;
                        
                    }
                    case '%' : {
                        if (write) {
                            // %1 = mod %0, %2
                            kstr += "    " + label + " = mod " + mul_exp->label + ", " + unary_exp->label + "\n";
                        }
                        value = mul_exp->value % unary_exp->value;
                        break;
                    }
                }
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// RelExp ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
class RelExpAST : public BaseAST {
    public:
        int type;
        std::string op;
        std::unique_ptr<BaseAST> add_exp;
        std::unique_ptr<BaseAST> rel_exp;

        void Generate(bool write = true) override {
            if (type == 0) {
                add_exp->Generate(write);
                if (write) label = add_exp->label;
                value = add_exp->value;
            }
            else if (type == 1) {
                rel_exp->Generate(write);
                add_exp->Generate(write);
                if (write) label = "%" + std::to_string(cnt++);
                if (op == "<") {
                    if (write) {
                        // %1 = lt %0, %2
                        kstr += "    " + label + " = lt " + rel_exp->label + ", " + add_exp->label + "\n";
                    }
                    value = rel_exp->value < add_exp->value;
                }
                else if (op == ">") {
                    if (write) {
                        // %1 = gt %0, %2
                        kstr += "    " + label + " = gt " + rel_exp->label + ", " + add_exp->label + "\n";
                    }
                    value = rel_exp->value > add_exp->value;
                }
                else if (op == "<=") {
                    if (write) {
                        // %1 = le %0, %2
                        kstr += "    " + label + " = le " + rel_exp->label + ", " + add_exp->label + "\n";
                    }
                    value = rel_exp->value <= add_exp->value;
                }
                else if (op == ">=") {
                    if (write) {
                        // %1 = sge %0, %2
                        kstr += "    " + label + " = ge " + rel_exp->label + ", " + add_exp->label + "\n";
                    }
                    value = rel_exp->value >= add_exp->value;
                }
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// EqExp ::= RelExp | EqExp ("==" | "!=") RelExp;
class EqExpAST : public BaseAST {
    public:
        int type;
        std::string op;
        std::unique_ptr<BaseAST> rel_exp;
        std::unique_ptr<BaseAST> eq_exp;

        void Generate(bool write = true) override {
            if (type == 0) {
                rel_exp->Generate(write);
                if (write) label = rel_exp->label;
                value = rel_exp->value;
            }
            else if (type == 1) {
                eq_exp->Generate(write);
                rel_exp->Generate(write);
                if (write) label = "%" + std::to_string(cnt++);
                if (op == "==") {
                    if (write) {
                        // %1 = eq %0, %2
                        kstr += "    " + label + " = eq " + eq_exp->label + ", " + rel_exp->label + "\n";
                    }
                    value = eq_exp->value == rel_exp->value;
                }
                else if (op == "!=") {
                    if (write) {
                        // %1 = ne %0, %2
                        kstr += "    " + label + " = ne " + eq_exp->label + ", " + rel_exp->label + "\n";
                    }
                    value = eq_exp->value != rel_exp->value;
                }
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> eq_exp;
        std::unique_ptr<BaseAST> land_exp;

        void Generate(bool write = true) override {
            if (type == 0) {
                eq_exp->Generate(write);
                if (write) label = eq_exp->label;
                value = eq_exp->value;
            }
            else if (type == 1) {
                if (write) {
                    int _logic_cnt = logic_cnt++;
                    std::string label_var = "@temp_" + std::to_string(_logic_cnt);
                    std::string _true = "%true_" + std::to_string(_logic_cnt);
                    std::string _false = "%false_" + std::to_string(_logic_cnt);
                    std::string _final = "%final_" + std::to_string(_logic_cnt);

                    kstr += "    " + label_var + " = alloc i32\n";

                    land_exp->Generate(write);
                    kstr += "    br " + land_exp->label + ", " + _true + ", " + _false + "\n\n";

                    kstr += _true + ":\n";
                    eq_exp->Generate(write);
                    kstr += "    store " + eq_exp->label + ", " + label_var + "\n";
                    kstr += "    jump " + _final + "\n\n";

                    kstr += _false + ":\n";
                    kstr += "    store 0, " + label_var + "\n";
                    kstr += "    jump " + _final + "\n\n";

                    kstr += _final + ":\n";
                    label = "%" + std::to_string(cnt++);
                    kstr += "    %" + std::to_string(cnt++) + " = load " + label_var + "\n";
                    kstr += "    " + label + " = ne 0, %" + std::to_string(cnt - 1) + "\n";
                }
                else {
                    land_exp->Generate(write);
                    eq_exp->Generate(write);
                }
                value = land_exp->value && eq_exp->value;
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// LOrExp ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> land_exp;
        std::unique_ptr<BaseAST> lor_exp;

        void Generate(bool write) override {
            if (type == 0) {
                land_exp->Generate(write);
                if (write) label = land_exp->label;
                value = land_exp->value;
            }
            else if (type == 1) {
                if (write) {
                    int _logic_cnt = logic_cnt++;
                    std::string label_var = "@temp_" + std::to_string(_logic_cnt);
                    std::string _true = "%true_" + std::to_string(_logic_cnt);
                    std::string _false = "%false_" + std::to_string(_logic_cnt);
                    std::string _final = "%final_" + std::to_string(_logic_cnt);

                    kstr += "    " + label_var + " = alloc i32\n";

                    lor_exp->Generate(write);
                    kstr += "    br " + lor_exp->label + ", " + _true + ", " + _false + "\n\n";

                    kstr += _true + ":\n";
                    kstr += "    store 1, " + label_var + "\n";
                    kstr += "    jump " + _final + "\n\n";

                    kstr += _false + ":\n";
                    land_exp->Generate(write);
                    kstr += "    store " + land_exp->label + ", " + label_var + "\n";
                    kstr += "    jump " + _final + "\n\n";

                    kstr += _final + ":\n";
                    label = "%" + std::to_string(cnt++);
                    kstr += "    %" + std::to_string(cnt++) + " = load " + label_var + "\n";
                    kstr += "    " + label + " = ne 0, %" + std::to_string(cnt - 1) + "\n";
                }
                else {
                    lor_exp->Generate(write);
                    land_exp->Generate(write);
                }
                value = lor_exp->value || land_exp->value;
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// Decl ::= ConstDecl | VarDecl;
class DeclAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> const_decl;
        std::unique_ptr<BaseAST> var_decl;

        void Generate(bool write = true) override {
            if (type == 0) const_decl->Generate(false);
            if (type == 1) var_decl->Generate(write);
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// ConstDecl ::= "const" BType ConstDef ";";
class ConstDeclAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> b_type;
        std::unique_ptr<BaseAST> const_def;

        void Generate(bool write = true) override {
            const_def->AddtoSymbolTable(b_type->label, true);
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// ConstDef ::= IDENT [ConstArray] "=" ConstInitVal | IDENT [ConstArray] "=" ConstInitVal "," ConstDef;
class ConstDefAST : public BaseAST {
    public:
        std::string ident;
        std::unique_ptr<BaseAST> const_init_val;
        std::unique_ptr<BaseAST> const_array;

        void Generate(bool write = true) override {}
        void AddtoSymbolTable(std::string str, bool is_const) override {
            if (global) {
                if (const_array) {
                    SymbolInfo info;
                    info.name = ident;

                    kstr += "global @" + info.name + " = alloc ";
                    const_array->Generate(true);
                    kstr += const_array->label + ", ";

                    info.type = const_array->label;
                    info.value = 0;
                    info.is_const = false;
                    info.level = 0;

                    GlobalSymbolTable->table[ident] = info;

                    array_temp.clear();
                    array_shape.clear();
                    int size = 1;
                    loc = 0, depth = -1;

                    while (!array_depth.empty()) {
                        size *= array_depth.top();
                        array_shape.push_back(array_depth.top());
                        array_depth.pop();
                    }
                    array_temp = std::vector<int>(size, 0);
                    std::reverse(array_shape.begin(), array_shape.end());

                    if (const_init_val) {
                        kstr += "{";
                        const_init_val->Generate(false);
                        loc = 0;
                        array_generate_glb(0);
                        kstr += "}";
                    }
                    else kstr += "zeroinit";
                    kstr += "\n\n";
                }
                else {
                    SymbolInfo info;
                    info.type = str;
                    const_init_val->Generate(false);
                    info.value = const_init_val->value;
                    info.is_const = is_const;
                    info.level = 0;
                    info.name = ident;
                    GlobalSymbolTable->table[ident] = info;
                }
            }
            else {
                if (const_array) {
                    SymbolInfo info;
                    info.name = ident + "_" + std::to_string(level);

                    kstr += "    @" + info.name + " = alloc ";
                    const_array->Generate(true);
                    kstr += const_array->label + "\n";

                    info.type = const_array->label;
                    info.value = 0;
                    info.is_const = false;
                    info.level = level;

                    CurrentSymbolTable->table[ident] = info;

                    array_temp.clear();
                    array_shape.clear();
                    int size = 1;
                    loc = 0, depth = -1;

                    while (!array_depth.empty()) {
                        size *= array_depth.top();
                        array_shape.push_back(array_depth.top());
                        array_depth.pop();
                    }
                    array_temp = std::vector<int>(size, 0);
                    std::reverse(array_shape.begin(), array_shape.end());
                    
                    if (const_init_val) {
                        const_init_val->Generate(true);
                    }

                    loc = 0;
                    current_ptr.push("@" + info.name);
                    array_generate_loc(0);
                    current_ptr.pop();                }
                else {
                    SymbolInfo info;
                    info.type = str;
                    const_init_val->Generate(false);
                    info.value = const_init_val->value;
                    info.is_const = is_const;
                    info.level = level;
                    info.name = ident + "_" + std::to_string(level);
                    CurrentSymbolTable->table[ident] = info;
                }
            }
            if (next)
                next->AddtoSymbolTable(str, true);
        }
};

// ConstInitVal ::= ConstExp | "{" [ConstInitVals] "}";
class ConstInitValAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> const_exp;
        std::unique_ptr<BaseAST> const_init_vals;

        void Generate(bool write = true) override {
            if (type == 0) {
                const_exp->Generate(write);
                if (write) label = const_exp->label;
                value = const_exp->value;
            }
            else {                
                value = 810975;
                depth++;
                if (const_init_vals) {
                    const_init_vals->Generate(write);
                }
                depth--;
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// ConstInitVals ::= ConstInitVal | ConstInitVal "," ConstInitVals;
class ConstInitValsAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> const_init_val;

        void Generate(bool write = true) override {
            const_init_val->Generate(write);
            if (const_init_val->value != 810975) array_temp[loc++] = const_init_val->value;
            if (next) {
                next->Generate(write);
            }
            else {
                int _align = 1;
                for (int i = depth; i < array_shape.size(); i++)
                    _align *= array_shape[i];
                while (loc % _align != 0)
                    loc++;
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// ConstExp ::= Exp;
class ConstExpAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;

        void Generate(bool write = true) override {
            exp->Generate(write);
            if (write) label = exp->label;
            value = exp->value;
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// LVal ::= IDENT Array;
class LValAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> array = nullptr;

        void Generate(bool write = true) override {
            if (array) {
                auto it = CurrentSymbolTable;
                while (it) {
                    auto it2 = it->table.find(label);
                    if (it2 != it->table.end()) {
                        label = it2->second.name;
                        array->_ptr = it2->second.is_const;
                        break;
                    }
                    it = it->parent;
                }
                current_ptr.push("@" + label);
                array->Generate(true);
                current_ptr.pop();
                label = "%" + std::to_string(cnt++);
                rabel = array->label;
                if (need_load) {
                    if (!as_para) kstr += "    " + label + " = load " + array->label + "\n";
                    else kstr += "    " + label + " = getelemptr " + array->label + ", 0\n";
                }
            }
            else {
                auto it = CurrentSymbolTable;
                while (it) {
                    auto it2 = it->table.find(label);
                    if (it2 != it->table.end()) {
                        if (it2->second.type != "int") {
                            if (write) {
                                if (it2->second.is_const) kstr += "    %" + std::to_string(cnt++) + " = getptr @" + it2->second.name + "\n";
                                else kstr += "    %" + std::to_string(cnt++) + " = getelemptr @" + it2->second.name + "\n";
                            }
                            label = "%" + std::to_string(cnt - 1);
                            rabel = "@" + it2->second.name;
                            break;
                        }
                        if (it2->second.is_const) {
                            label = std::to_string(it2->second.value);
                            value = it2->second.value;
                        }
                        else {
                            if (write) {
                                if (need_load) kstr += "    %" + std::to_string(cnt++) + " = load @" + it2->second.name + "\n"; 
                                label = "%" + std::to_string(cnt - 1);
                                rabel = "@" + it2->second.name;
                            }
                            value = it2->second.value;
                        }
                        break;
                    }
                    it = it->parent;
                }
                if (!it) label = "err";
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// BType ::= INT;
class BTypeAST : public BaseAST {
    public:
        void Generate(bool write = true) override {}
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// VarDecl ::= BType VarDef ";";
class VarDeclAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> b_type;
        std::unique_ptr<BaseAST> var_def;

        void Generate(bool write = true) override {
            var_def->AddtoSymbolTable(b_type->label, false);
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// VarDef ::= IDENT ConstArray | IDENT "=" InitVal | IDENT ',' VarDef | IDENT "=" InitVal "," VarDef;
class VarDefAST : public BaseAST {
    public:
        std::string ident;
        std::unique_ptr<BaseAST> init_val = nullptr;
        std::unique_ptr<BaseAST> const_array = nullptr;

        void Generate(bool write = true) override {}
        void AddtoSymbolTable(std::string str, bool is_const) override {
            if (global) {
                if (const_array) {
                    SymbolInfo info;
                    info.name = ident;

                    kstr += "global @" + info.name + " = alloc ";
                    const_array->Generate(true);
                    kstr += const_array->label + ", ";

                    info.type = const_array->label;
                    info.value = 0;
                    info.is_const = is_const;
                    info.level = 0;
                    GlobalSymbolTable->table[ident] = info;

                    array_temp.clear();
                    array_shape.clear();
                    int size = 1;
                    loc = 0, depth = -1;

                    while (!array_depth.empty()) {
                        size *= array_depth.top();
                        array_shape.push_back(array_depth.top());
                        array_depth.pop();
                    }
                    array_temp = std::vector<int>(size, 0);
                    std::reverse(array_shape.begin(), array_shape.end());

                    if (init_val) {
                        kstr += "{";
                        init_val->Generate(false);
                        loc = 0;
                        array_generate_glb(0);
                        kstr += "}";
                    }
                    else kstr += "zeroinit";
                    kstr += "\n";
                }
                else {
                    SymbolInfo info;
                    info.type = str;
                    info.value = 0;
                    info.is_const = is_const;
                    info.level = 0;
                    info.name = ident;
                    if (init_val) {
                        has_left = true;
                        init_val->Generate(true);
                        has_left = false;
                    }
                    GlobalSymbolTable->table[ident] = info;
                    kstr += "global @" + info.name + " = alloc ";
                    if (info.type == "int")
                        kstr += "i32";
                    kstr += ", zeroinit\n";
                }
            }
            else {
                if (const_array) {
                    SymbolInfo info;
                    info.name = ident + "_" + std::to_string(level);

                    kstr += "    @" + info.name + " = alloc ";
                    const_array->Generate(true);
                    kstr += const_array->label + "\n";
                    
                    info.type = const_array->label;
                    info.value = 0;
                    info.is_const = is_const;
                    info.level = level;

                    CurrentSymbolTable->table[ident] = info;

                    array_temp.clear();
                    array_shape.clear();
                    int size = 1;
                    loc = 0, depth = -1;

                    while (!array_depth.empty()) {
                        size *= array_depth.top();
                        array_shape.push_back(array_depth.top());
                        array_depth.pop();
                    }
                    array_temp = std::vector<int>(size, 0);
                    std::reverse(array_shape.begin(), array_shape.end());
                    
                    if (init_val) {
                        init_val->Generate(true);
                    }

                    loc = 0;
                    current_ptr.push("@" + info.name);
                    array_generate_loc(0);
                    current_ptr.pop();
                }
                else {
                    SymbolInfo info;
                    info.type = str;
                    info.value = 0;
                    info.is_const = is_const;
                    info.level = level;
                    info.name = ident + "_" + std::to_string(level);
                    if (init_val) {
                        has_left = true;
                        init_val->Generate(true);
                        has_left = false;
                    }
                    CurrentSymbolTable->table[ident] = info;
                    if (info.type == "int") {
                        kstr += "    @" + ident + "_" + std::to_string(level) + " = alloc i32\n";
                        if (init_val) kstr += "    store " + init_val->label + ", @" + ident + "_" + std::to_string(level) + "\n";
                        else kstr += "    store 0, @" + ident + "_" + std::to_string(level) + "\n";
                    }
                    while (!array_depth.empty()) {
                        array_depth.pop();      
                    }             
                }
            }
            if (next)
                next->AddtoSymbolTable(str, false);
        }
};

// InitVal ::= Exp | "{" [InitVals] "}";
class InitValAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> exp;
        std::unique_ptr<BaseAST> init_vals;

        void Generate(bool write = true) override {
            if (type == 0) {
                exp->Generate(write);
                if (write) label = exp->label;
                value = exp->value;
            }
            else {
                value = 810975;
                depth++;
                if (init_vals) {
                    init_vals->Generate(write);
                }
                depth--;
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// InitVals ::= InitVal | InitVal "," InitVals;
class InitValsAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> init_val;

        void Generate(bool write = true) override {
            init_val->Generate(write);
            if (init_val->value != 810975) array_temp[loc++] = init_val->value;
            if (next) {
                next->Generate(write);
            }
            else {
                int _align = 1;
                for (int i = depth; i < array_shape.size(); i++)
                    _align *= array_shape[i];
                while (loc % _align != 0)
                    loc++;
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// FuncFParams ::= FuncFParam | FuncFParam "," FuncFParams;
class FuncFParamsAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_f_param;

        void Generate(bool write = true) override {
            func_f_param->Generate(write);
            if (next) {
                kstr += ", ";
                next->Generate(write);
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// FuncFParam ::= BType IDENT ['[' ']' ConstArray]
class FuncFParamAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> b_type;
        std::string ident;
        std::unique_ptr<BaseAST> const_array;

        void Generate(bool write = true) override {
            if (type == 0) {
                if (b_type->label == "int")
                    kstr += "@" + ident + ": i32";
                label = b_type->label;
            }
            else {
                kstr += "@" + ident + ": ";
                label = "*";
                if (const_array) {
                    const_array->Generate(false);
                    label += const_array->label;
                }
                else label += "i32";
                kstr += label;
            }
            this->AddtoSymbolTable(label, false);
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {
            SymbolInfo info;
            info.type = str;
            info.value = 0;
            info.is_const = is_const;
            info.level = 1;
            info.name = ident + "_1";
            FuncSymbolTable->table[ident] = info;
        }
};

// FuncRParams ::= Exp | Exp "," FuncRParams;
class FuncRParamsAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;

        void Generate(bool write = true) override {
            exp->Generate(write);
            label = exp->label;
            if (next) {
                next->Generate(write);
                label += ", " + next->label;
            }
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// ConstArray ::= "[" ConstExp "]" {ConstArray};
class ConstArrayAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> const_exp;

        void Generate(bool write = true) override {
            const_exp->Generate(false);
            array_depth.push(const_exp->value);
            label = "[";
            value = const_exp->value;
            if (next) {
                next->Generate(write);
                label += next->label;
            }
            else label += "i32";
            label += ", " + std::to_string(const_exp->value) + "]";
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// Array ::= "[" Exp "]" {Array};
class ArrayAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;

        void Generate(bool write = true) override {
            exp->Generate(write);
            if (write) {
                label = "%" + std::to_string(cnt++);
                if (_ptr) kstr += "    " + label + " = getptr " + current_ptr.top() + ", " + exp->label + "\n";
                else kstr += "    " + label + " = getelemptr " + current_ptr.top() + ", " + exp->label + "\n";
            }
            if (next) {
                current_ptr.push(label);
                next->Generate(write);
                current_ptr.pop();
            }
            label = "%" + std::to_string(cnt - 1);
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};