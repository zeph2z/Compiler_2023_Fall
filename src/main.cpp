#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <stack>
#include <unordered_map>
#include "ast.hpp"
#include "koopa.h"
#include "raw.hpp"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
extern void raw2riscv(koopa_raw_program_t &raw, string &str);

bool must_return = false, branch = false, func_is_void = false, has_left = false;
int cnt = 0, level = 0, block_cnt = 0;
std::string kstr, last_br, true_block_name, false_block_name;
std::shared_ptr<SymbolTableNode> CurrentSymbolTable, FuncSymbolTable;
std::unordered_map<std::string, SymbolInfo> FuncTable;
std::stack<int> while_stack;

std::ostream& operator<<(std::ostream& os, const SymbolInfo& info) {
    os << "type: " << info.type << ", value: " << info.value << ", is_const: " << info.is_const << ", level: " << info.level;
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
  kstr += "decl @getarray(*i32): i32\n";
  kstr += "decl @putint(i32)\n";
  kstr += "decl @putch(i32)\n";
  kstr += "decl @putarray(i32, *i32)\n";
  kstr += "decl @starttime()\n";
  kstr += "decl @stoptime()\n";
  kstr += "\n";
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

  decl_lib_func();

  ast->Generate();
  cout << endl;

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