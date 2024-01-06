#pragma once

#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include "symbol_table.hpp"

extern bool must_return, branch, func_is_void, has_left;
extern int cnt, level, block_cnt;
extern std::string kstr, last_br, true_block_name, false_block_name;
extern std::stack<int> while_stack;

extern std::ostream& operator<<(std::ostream& os, const SymbolInfo& info);
extern bool last_is_br();

class BaseAST {
    public:
        std::string label;
        int value;
        std::unique_ptr<BaseAST> next = nullptr;

        virtual void Generate(bool write = true) = 0;
        virtual void AddtoSymbolTable(std::string str, bool is_const) = 0;

        virtual ~BaseAST() = default;
};

// CompUnit ::= [CompUnit] FuncDef;
class CompUnitAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_def;

        void Generate(bool write = true) override {
            func_def->Generate(write);
            if (next) next->Generate(write);
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// FuncDef ::= FuncType IDENT "(" [FuncFParams] ")" Block ;
class FuncDefAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_type;
        std::string ident;
        std::unique_ptr<BaseAST> block;
        std::unique_ptr<BaseAST> func_f_params;

        void Generate(bool write = true) override {
            FuncSymbolTable.reset(new SymbolTableNode());
            CurrentSymbolTable.reset();
            level = 0;
            last_br.clear();
            kstr += "fun @" + ident + "(";
            if (func_f_params) func_f_params->Generate(write);
            kstr += "): ";

            func_type->Generate(write);
            if (func_type->label == "void") func_is_void = true;
            else func_is_void = false;

            FuncTable[ident].type = func_type->label;

            kstr += "{\n%entry:\n";
            block->Generate(write);
            if (func_is_void && !last_is_br()) kstr += "    ret\n"; 
            kstr += "}\n\n";
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// FuncType ::= BType;
class FuncTypeAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> b_type;

        void Generate(bool write = true) override {
            label = b_type->label;
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
                for (const auto& pair : FuncSymbolTable->table) {
                    CurrentSymbolTable->table.insert(pair);
                    kstr += "    @" + pair.first + "_1" + " = alloc i32\n";
                    kstr += "    store @" + pair.first + ", @" + pair.first + "_1" + "\n";
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
                        kstr += "    store " + exp->label + ", @" + l_val->label + "_" + std::to_string(it2->second.level) + "\n";
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
                if (func_r_params) func_r_params->Generate(write);
                if (write) {
                    if (has_left) {
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
                auto it = CurrentSymbolTable;
                while (it) {
                    auto it2 = it->table.find(l_val->label);
                    if (it2 != it->table.end()) {
                        if (it2->second.is_const) {
                            label = std::to_string(it2->second.value);
                            value = it2->second.value;
                        }
                        else {
                            if (write) {
                                kstr += "    %" + std::to_string(cnt++) + " = load @" + it2->first + "_" + std::to_string(it2->second.level) + "\n"; 
                                label = "%" + std::to_string(cnt - 1);
                            }
                            value = it2->second.value;
                        }
                        break;
                    }
                    it = it->parent;
                }
                if (!it) label = "err";
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
                    land_exp->Generate(write);
                    std::string label1 = "%" + std::to_string(cnt++);
                    // %2 = ne 0, %0
                    kstr += "    " + label1 + " = ne 0, " + land_exp->label + "\n";
                    if (branch) {
                        kstr += "    br " + label1 + ", %logic_true_" + std::to_string(block_cnt) + ", " + false_block_name + "\n\n";
                        kstr += "%logic_true_" + std::to_string(block_cnt++) + ":\n";
                    }

                    eq_exp->Generate(write);
                    std::string label2 = "%" + std::to_string(cnt++);
                    // %3 = ne 0, %1
                    kstr += "    " + label2 + " = ne 0, " + eq_exp->label + "\n";
                    // %4 = and %2, %3
                    if (branch) label = label2;
                    else {
                        label = "%" + std::to_string(cnt++);
                        kstr += "    " + label + " = and " + label1 + ", " + label2 + "\n";
                    }
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
                    lor_exp->Generate(write);
                    std::string label1 = "%" + std::to_string(cnt++);
                    // %2 = ne 0, %0
                    kstr += "    " + label1 + " = ne 0, " + lor_exp->label + "\n";
                    if (branch) {
                        kstr += "    br " + label1 + ", " + true_block_name + ", %logic_false_" + std::to_string(block_cnt) + "\n\n";
                        kstr += "%logic_false_" + std::to_string(block_cnt++) + ":\n";
                    }
                    
                    land_exp->Generate(write);
                    std::string label2 = "%" + std::to_string(cnt++);
                    // %3 = ne 0, %1
                    kstr += "    " + label2 + " = ne 0, " + land_exp->label + "\n";
                    // %4 = or %2, %3
                    if (branch) label = label2;
                    else {
                        label = "%" + std::to_string(cnt++);
                        kstr += "    " + label + " = or " + label1 + ", " + label2 + "\n";
                    }
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

// ConstDef ::= IDENT "=" ConstInitVal | IDENT "=" ConstInitVal "," ConstDef;
class ConstDefAST : public BaseAST {
    public:
        std::string ident;
        std::unique_ptr<BaseAST> const_init_val;

        void Generate(bool write = true) override {}
        void AddtoSymbolTable(std::string str, bool is_const) override {
            SymbolInfo info;
            info.type = str;
            const_init_val->Generate(false);
            info.value = const_init_val->value;
            info.is_const = is_const;
            info.level = level;
            CurrentSymbolTable->table[ident] = info;
            if (next)
                next->AddtoSymbolTable(str, true);
        }
};

// ConstInitVal ::= ConstExp;
class ConstInitValAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> const_exp;

        void Generate(bool write = true) override {
            const_exp->Generate(write);
            if (write) label = const_exp->label;
            value = const_exp->value;
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

// LVal ::= IDENT;
class LValAST : public BaseAST {
    public:
        void Generate(bool write = true) override {}
        void AddtoSymbolTable(std::string str, bool is_const) override {}
};

// BType ::= INT | VOID;
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

// VarDef ::= IDENT | IDENT "=" InitVal | IDENT ',' VarDef | IDENT "=" InitVal "," VarDef;
class VarDefAST : public BaseAST {
    public:
        std::string ident;
        std::unique_ptr<BaseAST> init_val = nullptr;

        void Generate(bool write = true) override {}
        void AddtoSymbolTable(std::string str, bool is_const) override {
            SymbolInfo info;
            info.type = str;
            info.value = 0;
            info.is_const = is_const;
            info.level = level;
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
            if (next)
                next->AddtoSymbolTable(str, false);
        }
};

// InitVal ::= Exp;
class InitValAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;

        void Generate(bool write = true) override {
            exp->Generate(write);
            if (write) label = exp->label;
            value = exp->value;
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

// FuncFParam ::= BType IDENT;
class FuncFParamAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> b_type;
        std::string ident;

        void Generate(bool write = true) override {
            this->AddtoSymbolTable(b_type->label, false);
            if (b_type->label == "int")
                kstr += "@" + ident + ": i32";
        }
        void AddtoSymbolTable(std::string str, bool is_const) override {
            SymbolInfo info;
            info.type = str;
            info.value = 0;
            info.is_const = is_const;
            info.level = 1;
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