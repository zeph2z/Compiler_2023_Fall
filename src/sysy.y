%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <stdio.h>
#include "ast.hpp"

#define YYDEBUG 1

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

%token INT RETURN CONST IF ELSE WHILE FOR BREAK CONTINUE VOID
%token <int_val> INT_CONST
%token <char_val> PLUS MINUS NOT TIMES DIVIDE MOD
%token <str_val> IDENT LT GT LE GE EQ NE AND OR

// lv1
%type <ast_val> FuncType Block Stmt
// lv3
%type <ast_val> Exp UnaryExp PrimaryExp MulExp AddExp LOrExp LAndExp EqExp RelExp
// lv4
%type <ast_val> Decl ConstDecl ConstDef ConstInitVal BlockItem LVal BType ConstExp VarDecl VarDef InitVal 
// lv8
%type <ast_val> FuncFParams FuncFParam FuncRParams RestCompUnit
// lv9
%type <ast_val> ConstArray Array ConstInitVals InitVals


%type <int_val> Number
%type <char_val> UnaryOp AddOp MulOp
%type <str_val> RelOp EqOp 

%%

// lv1
CompUnit
  : ConstDecl {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->type = 0;
    comp_unit->const_decl = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  | ConstDecl CompUnit {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->type = 0;
    comp_unit->const_decl = unique_ptr<BaseAST>($1);
    comp_unit->next = move(ast);
    ast = move(comp_unit);
  }
  | FuncType IDENT RestCompUnit {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->type = 1;
    comp_unit->func_type = unique_ptr<BaseAST>($1);
    comp_unit->ident = *$2;
    comp_unit->rest_comp_unit = unique_ptr<BaseAST>($3);
    ast = move(comp_unit);
  }
  | FuncType IDENT RestCompUnit CompUnit {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->type = 1;
    comp_unit->func_type = unique_ptr<BaseAST>($1);
    comp_unit->ident = *$2;
    comp_unit->rest_comp_unit = unique_ptr<BaseAST>($3);
    comp_unit->next = move(ast);
    ast = move(comp_unit);
  }
  ;

RestCompUnit
  : '(' ')' Block {
    auto ast = new RestCompUnitAST();
    ast->type = 0;
    ast->block = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | '(' FuncFParams ')' Block {
    auto ast = new RestCompUnitAST();
    ast->type = 0;
    ast->func_f_params = unique_ptr<BaseAST>($2);
    ast->block = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  | ';' {
    auto ast = new RestCompUnitAST();
    ast->type = 1;
    $$ = ast;
  }
  | ',' VarDef ';' {
    auto ast = new RestCompUnitAST();
    ast->type = 1;
    ast->var_def = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '=' InitVal ';' {
    auto ast = new RestCompUnitAST();
    ast->type = 1;
    ast->init_val = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '=' InitVal ',' VarDef ';' {
    auto ast = new RestCompUnitAST();
    ast->type = 1;
    ast->init_val = unique_ptr<BaseAST>($2);
    ast->var_def = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  | ConstArray ';' {
    auto ast = new RestCompUnitAST();
    ast->type = 1;
    ast->const_array = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ConstArray ',' VarDef ';' {
    auto ast = new RestCompUnitAST();
    ast->type = 1;
    ast->const_array = unique_ptr<BaseAST>($1);
    ast->var_def = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | ConstArray '=' InitVal ';' {
    auto ast = new RestCompUnitAST();
    ast->type = 1;
    ast->const_array = unique_ptr<BaseAST>($1);
    ast->init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | ConstArray '=' InitVal ',' VarDef ';' {
    auto ast = new RestCompUnitAST();
    ast->type = 1;
    ast->const_array = unique_ptr<BaseAST>($1);
    ast->init_val = unique_ptr<BaseAST>($3);
    ast->var_def = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->label = "i32";
    $$ = ast;
  }
  | VOID {
    auto ast = new FuncTypeAST();
    ast->label = "void";
    $$ = ast;
  }
  ;

Block
  : '{' BlockItem '}' {
    auto ast = new BlockAST();
    ast->block_item = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new BlockAST();
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->type = 0;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new StmtAST();
    ast->type = 0;
    $$ = ast;
  }
  | LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->type = 1;
    ast->l_val = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new StmtAST();
    ast->type = 2;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ';' {
    auto ast = new StmtAST();
    ast->type = 2;
    $$ = ast;
  }
  | Block {
    auto ast = new StmtAST();
    ast->type = 3;
    ast->block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->type = 4;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt ELSE Stmt {
    auto ast = new StmtAST();
    ast->type = 4;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    ast->else_stmt = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->type = 5;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new StmtAST();
    ast->type = 6;
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new StmtAST();
    ast->type = 7;
    $$ = ast;
  }
  ;

// lv3
Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

PrimaryExp 
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->type = 0;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  } 
  | LVal {
    auto ast = new PrimaryExpAST();
    ast->type = 1;
    ast->l_val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST();
    ast->type = 2;
    ast->number = $1;
    $$ = ast;
  }
  ;

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
  | IDENT '(' ')' {
    auto ast = new UnaryExpAST();
    ast->type = 2;
    ast->ident = *$1;
    $$ = ast;
  }
  | IDENT '(' FuncRParams ')' {
    auto ast = new UnaryExpAST();
    ast->type = 2;
    ast->ident = *$1;
    ast->func_r_params = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

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
  ;

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
  ;

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
  ;

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
  ;

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
  ;

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
  ;

// lv4
Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast->type = 0;
    ast->const_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  } 
  | VarDecl {
    auto ast = new DeclAST();
    ast->type = 1;
    ast->var_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstDecl
  : CONST BType ConstDef ';' {
    auto ast = new ConstDeclAST();
    ast->b_type = unique_ptr<BaseAST>($2);
    ast->const_def = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *$1;
    ast->const_init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT '=' ConstInitVal ',' ConstDef {
    auto ast = new ConstDefAST();
    ast->ident = *$1;
    ast->const_init_val = unique_ptr<BaseAST>($3);
    ast->next = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IDENT ConstArray '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *$1;
    ast->const_array = unique_ptr<BaseAST>($2);
    ast->const_init_val = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  | IDENT ConstArray '=' ConstInitVal ',' ConstDef {
    auto ast = new ConstDefAST();
    ast->ident = *$1;
    ast->const_array = unique_ptr<BaseAST>($2);
    ast->const_init_val = unique_ptr<BaseAST>($4);
    ast->next = unique_ptr<BaseAST>($6);
    $$ = ast;
  }
  ;

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->type = 0;
    ast->const_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | '{' ConstInitVals '}' {
    auto ast = new ConstInitValAST();
    ast->type = 1;
    ast->const_init_vals = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new ConstInitValAST();
    ast->type = 1;
    $$ = ast;
  }
  ;

ConstInitVals
  : ConstInitVal {
    auto ast = new ConstInitValsAST();
    ast->const_init_val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ConstInitVal ',' ConstInitVals {
    auto ast = new ConstInitValsAST();
    ast->const_init_val = unique_ptr<BaseAST>($1);
    ast->next = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast->decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Decl BlockItem {
    auto ast = new BlockItemAST();
    ast->decl = unique_ptr<BaseAST>($1);
    ast->next = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | Stmt BlockItem {
    auto ast = new BlockItemAST();
    ast->stmt = unique_ptr<BaseAST>($1);
    ast->next = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

LVal
  : IDENT {
    auto ast = new LValAST();
    ast->label = *$1;
    $$ = ast;
  }
  | IDENT Array {
    auto ast = new LValAST();
    ast->label = *$1;
    ast->array = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

VarDecl
  : BType VarDef ';' {
    auto ast = new VarDeclAST();
    ast->b_type = unique_ptr<BaseAST>($1);
    ast->var_def = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->ident = *$1;
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *$1;
    ast->init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT ',' VarDef {
    auto ast = new VarDefAST();
    ast->ident = *$1;
    ast->next = unique_ptr<BaseAST>($3);
    $$ = ast;
  } 
  | IDENT '=' InitVal ',' VarDef {
    auto ast = new VarDefAST();
    ast->ident = *$1;
    ast->init_val = unique_ptr<BaseAST>($3);
    ast->next = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IDENT ConstArray {
    auto ast = new VarDefAST();
    ast->ident = *$1;
    ast->const_array = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | IDENT ConstArray '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *$1;
    ast->const_array = unique_ptr<BaseAST>($2);
    ast->init_val = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  | IDENT ConstArray ',' VarDef {
    auto ast = new VarDefAST();
    ast->ident = *$1;
    ast->const_array = unique_ptr<BaseAST>($2);
    ast->next = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  | IDENT ConstArray '=' InitVal ',' VarDef {
    auto ast = new VarDefAST();
    ast->ident = *$1;
    ast->const_array = unique_ptr<BaseAST>($2);
    ast->init_val = unique_ptr<BaseAST>($4);
    ast->next = unique_ptr<BaseAST>($6);
    $$ = ast;
  }
  ;

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->type = 0;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | '{' InitVals '}' {
    auto ast = new InitValAST();
    ast->type = 1;
    ast->init_vals = unique_ptr<BaseAST>($2);
    $$ = ast;
  } 
  | '{' '}' {
    auto ast = new InitValAST();
    ast->type = 1;
    $$ = ast;
  }
  ;

InitVals
  : InitVal {
    auto ast = new InitValsAST();
    ast->init_val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | InitVal ',' InitVals {
    auto ast = new InitValsAST();
    ast->init_val = unique_ptr<BaseAST>($1);
    ast->next = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// lv8
FuncFParams
  : FuncFParam {
    auto ast = new FuncFParamsAST();
    ast->func_f_param = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | FuncFParam ',' FuncFParams {
    auto ast = new FuncFParamsAST();
    ast->func_f_param = unique_ptr<BaseAST>($1);
    ast->next = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

FuncFParam
  : BType IDENT {
    auto ast = new FuncFParamAST();
    ast->type = 0;
    ast->b_type = unique_ptr<BaseAST>($1);
    ast->ident = *$2;
    $$ = ast;
  }
  | BType IDENT '[' ']' {
    auto ast = new FuncFParamAST();
    ast->type = 1;
    ast->b_type = unique_ptr<BaseAST>($1);
    ast->ident = *$2;
    $$ = ast;
  }
  | BType IDENT '[' ']' ConstArray {
    auto ast = new FuncFParamAST();
    ast->type = 1;
    ast->b_type = unique_ptr<BaseAST>($1);
    ast->ident = *$2;
    ast->const_array = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncRParams
  : Exp {
    auto ast = new FuncRParamsAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Exp ',' FuncRParams {
    auto ast = new FuncRParamsAST();
    ast->exp = unique_ptr<BaseAST>($1);
    ast->next = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// lv9
ConstArray
  : '[' ConstExp ']' {
    auto ast = new ConstArrayAST();
    ast->const_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '[' ConstExp ']' ConstArray {
    auto ast = new ConstArrayAST();
    ast->const_exp = unique_ptr<BaseAST>($2);
    ast->next = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  ;

Array
  : '[' Exp ']' {
    auto ast = new ArrayAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '[' Exp ']' Array {
    auto ast = new ArrayAST();
    ast->exp = unique_ptr<BaseAST>($2);
    ast->next = unique_ptr<BaseAST>($4);
    $$ = ast;
  }

// not ast_val
BType
  : INT {
    auto ast = new BTypeAST();
    ast->label = "i32";
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

UnaryOp
  : PLUS | MINUS | NOT {
    $$ = $1;
  }
  ;

AddOp
  : PLUS | MINUS {
    $$ = $1;
  }
  ;

MulOp
  : TIMES | DIVIDE | MOD {
    $$ = $1;
  }
  ;

RelOp
  : LT | GT | LE | GE {
    $$ = $1;
  }
  ;

EqOp
  : EQ | NE {
    $$ = $1;
  }
  ;

%%

void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
