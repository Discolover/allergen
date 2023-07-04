%token EOI
%token TOKEN_DIRECTIVE, NAME, ACTION, CODE_BLOCK, SEMI, COLON, SEPARATOR

%%
start: definitions SEPARATOR rules SEPARATOR /* code */

definitions: tokens_decl definitions;
definitions: CODE_BLOCK definitions;
definitions:;

tokens_decl: TOKEN_DIRECTIVE tokens SEMI;

tokens: NAME tokens;
tokens:;

rules: production rules;
rules:;
production: NAME COLON rhs SEMI;
rhs: NAME rhs;
rhs: ACTION rhs;
rhs:;
%%
