%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"

int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

%parse-param { std::unique_ptr<BaseAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  char char_val;
  BaseAST* ast_val;
}

%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST
%token <char_val> PLUS MINUS NOT TIMES DIVIDE MOD
%token <str_val> LT GT LE GE EQ NE AND OR

%type <ast_val> FuncDef FuncType Block Stmt Exp UnaryExp PrimaryExp MulExp AddExp LOrExp LAndExp EqExp RelExp
%type <int_val> Number
%type <char_val> UnaryOp AddOp MulOp
%type <str_val> RelOp EqOp

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type = "int";
    $$ = ast;
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->kind = "return";
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

PrimaryExp 
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->type = 0;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  } 
  | Number {
    auto ast = new PrimaryExpAST();
    ast->type = 1;
    ast->number = $1;
    $$ = ast;
  }

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->type = 0;
    ast->primary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } 
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST();
    ast->type = 1;
    ast->op = $1;
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

MulExp
  : UnaryExp {
    auto ast = new MulExpAST();
    ast->type = 0;
    ast->unary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp MulOp UnaryExp {
    auto ast = new MulExpAST();
    ast->op = $2;
    ast->type = 1;
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

AddExp
  : MulExp {
    auto ast = new AddExpAST();
    ast->type = 0;
    ast->mul_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp AddOp MulExp {
    auto ast = new AddExpAST();
    ast->op = $2;
    ast->type = 1;
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

RelExp
  : AddExp {
    auto ast = new RelExpAST();
    ast->type = 0;
    ast->add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | RelExp RelOp AddExp {
    auto ast = new RelExpAST();
    ast->op = *$2;
    ast->type = 1;
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

EqExp
  : RelExp {
    auto ast = new EqExpAST();
    ast->type = 0;
    ast->rel_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | EqExp EqOp RelExp {
    auto ast = new EqExpAST();
    ast->op = *$2;
    ast->type = 1;
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->rel_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

LAndExp
  : EqExp {
    auto ast = new LAndExpAST();
    ast->type = 0;
    ast->eq_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp AND EqExp {
    auto ast = new LAndExpAST();
    ast->type = 1;
    ast->land_exp = unique_ptr<BaseAST>($1);
    ast->eq_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

LOrExp
  : LAndExp {
    auto ast = new LOrExpAST();
    ast->type = 0;
    ast->land_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp OR LAndExp {
    auto ast = new LOrExpAST();
    ast->type = 1;
    ast->lor_exp = unique_ptr<BaseAST>($1);
    ast->land_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

Number
  : INT_CONST {
    $$ = $1;
  }

UnaryOp
  : PLUS | MINUS | NOT {
    $$ = $1;
  }

AddOp
  : PLUS | MINUS {
    $$ = $1;
  }

MulOp
  : TIMES | DIVIDE | MOD {
    $$ = $1;
  }

RelOp
  : LT | GT | LE | GE {
    $$ = $1;
  }

EqOp
  : EQ | NE {
    $$ = $1;
  }

%%

void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
