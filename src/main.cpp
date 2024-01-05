#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <unordered_map>
#include "ast.hpp"
#include "koopa.h"
#include "raw.hpp"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
extern void raw2riscv(koopa_raw_program_t &raw, string &str);
int cnt = 0;
std::string kstr;
std::unordered_map<std::string, SymbolInfo> SymbolTable;

std::ostream& operator<<(std::ostream& os, const SymbolInfo& info) {
    os << "type: " << info.type << ", value: " << info.value;
    return os;
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

  ast->Dump();
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