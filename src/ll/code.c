#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

char *LL_LEXEME;
int LL_SIZE;

#define LL_TYPE double

struct LL_Node {
	struct LL_Node *next;
	int sym;
} *LL_STACK;

int LL_empty() {
	return LL_STACK == NULL;
}

int LL_pop() {
	assert(LL_STACK);

	struct LL_Node *n;

	n = LL_STACK;
	LL_STACK = n->next;

	int s = n->sym;
	free(n);

	return s;
}

int LL_get_ith(int i) {
	assert(LL_STACK);

	int cnt = 1;
	for (struct LL_Node *it = LL_STACK; it; it = it->next) {
		if (i == cnt) {
			return it->sym;
		}
		++cnt;
	}

	printf("unreachable\n");
	exit(1);

	return -1;
}

void LL_push(int sym) {
	struct LL_Node *n;

	n = calloc(1, sizeof(struct LL_Node));
	n->sym = sym;
	n->next = LL_STACK;

	LL_STACK = n;
}

int LL_LOOKAHED;

int LL_peek_token() {
	return LL_LOOKAHED;
}

int LL_advance_token() {
	LL_LOOKAHED = LL_GET_TOKEN();
}

void LL_parse() {
	// the first call to initialize the lookahead
	LL_advance_token();
	// a first defined non-terminal is the `start` symbol
	LL_push(LL_TOKENS);

	int symbol;
	int token;
	while (!LL_empty()) {
		// printf("[");
		// for (struct LL_Node *it = LL_STACK; it; it = it->next) {
		// 	printf(" %s", LL_NAME[it->sym]);
		// }
		// printf("\n");
		symbol = LL_pop();
		// printf("symbol: %s\n", LL_NAME[symbol]);
		token = LL_peek_token();
		// printf("token: %s\n", LL_NAME[token]);

		if (LL_IS_TOKEN(symbol)) {
			assert(symbol == token);
			// printf("> %s\n", LL_LEXEME);
			LL_advance_token();
		} else if (LL_IS_NONTERMINAL(symbol)) {
			int p = LL_PARSE_TABLE[symbol - LL_TOKENS][token];
			assert(p != -1);
			int i = LL_PRODUCTION_NSYMBOLS[p] - 1;
			for (; i >= 0; --i) {
				int index = i + LL_PRODUCTION_OFFSET[p];
				LL_push(LL_PRODUCTIONS[index]);
			}

		} else if (LL_IS_ACTION(symbol)) {
			LL_process_action(symbol);
		} else {
			printf("unreachable\n");
			exit(1);
		}
	}
}
