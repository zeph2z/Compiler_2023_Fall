#pragma once

#include <iostream>
#include <memory>
#include <string>

class BaseAST {
    public:
        virtual ~BaseAST() = default;

        virtual void Dump() const = 0;
        virtual std::string Generate() { return ""; }
};

class CompUnitAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_def;

        void Dump() const override {
            std::cout << "CompUnitAST { ";
            func_def->Dump();
            std::cout << " }";
        }
        std::string Generate() override {
            return func_def->Generate();
        }
};

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
        std::string Generate() override {
            return "fun @" + ident + "(): i32 " + block->Generate();
        }
};

class FuncTypeAST : public BaseAST {
    public:
        std::string type;

        void Dump() const override {
            std::cout << "FuncTypeAST { " << type << " }";
        }
        std::string Generate() override {
            return type;
        }
};

class BlockAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> stmt;

        void Dump() const override {
            std::cout << "BlockAST { ";
            stmt->Dump();
            std::cout << " }";
        }
        std::string Generate() override {
            return "{\n%entry:\n" + stmt->Generate() + "\n}\n";
        }
};

class StmtAST : public BaseAST {
    public:
        int number;

        void Dump() const override {
            std::cout << "StmtAST { " << number << " }";
        }
        std::string Generate() override {
            return "    ret " + std::to_string(number);
        }
};