%token SEMI PLUS TIMES LP RP NUMBER;

%{
extern char *LL_LEXEME;

int get_token();
#include <stdio.h>
#include <assert.h>
#define LL_GET_TOKEN get_token

%}

/* 4 * 5 + 2 + 3*/

%%
stmt: expr SEMI { printf("> %d\n", $$);} ; /* 0 */

expr: term expr_; /* 1 */
expr: /* empty comment */ ; /* 2 */

expr_: PLUS { $2 = $$ ;} term { $$ += $0;} expr_ ; /* 3 */
expr_: ; /* 4 */

term: factor term_; /* 5 */

term_: TIMES { $2 = $$ ;} factor { $$ *= $0;} term_ ; /* 6 */

term_:; /* 7 */

factor: LP expr RP; /* 8 */
factor: { $$ = atoi(LL_LEXEME) ; } NUMBER; /* 9 */

%%

{

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int get_token() {
	char c;

	while (isspace((c = getchar()))) {
		;
	}

	if (!LL_SIZE) {
		LL_LEXEME = calloc(8, sizeof(*LL_LEXEME));
		LL_SIZE = 8;
	}

	LL_LEXEME[0] = '\0';
	LL_LEXEME[1] = '\0';
	if (c == ';') {
		*LL_LEXEME = ';';
		return 0;
	} else if (c == '+') {
		*LL_LEXEME = '+';
		return 1;
	} else if (c == '*') {
		*LL_LEXEME = '*';
		return 2;
	} else if (c == '(') {
		*LL_LEXEME = '(';
		return 3;
	} else if (c == ')') {
		*LL_LEXEME = ')';
		return 4;
	} else if (isdigit(c)) {
		LL_LEXEME[0] = c;
		assert(LL_SIZE > 1);

		int i = 1;
		for (;;) {
			c = getchar();

			if (!isdigit(c)) {
				break;
			}

			if (i >= LL_SIZE) {
				LL_SIZE *= 2;
				LL_LEXEME = realloc(LL_LEXEME, LL_SIZE);
			}

			LL_LEXEME[i] = c;
			++i;
		}
		LL_LEXEME[i] = '\0';

		ungetc(c, stdin);

		return 5;
	} else if (c == EOF) {
		return -1;
	}

	printf("unrecognized character %c", c);
	exit(1);
}

int main() {
	LL_parse();
	return 0;
}
}
