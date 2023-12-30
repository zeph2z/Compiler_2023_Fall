#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
  // compiler mode input -o hello.koopa
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  yyin = fopen(input, "r");
  assert(yyin);

  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  ast->Dump();
  cout << endl;

  FILE *yyout = fopen(output, "w");
  assert(yyout);
  fprintf(yyout, "%s", ast->Generate().c_str());
  fclose(yyout);

  return 0;
}
