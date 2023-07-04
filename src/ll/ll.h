enum TokenKind {
	EOI,
	TOKEN_DIRECTIVE,
	NAME,
	ACTION,
	CODE_BLOCK,
	SEMI,
	COLON,
	SEPARATOR
};

struct Token {
	enum TokenKind kind;
	// char *name;
	char *lexeme;
};

struct Token yylex();
