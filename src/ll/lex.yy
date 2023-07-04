%option noyywrap

%{
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "ll.h"

#define BUF_SZ 1024
#define YY_DECL struct Token yylex()
%}

%x comment

%x literal_block
%x string
%x character

%x action

%%
	char BUF[BUF_SZ];
	char *p;
	int depth = 0;
	int comment_caller;

^%token {
	return (struct Token) {
		.kind = TOKEN_DIRECTIVE,
		.lexeme = "%token"
	};
	}

[a-zA-Z][_a-zA-Z0-9]* {
	return (struct Token) {
		.kind = NAME,
		.lexeme = strdup(yytext)
	};
	}

; {return (struct Token) {.kind = SEMI, .lexeme = ";"};}

: {return (struct Token) {.kind = COLON, .lexeme = ":"};}

^%% {return (struct Token) {.kind = SEPARATOR, .lexeme = "%%"};}

<INITIAL,action>"/*" comment_caller = YY_START; BEGIN(comment);

<comment>[^*]*
<comment>("*"+)[^/*]*
<comment><<EOF>>	{
	printf("unterminated comment\n");
	return (struct Token) {.kind = EOI};
	}
<comment>("*"+)"/" BEGIN(comment_caller);

^"%{" BEGIN(literal_block);
<literal_block>[^%]+ ECHO;
<literal_block>%+[^}%]* ECHO;
<literal_block><<EOF>>	{
	printf("unterminated literal block\n");
	return (struct Token) {.kind = EOI};
	}
<literal_block>^"%}" BEGIN(INITIAL);

[ \r\t\n]*



"{" BEGIN(action); assert(!depth); depth = 1; p = BUF;



<action>\{	{
	++depth;
	*p++ = '{';
	}

<action>\"([^"]|\\\")*\"	{
	for (char *q = yytext; q < yytext + yyleng; ++q) {
		*p++ = *q;
	}
	}

<action>'([^']|\\')+'	{
	for (char *q = yytext; q < yytext + yyleng; ++q) {
		*p++ = *q;
	}
}

<action><<EOF>>	{
	printf("unterminated action\n");
	return (struct Token) {.kind = EOI};
	}

<action>[^}] assert(yyleng == 1); *p++ = *yytext;

<action>"}"	{
	--depth;
	if (!depth) {
		BEGIN(INITIAL);

		*p = '\0';
		return (struct Token) {
			.kind = ACTION,
			.lexeme = strdup(BUF)
		};
	}
	*p++ = '}';
	}



.	{
	printf("bad character: %c\n", *yytext);
	return (struct Token) {.kind = EOI};
	}

<<EOF>> return (struct Token) {.kind = EOI};

%%
