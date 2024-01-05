#pragma once

#include <iostream>
#include <memory>
#include <string>
#include "symbol_table.hpp"

extern int cnt;
extern std::string kstr;
extern std::unordered_map<std::string, SymbolInfo> SymbolTable;

extern std::ostream& operator<<(std::ostream& os, const SymbolInfo& info);

class BaseAST {
    public:
        std::string label;
        int value;
        std::unique_ptr<BaseAST> next = nullptr;

        virtual void Dump() const = 0;
        virtual void Generate(bool write = true) = 0;
        virtual void CreateSymbolTable(std::string str) = 0;
        // virtual std::string GetIdent() { return ""; };

        virtual ~BaseAST() = default;
};

// CompUnit ::= FuncDef;
class CompUnitAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_def;

        void Dump() const override {
            std::cout << "CompUnitAST { ";
            func_def->Dump();
            std::cout << " }\n";
        }
        void Generate(bool write = true) override {
            func_def->Generate(write);
        }
        void CreateSymbolTable(std::string str) override {}
};

// FuncDef ::= FuncType IDENT "(" ")" Block ;
class FuncDefAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_type;
        std::string ident;
        std::unique_ptr<BaseAST> block;

        void Dump() const override {
            std::cout << "FuncDefAST { ";
            func_type->Dump();
            std::cout << ", " << ident << ", ";
            block->Dump();
            std::cout << " }";
        }
        void Generate(bool write = true) override {
            kstr += "fun @" + ident + "(): ";
            func_type->Generate(write);
            kstr += " ";
            block->Generate(write);
        }
        void CreateSymbolTable(std::string str) override {}
};

// FuncType ::= BType;
class FuncTypeAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> b_type;

        void Dump() const override {
            std::cout << "FuncTypeAST { " << b_type->label << " }";
        }
        void Generate(bool write = true) override {
            kstr += "i32";
        }
        void CreateSymbolTable(std::string str) override {}
};

// Block ::= "{" {BlockItem} "}";
class BlockAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> block_item;

        void Dump() const override {
            std::cout << "BlockAST { ";
            block_item->Dump();
            std::cout << " }";
        }
        void Generate(bool write = true) override {
            kstr += "{\n%entry:\n";
            block_item->Generate(write);
            kstr += "}\n";
        }
        void CreateSymbolTable(std::string str) override {}
};

// BlockItem ::= Decl | Stmt;
class BlockItemAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> decl = nullptr;
        std::unique_ptr<BaseAST> stmt = nullptr;

        void Dump() const override {
            std::cout << "BlockItemAST { ";
            if (decl)
                decl->Dump();
            else if (stmt)
                stmt->Dump();
            std::cout << " }";
            if (next) {
                std::cout << ", ";
                next->Dump();
            }
        }
        void Generate(bool write = true) override {
            if (decl)
                decl->Generate(write);
            else if (stmt)
                stmt->Generate(write);
            if (next) {
                next->Generate(write);
            }
        }
        void CreateSymbolTable(std::string str) override {}
};

// Stmt ::= "return" Exp ";";
class StmtAST : public BaseAST {
    public:
        std::string kind;
        std::unique_ptr<BaseAST> exp;

        void Dump() const override {
            std::cout << "StmtAST { ";
            exp->Dump();
            std::cout << " }";
        }
        void Generate(bool write = true) override {
            exp->Generate(write);
            kstr += "    ";
            if (kind == "return")
                kstr += "ret";
            kstr += " " + exp->label + "\n";
        }
        void CreateSymbolTable(std::string str) override {}
};

// Exp ::= LOrExp;
class ExpAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> lor_exp;

        void Dump() const override {
            std::cout << "ExpAST { ";
            lor_exp->Dump();
            std::cout << " }";
        }
        void Generate(bool write = true) override {
            lor_exp->Generate(write);
            if (write) label = lor_exp->label;
            value = lor_exp->value;
        }
        void CreateSymbolTable(std::string str) override {}
};

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp;
class UnaryExpAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> primary_exp;
        char op;
        std::unique_ptr<BaseAST> unary_exp;

        void Dump() const override {
            std::cout << "UnaryExpAST { ";
            if (type == 0)
                primary_exp->Dump();
            else if (type == 1) {
                std::cout << op << " ";
                unary_exp->Dump();
            }
            std::cout << " }";
        }
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
        }
        void CreateSymbolTable(std::string str) override {}
};

// PrimaryExp ::= "(" Exp ")" | LVal | Number;
class PrimaryExpAST : public BaseAST {
    public:
        int type;
        int number;
        std::unique_ptr<BaseAST> exp, l_val;

        void Dump() const override {
            std::cout << "PrimaryExpAST { ";
            if (type == 0) {
                std::cout << "( ";
                exp->Dump();
                std::cout << " )";
            }
            else if (type == 1)
                l_val->Dump();
            else if (type == 2)
                std::cout << number;
            std::cout << " }";
        }
        void Generate(bool write = true) override {
            if (type == 0) {
                exp->Generate(write);
                if (write) label = exp->label;
                value = exp->value;
            }
            else if (type == 1) {
                auto it = SymbolTable.find(l_val->label);
                value = 0;
                if (it != SymbolTable.end()) {
                    label = std::to_string(it->second.value);
                    value = it->second.value;
                }
                else label = "err";
            }
            else if (type == 2) {
                if (write) label = std::to_string(number);
                value = number;
            }
        }
        void CreateSymbolTable(std::string str) override {}
};

// AddExp ::= MulExp | AddExp ("+" | "-") MulExp;
class AddExpAST : public BaseAST {
    public:
        int type;
        char op;
        std::unique_ptr<BaseAST> mul_exp;
        std::unique_ptr<BaseAST> add_exp;

        void Dump() const override {
            if (type == 0) {
                std::cout << "AddExpAST { ";
                mul_exp->Dump();
                std::cout << " }";
            }
            else if (type == 1) {
                std::cout << "AddExpAST { ";
                add_exp->Dump();
                std::cout << " " << op << " ";
                mul_exp->Dump();
                std::cout << " }";
            }
        }
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
        void CreateSymbolTable(std::string str) override {}
};

// MulExp ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
class MulExpAST : public BaseAST {
    public:
        int type;
        char op;
        std::unique_ptr<BaseAST> unary_exp;
        std::unique_ptr<BaseAST> mul_exp;

        void Dump() const override {
            if (type == 0) {
                std::cout << "MulExpAST { ";
                unary_exp->Dump();
                std::cout << " }";
            }
            else if (type == 1) {
                std::cout << "MulExpAST { ";
                mul_exp->Dump();
                std::cout << " " << op << " ";
                unary_exp->Dump();
                std::cout << " }";
            }
        }
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
                        value = mul_exp->value / unary_exp->value;
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
        void CreateSymbolTable(std::string str) override {}
};

// RelExp ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
class RelExpAST : public BaseAST {
    public:
        int type;
        std::string op;
        std::unique_ptr<BaseAST> add_exp;
        std::unique_ptr<BaseAST> rel_exp;

        void Dump() const override {
            if (type == 0) {
                std::cout << "RelExpAST { ";
                add_exp->Dump();
                std::cout << " }";
            }
            else if (type == 1) {
                std::cout << "RelExpAST { ";
                rel_exp->Dump();
                std::cout << " " << op << " ";
                add_exp->Dump();
                std::cout << " }";
            }
        }
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
        void CreateSymbolTable(std::string str) override {}
};

// EqExp ::= RelExp | EqExp ("==" | "!=") RelExp;
class EqExpAST : public BaseAST {
    public:
        int type;
        std::string op;
        std::unique_ptr<BaseAST> rel_exp;
        std::unique_ptr<BaseAST> eq_exp;

        void Dump() const override {
            if (type == 0) {
                std::cout << "EqExpAST { ";
                rel_exp->Dump();
                std::cout << " }";
            }
            else if (type == 1) {
                std::cout << "EqExpAST { ";
                eq_exp->Dump();
                std::cout << " " << op << " ";
                rel_exp->Dump();
                std::cout << " }";
            }
        }
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
        void CreateSymbolTable(std::string str) override {}
};

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> eq_exp;
        std::unique_ptr<BaseAST> land_exp;

        void Dump() const override {
            if (type == 0) {
                std::cout << "LAndExpAST { ";
                eq_exp->Dump();
                std::cout << " }";
            }
            else if (type == 1) {
                std::cout << "LAndExpAST { ";
                land_exp->Dump();
                std::cout << " && ";
                eq_exp->Dump();
                std::cout << " }";
            }
        }
        void Generate(bool write = true) override {
            if (type == 0) {
                eq_exp->Generate(write);
                if (write) label = eq_exp->label;
                value = eq_exp->value;
            }
            else if (type == 1) {
                land_exp->Generate(write);
                eq_exp->Generate(write);
                if (write) {
                    std::string label1 = "%" + std::to_string(cnt++);
                    std::string label2 = "%" + std::to_string(cnt++);
                    // %2 = ne 0, %0
                    kstr += "    " + label1 + " = ne 0, " + land_exp->label + "\n";
                    // %3 = ne 0, %1
                    kstr += "    " + label2 + " = ne 0, " + eq_exp->label + "\n";
                    // %4 = and %2, %3
                    label = "%" + std::to_string(cnt++);
                    kstr += "    " + label + " = and " + label1 + ", " + label2 + "\n";
                }
                value = land_exp->value && eq_exp->value;
            }
        }
        void CreateSymbolTable(std::string str) override {}
};

// LOrExp ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST {
    public:
        int type;
        std::unique_ptr<BaseAST> land_exp;
        std::unique_ptr<BaseAST> lor_exp;

        void Dump() const override {
            if (type == 0) {
                std::cout << "LOrExpAST { ";
                land_exp->Dump();
                std::cout << " }";
            }
            else if (type == 1) {
                std::cout << "LOrExpAST { ";
                lor_exp->Dump();
                std::cout << " || ";
                land_exp->Dump();
                std::cout << " }";
            }
        }
        void Generate(bool write) override {
            if (type == 0) {
                land_exp->Generate(write);
                if (write) label = land_exp->label;
                value = land_exp->value;
            }
            else if (type == 1) {
                lor_exp->Generate(write);
                land_exp->Generate(write);
                if (write) {
                    std::string label1 = "%" + std::to_string(cnt++);
                    std::string label2 = "%" + std::to_string(cnt++);
                    // %2 = ne 0, %0
                    kstr += "    " + label1 + " = ne 0, " + lor_exp->label + "\n";
                    // %3 = ne 0, %1
                    kstr += "    " + label2 + " = ne 0, " + land_exp->label + "\n";
                    // %4 = or %2, %3
                    label = "%" + std::to_string(cnt++);
                    kstr += "    " + label + " = or " + label1 + ", " + label2 + "\n";
                }
                value = lor_exp->value || land_exp->value;
            }
        }
        void CreateSymbolTable(std::string str) override {}
};

// Decl ::= ConstDecl;
class DeclAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> const_decl;

        void Dump() const override {
            std::cout << "DeclAST { ";
            const_decl->Dump();
            std::cout << " }";
        }
        void Generate(bool write = true) override {
            const_decl->Generate(write);
        }
        void CreateSymbolTable(std::string str) override {}
};

// ConstDecl ::= "const" BType ConstDef ";";
class ConstDeclAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> b_type;
        std::unique_ptr<BaseAST> const_def;

        void Dump() const override {
            std::cout << "ConstDeclAST { " << b_type->label << ", ";
            const_def->Dump();
            std::cout << " }";
        }
        void Generate(bool write = true) override {
            const_def->CreateSymbolTable(b_type->label);
            for (const auto& pair : SymbolTable) {
                std::cout << "Key: " << pair.first << " Value: " << pair.second << std::endl;
            }
        }
        void CreateSymbolTable(std::string str) override {}
};

// ConstDef ::= IDENT "=" ConstInitVal | IDENT "=" ConstInitVal "," ConstDef;
class ConstDefAST : public BaseAST {
    public:
        std::string ident;
        std::unique_ptr<BaseAST> const_init_val;

        void Dump() const override {
            std::cout << "ConstDefAST { ";
            std::cout << ident << ", ";
            const_init_val->Dump();
            std::cout << " }";
            if (next) {
                std::cout << ", ";
                next->Dump();
            }
        }
        void Generate(bool write = true) override {}
        void CreateSymbolTable(std::string str) override {
            SymbolInfo info;
            info.type = str;
            const_init_val->Generate(false);
            info.value = const_init_val->value;
            SymbolTable[ident] = info;
            if (next)
                next->CreateSymbolTable(str);
        }
};

// ConstInitVal ::= ConstExp;
class ConstInitValAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> const_exp;

        void Dump() const override {
            std::cout << "ConstInitValAST { ";
            const_exp->Dump();
            std::cout << " }";
        }
        void Generate(bool write = true) override {
            const_exp->Generate(write);
            if (write) label = const_exp->label;
            value = const_exp->value;
        }
        void CreateSymbolTable(std::string str) override {}
};

// ConstExp ::= Exp;
class ConstExpAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;

        void Dump() const override {
            std::cout << "ConstExpAST { ";
            exp->Dump();
            std::cout << " }";
        }
        void Generate(bool write = true) override {
            exp->Generate(write);
            if (write) label = exp->label;
            value = exp->value;
        }
        void CreateSymbolTable(std::string str) override {}
};

// LVal ::= IDENT;
class LValAST : public BaseAST {
    public:
        void Dump() const override {
            std::cout << "LValAST { " << label << " }";
        }
        void Generate(bool write = true) override {}
        void CreateSymbolTable(std::string str) override {}
};

class BTypeAST : public BaseAST {
    public:
        void Dump() const override {
            std::cout << "BTypeAST { " << label << " }";
        }
        void Generate(bool write = true) override {}
        void CreateSymbolTable(std::string str) override {}
};