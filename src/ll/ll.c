#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

#include <vector.h>
#include <bitset.h>

#include "ll.h"
#include "utils.h"

#define DIE fprintf(stderr, "ERROR: \"%s\", line %d: ", __FILE__, __LINE__), die

#ifdef DEBUG
#define DEBUGF fprintf(stderr, "DEBUG: \"%s\", line %d: ", __FILE__, __LINE__), printf(
#else
#define DEBUGF
#endif

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define AT(ARRAY, COLUMNS, Y, X) ((ARRAY)[(COLUMNS) * (Y) + (X)])

void input(), definitions(), tokens_decl(), tokens(), rules(), production(),
     rhs();

enum SymbolKind {
	TERMINAL, // @todo: should I rename it to TOKEN?
	NONTERMINAL,
	ACTION_SYMBOL
};

struct Symbol {
	enum SymbolKind kind;

	char *lexeme;

	size_t index;

	BitSet first;
	BitSet follow;

	bool nullable;

	Vector /* struct Production * */ productions;
};

struct Production {
	struct Symbol *lhs;

	Vector /* struct Symbol * */ rhs;
	size_t index;

	BitSet select;
};

FILE *INPUT;

char *SYMBOLKIND_TO_STR[] = {"TERMINAL", "NON-TERMINAL", "ACTION-SYMBOL"};

Vector /* struct Symbol * */ SYMBOLS;
Vector /* struct Symbol * */ TOKENS, NONTERMINALS, ACTIONS;

int *PARSE_TABLE;

Vector /* struct Production * */ PRODUCTIONS;

char *TOKENKIND_TO_STRING[] = {
	"EOI",
	"TOKEN-DIRECTIVE",
	"NAME",
	"ACTION",
	"CODE-BLOCK",
	"SEMI",
	"COLON",
	"SEPARATOR"
};

struct {
	struct Token token;
} LOOKAHEAD;

struct Symbol *make_symbol(char *lexeme, enum SymbolKind kind) {
	struct Symbol *s = calloc(1, sizeof(struct Symbol));
	s->kind = kind;
	s->lexeme = lexeme;

	switch (kind) {
	case TERMINAL:
		vector_append(TOKENS, &s);
		break;
	case NONTERMINAL:
		s->productions = VECTOR_NEW(struct Production *);
		vector_append(NONTERMINALS, &s);
		break;
	case ACTION_SYMBOL:
		vector_append(ACTIONS, &s);
		break;
	default:
		assert(false);
	}

	return s;
}

struct Symbol *find_symbol(char *s) {
	struct Symbol *sym;

	for (size_t i = 0; i < vector_len(TOKENS); ++i) {
		vector_get(TOKENS, i, &sym);

		if (!strcmp(s, sym->lexeme)) {
			return sym;
		}
	}

	for (size_t i = 0; i < vector_len(NONTERMINALS); ++i) {
		vector_get(NONTERMINALS, i, &sym);

		if (!strcmp(s, sym->lexeme)) {
			return sym;
		}
	}

	for (size_t i = 0; i < vector_len(ACTIONS); ++i) {
		vector_get(ACTIONS, i, &sym);

		if (!strcmp(s, sym->lexeme)) {
			return sym;
		}
	}

	return NULL;
}

struct Token eat(enum TokenKind kind) {
	assert(LOOKAHEAD.token.kind == kind);

	struct Token t = LOOKAHEAD.token;

	LOOKAHEAD.token = yylex();

	return t;
}

enum TokenKind peek_token() {
	return LOOKAHEAD.token.kind;
}

void start() {
	definitions();
	eat(SEPARATOR);
	rules();
	eat(SEPARATOR);
}

void definitions() {
	enum TokenKind k;
	switch ((k = peek_token())) {
	case TOKEN_DIRECTIVE:
		tokens_decl();
		definitions();
		break;
	case CODE_BLOCK:
		eat(CODE_BLOCK);
		definitions();
		break;
	case SEPARATOR: /* empty production */
		break;
	default:
		DIE("definitions didn't expect %s", TOKENKIND_TO_STRING[k]);
	}
}

void tokens_decl() {
	eat(TOKEN_DIRECTIVE);
	tokens();
	eat(SEMI);
}

void tokens() {
	enum TokenKind k;
	struct Token t;
	switch ((k = peek_token())) {
	case NAME:
		t = eat(NAME);
		assert(!find_symbol(t.lexeme));
		make_symbol(t.lexeme, TERMINAL);
		tokens();
		break;
	case SEMI: /* empty production */
		break;
	default:
		DIE("tokens didn't expect %s", TOKENKIND_TO_STRING[k]);
	}
}

void rules() {
	enum TokenKind k;
	switch ((k = peek_token())) {
	case NAME:
		production();
		rules();
		break;
	case SEPARATOR: /* empty production */
		break;
	default:
		DIE("rules didn't expect %s", TOKENKIND_TO_STRING[k]);
	}
}

void production() {
	struct Token t = eat(NAME);

	struct Production *p = calloc(1, sizeof(struct Production));
	struct Symbol *s;

	if (!(s = find_symbol(t.lexeme))) {
		s = make_symbol(t.lexeme, NONTERMINAL);
	}

	p->lhs = s;
	p->rhs = VECTOR_NEW(struct Symbol *);
	p->index = vector_len(PRODUCTIONS);
	vector_append(PRODUCTIONS, &p);

	vector_append(s->productions, &p);

	eat(COLON);
	rhs();
	eat(SEMI);
}

void rhs() {
	enum TokenKind k;
	struct Production *p;
	struct Token t;
	struct Symbol *s;

	vector_get(PRODUCTIONS, vector_len(PRODUCTIONS) - 1, &p);

	for (k = peek_token(); k != SEMI; k = peek_token()) {
		switch (k) {
		case NAME:
			t = eat(NAME);
			if (!(s = find_symbol(t.lexeme))) {
				s = make_symbol(t.lexeme, NONTERMINAL);
			}
			break;
		case ACTION:
			t = eat(ACTION);
			s = make_symbol(t.lexeme, ACTION_SYMBOL);
			break;
		default:
			DIE("rhs didn't expect %s", TOKENKIND_TO_STRING[k]);
		}

		vector_append(p->rhs, &s);
	}
}

void init() {
	//INPUT = fopen("sample.yy", "r");
	//if (!INPUT) {
	//	DIE("can't open file:");
	//}

	SYMBOLS = VECTOR_NEW(struct Symbol *);
	TOKENS = VECTOR_NEW(struct Symbol *);
	NONTERMINALS = VECTOR_NEW(struct Symbol *);
	ACTIONS = VECTOR_NEW(struct Symbol *);
	PRODUCTIONS = VECTOR_NEW(struct Production *);
}

void compute_nullable_and_check_left_recursion_helper(struct Symbol *sym,
						      BitSet final,
						      BitSet visited) {
	if (bitset_is_on(visited, sym->index)) {
		DIE("left recursion");
	}

	bitset_on(visited, sym->index);

	assert(!bitset_is_on(final, sym->index));

	BitSet nullables = bitset_new(vector_len(sym->productions));
	bitset_all_on(nullables);

	for (size_t i = 0; i < vector_len(sym->productions); ++i) {
		struct Production *prod;
		vector_get(sym->productions, i, &prod);

		for (size_t j = 0; j < vector_len(prod->rhs); ++j) {
			struct Symbol *s;
			vector_get(prod->rhs, j, &s);

			if (!bitset_is_on(final, s->index)) {
				compute_nullable_and_check_left_recursion_helper
				(s, final, visited);
			}

			if (!s->nullable) {
				bitset_off(nullables, i);
				break;
			}
		}
	}

	if(!bitset_len(nullables) || bitset_size(nullables) > 0) {
		sym->nullable = true;
	}

	bitset_free(&nullables);

	bitset_on(final, sym->index);
}

void compute_nullable_and_check_left_recursion() {
	struct Symbol *sym;
	struct Production *prod;

	BitSet final = bitset_new(vector_len(SYMBOLS));
	BitSet visited = bitset_new(vector_len(SYMBOLS));

	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		vector_get(SYMBOLS, i, &sym);

		if (sym->kind == TERMINAL) {
			bitset_on(final, i);
		}

		if (!bitset_is_on(final, i)) {
			compute_nullable_and_check_left_recursion_helper
			(sym, final, visited);
		}
	}

	bitset_free(&final);
	bitset_free(&visited);
}

void compute_first_helper(struct Symbol *sym, BitSet final) {
	assert(!bitset_is_on(final, sym->index));

	for (size_t i = 0; i < vector_len(sym->productions); ++i) {
		struct Production *p;
		vector_get(sym->productions, i, &p);

		for (size_t j = 0; j < vector_len(p->rhs); ++j) {
			struct Symbol *s;
			vector_get(p->rhs, j, &s);

			if (!bitset_is_on(final, s->index)) {
				compute_first_helper(s, final);
			}

			bitset_or(sym->first, s->first);

			if (!s->nullable) {
				break;
			}
		}
	}

	bitset_on(final, sym->index);
}

void compute_first() {
	BitSet final = bitset_new(vector_len(SYMBOLS));

	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		struct Symbol *sym;
		vector_get(SYMBOLS, i, &sym);

		sym->first = bitset_new(vector_len(SYMBOLS));

		if (sym->kind == TERMINAL) {
			bitset_on(sym->first, i);
			bitset_on(final, i);
		} else if (sym->kind == ACTION_SYMBOL) {
			bitset_on(final, i);
		}
	}

	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		struct Symbol *sym;
		vector_get(SYMBOLS, i, &sym);

		if (!bitset_is_on(final, i)) {
			compute_first_helper(sym, final);
		}
	}

	bitset_free(&final);
}

void compute_follow_helper(struct Symbol *sym, BitSet final, BitSet *note) {
	assert(!bitset_is_on(final, sym->index));

	// @todo: infinite recursion
	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		if (!bitset_is_on(note[sym->index], i)) {
			continue;
		}

		struct Symbol *lhs;
		vector_get(SYMBOLS, i, &lhs);

		assert(sym != lhs);

		if (!bitset_is_on(final, lhs->index)) {
			compute_follow_helper(lhs, final, note);
		}

		bitset_or(sym->follow, lhs->follow);
	}


	bitset_on(final, sym->index);
}

void compute_follow() {
	BitSet *note = calloc(vector_len(SYMBOLS), sizeof(BitSet));

	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		note[i] = bitset_new(vector_len(SYMBOLS));

		struct Symbol *sym;
		vector_get(SYMBOLS, i, &sym);
		sym->follow = bitset_new(vector_len(SYMBOLS));
	}

	for (size_t i = 0; i < vector_len(PRODUCTIONS); ++i) {
		struct Production *p;
		vector_get(PRODUCTIONS, i, &p);

		for (size_t j = 0; j < vector_len(p->rhs); ++j) {
			struct Symbol *cur, *nxt;
			vector_get(p->rhs, j, &cur);

			if (cur->kind != NONTERMINAL) {
				continue;
			}

			for (size_t k = j + 1; k < vector_len(p->rhs); ++k) {
				vector_get(p->rhs, k, &nxt);

				bitset_or(cur->follow, nxt->first);

				if (!nxt->nullable) {
					break;
				}

				if (k + 1 >= vector_len(p->rhs) && cur != p->lhs) {
					// when all next symbols are nullable
					bitset_on(note[cur->index], p->lhs->index);
				}
			}

			if (j + 1 >= vector_len(p->rhs) && cur != p->lhs) {
				// when `cur` is a last symbol in a production
				bitset_on(note[cur->index], p->lhs->index);
			}
		}
	}

	BitSet final = bitset_new(vector_len(SYMBOLS));
	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		struct Symbol *sym;
		vector_get(SYMBOLS, i, &sym);

		if (!bitset_is_on(final, i)) {
			compute_follow_helper(sym, final, note);
		}
	}

	bitset_free(&final);
	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		bitset_free(&note[i]);
	}
	free(note);
}

void compute_select_sets() {
	for (size_t i = 0; i < vector_len(PRODUCTIONS); ++i) {
		struct Production *p;
		vector_get(PRODUCTIONS, i, &p);
		p->select = bitset_new(vector_len(SYMBOLS));
		for (size_t j = 0; j < vector_len(p->rhs); ++j) {
			struct Symbol *s;
			vector_get(p->rhs, j, &s);

			bitset_or(p->select, s->first);
			if (!s->nullable) {
				break;
			}

			if (j + 1 >= vector_len(p->rhs)) {
				bitset_or(p->select, p->lhs->follow);
			}
		}

		if (!vector_len(p->rhs)) {
			bitset_or(p->select, p->lhs->follow);
		}
	}

	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		struct Symbol *lhs;
		vector_get(SYMBOLS, i, &lhs);

		if (lhs->kind != NONTERMINAL) {
			continue;
		}

		BitSet test = bitset_new(vector_len(SYMBOLS));
		size_t expectedSize = 0;
		for (size_t j = 0; j < vector_len(lhs->productions); ++j) {
			struct Production *p;

			vector_get(lhs->productions, j, &p);

			expectedSize += bitset_size(p->select);
			bitset_or(test, p->select);
		}

		if (expectedSize != bitset_size(test)) {
			DIE("selection set conflict for productions with the "
			    "left-hand side `%s`", lhs->lexeme);
		}

		bitset_free(&test);
	}
};

void create_parse_table() {
	size_t rows = vector_len(NONTERMINALS);
	size_t cols = vector_len(TOKENS);

	PARSE_TABLE = calloc(rows * cols, sizeof(int));
	memset(PARSE_TABLE, -1, rows * cols * sizeof(int));

	for (size_t i = 0; i < rows; ++i) {
		struct Symbol *lhs;
		vector_get(NONTERMINALS, i, &lhs);

		for (size_t j = 0; j < vector_len(lhs->productions); ++j) {
			struct Production *p;
			vector_get(lhs->productions, j, &p);

			size_t k = bitset_len(p->select);
			while (bitset_next(p->select, &k)) {
				assert(k < rows);
				AT(PARSE_TABLE, cols, i, k) = p->index;
			}
		}
	}
}

void generate_code() {
	printf("int LL_PRODUCTION_NSYMBOLS[%d] = {", vector_len(PRODUCTIONS));
	for (size_t i = 0; i < vector_len(PRODUCTIONS); ++i) {
		struct Production *p;
		vector_get(PRODUCTIONS, i, &p);

		printf("%d", vector_len(p->rhs));

		if (i + 1 < vector_len(PRODUCTIONS)) {
			printf(", ");
		}
	}
	printf("};\n");

	printf("int LL_PRODUCTION_OFFSET[%d] = {", vector_len(PRODUCTIONS));
	size_t offset = 0;
	for (size_t i = 0; i < vector_len(PRODUCTIONS); ++i) {
		struct Production *p;
		vector_get(PRODUCTIONS, i, &p);

		printf("%d", offset);

		offset += vector_len(p->rhs);

		if (i + 1 < vector_len(PRODUCTIONS)) {
			printf(", ");
		}
	}
	printf("};\n");

	printf("int LL_PRODUCTIONS[] = {\n");
	for (size_t i = 0; i < vector_len(PRODUCTIONS); ++i) {
		struct Production *p;
		vector_get(PRODUCTIONS, i, &p);

		printf("\t");
		for (size_t j = 0; j < vector_len(p->rhs); ++j) {
			struct Symbol *s;
			vector_get(p->rhs, j, &s);

			printf("%d", s->index);
			if (i + 1 < vector_len(PRODUCTIONS) ||
			    j + 1 < vector_len(p->rhs)) {
				printf(", ");
			}
		}
		printf("\n");
	}
	printf("};\n");

	printf("char *LL_NAME[] = {\n");
	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		struct Symbol *s;
		vector_get(SYMBOLS, i, &s);

		if (s->kind == ACTION_SYMBOL) {
			printf("\t\"action%d\"", i);
		} else {
			printf("\t\"%s\"", s->lexeme);
		}

		if (i + 1 < vector_len(SYMBOLS)) {
			printf(",");
		}
		printf("\n");
	};
	printf("};\n");

	printf("#define LL_NONTERMINALS %d\n", vector_len(NONTERMINALS));
	printf("#define LL_TOKENS %d\n", vector_len(TOKENS));
	printf("#define LL_ACTIONS %d\n", vector_len(ACTIONS));

	printf("int LL_PARSE_TABLE[LL_NONTERMINALS][LL_TOKENS] = {\n");
	for (size_t i = 0; i < vector_len(NONTERMINALS); ++i) {
		printf("\t{");
		for (size_t j = 0; j < vector_len(TOKENS); ++j) {
			printf("%d", AT(PARSE_TABLE, vector_len(TOKENS), i, j));

			if (j + 1 < vector_len(TOKENS)) {
				printf(", ");
			}
		}

		printf("}");

		if (i + 1 < vector_len(NONTERMINALS)) {
			printf(",");
		}
		printf("\n");
	}
	printf("};\n");

	printf("#define LL_IS_TOKEN(SYMBOL) "
	       "(%d <= (SYMBOL) && (SYMBOL) < %d)\n",
	       0,
	       vector_len(TOKENS));

	printf("#define LL_IS_NONTERMINAL(SYMBOL) "
	       "(%d <= (SYMBOL) && (SYMBOL) < %d)\n",
	       vector_len(TOKENS),
	       vector_len(TOKENS) + vector_len(NONTERMINALS));

	printf("#define LL_IS_ACTION(SYMBOL) "
	       "(%d <= (SYMBOL) && (SYMBOL) < %d)\n",
	       vector_len(TOKENS) + vector_len(NONTERMINALS),
	       vector_len(TOKENS) + vector_len(NONTERMINALS)
	       + vector_len(ACTIONS));

	printf("#define LL_SYMBOLS %d\n",
	       vector_len(TOKENS) + vector_len(NONTERMINALS)
	       + vector_len(ACTIONS));

	printf("LL_TYPE LL_VALUE[LL_SYMBOLS];\n");
	printf("LL_TYPE LL_ROOT;\n");

	printf(""
	"void LL_process_action(int action) {\n"
	"	switch (action) {\n");
	for (size_t i = 0; i < vector_len(ACTIONS); ++i) {
		struct Symbol *action;
		vector_get(ACTIONS, i, &action);

		printf("\tcase %d: {", action->index);

		for (char *p = action->lexeme; *p != '\0'; ++p) {
			if (*p != '$') {
				putchar(*p);
				continue;
			}

			++p;

			if (!(*p == '$' || isdigit(*p))) {
				putchar('$');
				--p;
				continue;
			}

			if (*p == '$') {
				printf("LL_ROOT");
				continue;
			}

			if (*p == '0') {
				printf("LL_VALUE[action]");
				continue;
			}

			printf("LL_VALUE[LL_get_ith(");
			do {
				putchar(*p);
				++p;
			} while (isdigit(*p));
			--p;
			printf(")]");
		}

		printf("}; break;\n");
	}

	printf("\t}\n}\n");


	FILE *f = fopen("code.c", "r");

	char c;
	while ((c = fgetc(f)) != EOF) {
		putchar(c);
	}

	fclose(f);
}

void fancy_print_table(int *table, size_t nrows, size_t ncolumns) {

#define MAX_COLUMN_WIDTH 32

	size_t *columnWidth = calloc(ncolumns + 1, sizeof(size_t));

	for (size_t j = 0; j < ncolumns; ++j) {
		struct Symbol *t;
		vector_get(SYMBOLS, j, &t);

		assert(j == t->index);

		columnWidth[j + 1] = strnlen(t->lexeme, MAX_COLUMN_WIDTH);
	}

	char buf[MAX_COLUMN_WIDTH + 1];

	for (size_t i = 0; i < nrows; ++i) {
		struct Symbol *non;

		vector_get(SYMBOLS, i + ncolumns, &non);

		size_t len = strnlen(non->lexeme, MAX_COLUMN_WIDTH);

		if (len > columnWidth[0]) {
			columnWidth[0] = len;
		}
	}

	for (size_t i = 0; i < nrows; ++i) {
		for (size_t j = 0; j < ncolumns; ++j) {
			snprintf(buf, MAX_COLUMN_WIDTH, "%d",
				 table[ncolumns * i + j]);

			buf[MAX_COLUMN_WIDTH] = '\0';

			if (strlen(buf) > columnWidth[j + 1]) {
				columnWidth[j + 1] = strlen(buf);
			}
		}
	}

	fprintf(stderr, "| ");
	for (size_t i = 0; i < columnWidth[0]; ++i) {
		fprintf(stderr, " ");
	}
	fprintf(stderr, " | ");
	for (size_t i = 0; i < ncolumns; ++i) {
		struct Symbol *t;
		vector_get(SYMBOLS, i, &t);

		fprintf(stderr, "%*.*s", columnWidth[i + 1], columnWidth[i + 1],
			t->lexeme);
		fprintf(stderr, " | ");
	}
	fprintf(stderr, "\n");

	for (size_t i = 0; i < nrows; ++i) {
		struct Symbol *non;
		vector_get(SYMBOLS, i + ncolumns, &non);

		fprintf(stderr, "| ");

		fprintf(stderr, "%*.*s", columnWidth[0], columnWidth[0],
			non->lexeme);

		fprintf(stderr, " | ");

		for (size_t j = 0; j < ncolumns; ++j) {
			snprintf(buf, MAX_COLUMN_WIDTH + 1, "%d",
				 table[ncolumns * i + j]);

			buf[MAX_COLUMN_WIDTH] = '\0';

			fprintf(stderr, "%*.*s", columnWidth[j + 1],
			       columnWidth[j + 1], buf);
			fprintf(stderr, " | ");
		}
		fprintf(stderr, "\n");
	}
}

int main(int argc, char *argv[]) {
	struct Symbol *sym;
	struct Production *prod;

	init();
	eat(0);
	start();

	vector_extend(SYMBOLS, TOKENS);
	vector_extend(SYMBOLS, NONTERMINALS);
	vector_extend(SYMBOLS, ACTIONS);

	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		vector_get(SYMBOLS, i, &sym);
		sym->index = i;
	}

#ifdef DEBUG
	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		vector_get(SYMBOLS, i, &sym);
		fprintf(stderr, "%d) [%s] `%s`\n", sym->index,
		        SYMBOLKIND_TO_STR[sym->kind], sym->lexeme);
	}
	for (size_t i = 0; i < vector_len(PRODUCTIONS); ++i) {
		vector_get(PRODUCTIONS, i, &prod);
		fprintf(stderr, "%d) %s :", i, prod->lhs->lexeme);
		for (size_t j = 0; j < vector_len(prod->rhs); ++j) {
			vector_get(prod->rhs, j, &sym);
			fprintf(stderr, " `%s`", sym->lexeme);
		}
		fprintf(stderr, "\n");
	}
#endif

	compute_nullable_and_check_left_recursion();
	compute_first();
	//compute_follow();
	//compute_select_sets();
	//create_parse_table();
	//generate_code();


#ifdef DEBUG
	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
		vector_get(SYMBOLS, i, &sym);

		fprintf(stderr, "`%s` is %snullable\n", sym->lexeme,
			sym->nullable ? "" : "not ");
	}

//	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
//		vector_get(SYMBOLS, i, &sym);
//
//		fprintf(stderr, "FIRST(%s) = {\n", sym->lexeme);
//		size_t j = bitset_len(sym->first);
//		while (bitset_next(sym->first, &j)) {
//			struct Symbol *s;
//			vector_get(SYMBOLS, j, &s);
//			fprintf(stderr, "\t`%s`\n", s->lexeme);
//		}
//		fprintf(stderr, "}\n");
//	}
//
//	for (size_t i = 0; i < vector_len(SYMBOLS); ++i) {
//		vector_get(SYMBOLS, i, &sym);
//
//		fprintf(stderr, "FOLLOW(%s) = {\n", sym->lexeme);
//		size_t j = bitset_len(sym->follow);
//		while (bitset_next(sym->follow, &j)) {
//			struct Symbol *s;
//			vector_get(SYMBOLS, j, &s);
//			fprintf(stderr, "\t`%s`\n", s->lexeme);
//		}
//		fprintf(stderr, "}\n");
//	}
//
//	for (size_t i = 0; i < vector_len(PRODUCTIONS); ++i) {
//		struct Production *p;
//		vector_get(PRODUCTIONS, i, &p);
//		fprintf(stderr, "SELECTION(%d:%s) = {\n", i, p->lhs->lexeme);
//
//		size_t j = bitset_len(p->select);
//		while (bitset_next(p->select, &j)) {
//			struct Symbol *s;
//			vector_get(SYMBOLS, j, &s);
//			fprintf(stderr, "\t`%s`\n", s->lexeme);
//		}
//
//		fprintf(stderr, "}\n");
//	}
//
//	fancy_print_table(PARSE_TABLE, vector_len(NONTERMINALS),
//			  vector_len(TOKENS));
#endif

	if (peek_token() == ACTION) { // @todo: fix this
				      // some time in the future...
		struct Token t = eat(ACTION);
		printf("%s", t.lexeme);
	}

	return 0;
}
