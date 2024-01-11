#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <stack>
#include <unordered_map>
#include <vector>
#include "ast.hpp"
#include "koopa.h"
#include "raw.hpp"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
extern void raw2riscv(koopa_raw_program_t &raw, string &str);

bool must_return = false, branch = false, has_left = false, global = false, need_load = true, as_para = false;
int cnt = 0, level = 0, block_cnt = 0, logic_cnt = 0, array_cnt, loc, depth, func_r_cnt;
std::string kstr, last_br, true_block_name, false_block_name, current_func;
std::shared_ptr<SymbolTableNode> CurrentSymbolTable, FuncSymbolTable, GlobalSymbolTable;
std::unordered_map<std::string, SymbolInfo> FuncTable;
std::stack<int> while_stack, current_depth, array_depth;
std::vector<int> array_temp, array_shape;
std::vector<std::string> params;
std::stack<std::string> current_ptr;

std::ostream& operator<<(std::ostream& os, const SymbolInfo& info) {
    os << "name: " << info.name << ", type: " << info.type << ", value: " << info.value << ", is_const: " << info.is_const << ", level: " << info.level;
    return os;
}

bool last_is_br() {
    if (last_br == "br" || last_br == "jump" || last_br == "ret") {
        return true;
    }
    return false;
}

void decl_lib_func() {
  kstr += "decl @getint(): i32\n";
  kstr += "decl @getch(): i32\n";
  kstr += "decl @getarray(*i32): i32\n"; FuncTable["getarray"].param.push_back("*i32");
  kstr += "decl @putint(i32)\n"; FuncTable["putint"].param.push_back("i32");
  kstr += "decl @putch(i32)\n"; FuncTable["putch"].param.push_back("i32");
  kstr += "decl @putarray(i32, *i32)\n"; FuncTable["putarray"].param.push_back("i32"); FuncTable["putarray"].param.push_back("*i32");
  kstr += "decl @starttime()\n";
  kstr += "decl @stoptime()\n";
  kstr += "\n";
}

void array_generate_loc(int _depth) {
  if (_depth == array_shape.size()) {
    kstr += "    store " + to_string(array_temp[loc++]) + ", " + current_ptr.top() + "\n";
    return;
  }
  for (int i = 0; i < array_shape[_depth]; i++) {
    string temp = "@ptr_" + to_string(array_cnt++);
    kstr += "    " + temp + " = getelemptr " + current_ptr.top() + ", " + std::to_string(i) + "\n";
    current_ptr.push(temp);
    array_generate_loc(_depth + 1);
    current_ptr.pop();
  }
}

void array_generate_glb(int _depth) {
  if (_depth == array_shape.size() - 1) {
    kstr += to_string(array_temp[loc++]);
    for (int i = 1; i < array_shape[_depth]; i++)
      kstr += ", " + to_string(array_temp[loc++]);
    return;
  }
  else {
    kstr += "{";
    array_generate_glb(_depth + 1);
    kstr += "}";
    for (int i = 1; i < array_shape[_depth]; i++) {
      kstr += ", {";
      array_generate_glb(_depth + 1);
      kstr += "}";
    }
  }
}

int main(int argc, const char *argv[]) {
  // compiler mode input -o output 
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  yyin = fopen(input, "r");
  assert(yyin);

  extern int yydebug;
  yydebug = 0;

  unique_ptr<BaseAST> ast;
  auto retp = yyparse(ast);
  assert(!retp);

  GlobalSymbolTable = make_shared<SymbolTableNode>();
  decl_lib_func();

  ast->Generate();
  for (const auto &kv : GlobalSymbolTable->table) {
    auto &info = kv.second;
    cout << info << endl;
  }
  for (const auto& pair : FuncTable) {
    std::cout << pair.first << " ";
    for (const auto& param : pair.second.param) {
      std::cout << param << " ";
    }
    std::cout << std::endl;
  }

  FILE *yyout = fopen(output, "w");
  assert(yyout);
  fprintf(yyout, "%s", kstr.c_str());
  fclose(yyout);

  if (strcmp(mode, "-riscv") == 0) {
    koopa_program_t program;
    koopa_error_code_t retk = koopa_parse_from_string(kstr.c_str(), &program);
    assert(retk == KOOPA_EC_SUCCESS);
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);

    string ostr;
    raw2riscv(raw, ostr);

    FILE *rout = fopen(output, "w");
    assert(rout);
    fprintf(rout, "%s", ostr.c_str());
    fclose(rout);

    koopa_delete_raw_program_builder(builder);
  }
  
  return 0;
}