#pragma once

#include <iostream>
#include <memory>
#include <string>

extern int cnt;
extern std::string kstr;

class BaseAST {
    public:
        std::string label;
        bool prio = false;
        virtual ~BaseAST() = default;

        virtual void Dump() const = 0;
        virtual void Generate() {} ;
};

// CompUnit ::= FuncDef;
class CompUnitAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_def;

        void Dump() const override {
            std::cout << "CompUnitAST { ";
            func_def->Dump();
            std::cout << " }";
        }
        void Generate() override {
            func_def->Generate();
        }
};

// FuncDef ::= FuncType IDENT "(" ")" Block;
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
        void Generate() override {
            kstr += "fun @" + ident + "(): ";
            func_type->Generate();
            kstr += " ";
            block->Generate();
        }
};

// FuncType ::= "int";
class FuncTypeAST : public BaseAST {
    public:
        std::string type;

        void Dump() const override {
            std::cout << "FuncTypeAST { " << type << " }";
        }
        void Generate() override {
            if (type == "int")
                kstr += "i32";
        }
};

// Block ::= "{" Stmt* "}";
class BlockAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> stmt;

        void Dump() const override {
            std::cout << "BlockAST { ";
            stmt->Dump();
            std::cout << " }";
        }
        void Generate() override {
            kstr += "{\n%entry:\n";
            stmt->Generate();
            kstr += "\n}\n";
        }
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
        void Generate() override {
            exp->Generate();
            kstr += "    ";
            if (kind == "return")
                kstr += "ret";
            kstr += " " + exp->label;
        }
};

// Exp ::= AddExp;
class ExpAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> add_exp;

        void Dump() const override {
            std::cout << "ExpAST { ";
            add_exp->Dump();
            std::cout << " }";
        }
        void Generate() override {
            add_exp->Generate();
            label = add_exp->label;
        }
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
        void Generate() override {
            if (type == 0) {
                primary_exp->Generate();
                label = primary_exp->label;
            }
            else if (type == 1) {
                unary_exp->Generate();
                switch (op) {
                    case '+' : {
                        label = unary_exp->label; 
                        break;
                    }
                    case '-' : {
                        label = "%" + std::to_string(cnt++);
                        // %2 = sub 0, %1
                        kstr += "    " + label + " = sub 0, " + unary_exp->label + "\n";
                        break;
                    }
                    case '!' : {
                        label = "%" + std::to_string(cnt++);
                        // %0 = eq 6, 0
                        kstr += "    " + label + " = eq " + unary_exp->label + ", 0\n";
                        break;
                    }
                }
            }
        }
};

// PrimaryExp ::= "(" Exp ")" | Number;
class PrimaryExpAST : public BaseAST {
    public:
        int type;
        int number;
        std::unique_ptr<BaseAST> exp;

        void Dump() const override {
            std::cout << "PrimaryExpAST { ";
            if (type == 0) {
                std::cout << "( ";
                exp->Dump();
                std::cout << " )";
            }
            else if (type == 1)
                std::cout << number;
            std::cout << " }";
        }
        void Generate() override {
            if (type == 0) {
                exp->Generate();
                label = exp->label;
            }
            else if (type == 1)
                label = std::to_string(number);
        }
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
        void Generate() override {
            if (type == 0) {
                mul_exp->Generate();
                label = mul_exp->label;
            }
            else if (type == 1) {
                add_exp->Generate();
                mul_exp->Generate();
                label = "%" + std::to_string(cnt++);
                switch (op) {
                    case '+' : {
                        // %1 = add %0, %2
                        kstr += "    " + label + " = add " + add_exp->label + ", " + mul_exp->label + "\n";
                        break;
                    }
                    case '-' : {
                        // %1 = sub %0, %2
                        kstr += "    " + label + " = sub " + add_exp->label + ", " + mul_exp->label + "\n";
                        break;
                    }
                }
            }
        }
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
        void Generate() override {
            if (type == 0) {
                unary_exp->Generate();
                label = unary_exp->label;
            }
            else if (type == 1) {
                mul_exp->Generate();
                unary_exp->Generate();
                label = "%" + std::to_string(cnt++);
                switch (op) {
                    case '*' : {
                        // %1 = mul %0, %2
                        kstr += "    " + label + " = mul " + mul_exp->label + ", " + unary_exp->label + "\n";
                        break;
                    }
                    case '/' : {
                        // %1 = div %0, %2
                        kstr += "    " + label + " = div " + mul_exp->label + ", " + unary_exp->label + "\n";
                        break;
                    }
                    case '%' : {
                        // %1 = rem %0, %2
                        kstr += "    " + label + " = rem " + mul_exp->label + ", " + unary_exp->label + "\n";
                        break;
                    }
                }
            }
        }
};