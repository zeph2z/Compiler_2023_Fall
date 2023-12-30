#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include "ast.hpp"
#include "koopa.h"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
  // compiler mode input -o output 
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];
  
  if (strcmp(mode, "-koopa") == 0) {
    yyin = fopen(input, "r");
    assert(yyin);

    unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);

    ast->Dump();
    string str = ast->Generate();
    cout << endl;

    FILE *yyout = fopen(output, "w");
    assert(yyout);
    fprintf(yyout, "%s", str.c_str());
    fclose(yyout);
  }
  
  // koopa_program_t program;
  // koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  // assert(ret == KOOPA_EC_SUCCESS);
  // koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // koopa_delete_program(program);

  // koopa_delete_raw_program_builder(builder);

  return 0;
}