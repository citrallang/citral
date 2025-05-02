#pragma once
#include <stdint.h>




typedef enum TokenType {
	TOKEN_PLUS=0, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT, TOKEN_CARET, TOKEN_TILDE,
	TOKEN_PLUSEQ, TOKEN_MINUSEQ, TOKEN_STAREQ, TOKEN_SLASHEQ, TOKEN_PERCENTEQ, TOKEN_CARETEQ, TOKEN_PLUSPLUS, TOKEN_MINUSMINUS,

	TOKEN_EQ, TOKEN_EQEQ, TOKEN_BANG, TOKEN_BANGEQ, TOKEN_AMPAMP, TOKEN_BARBAR,
	
	TOKEN_OPENPAREN, TOKEN_CLOSEPAREN, TOKEN_OPENBRACK, TOKEN_CLOSEBRACK, TOKEN_OPENBRACE, TOKEN_CLOSEBRACE,

	TOKEN_STRING, TOKEN_INT, TOKEN_FLOAT, TOKEN_IDENTIFIER, TOKEN_CHAR,

	TOKEN_EOF, TOKEN_START, TOKEN_ERROR,
} TokenType;

static const char* TOKEN_NAMES[] = {
	"+", "-", "*", "/", "%", "^", "~", 
	"+=", "-=", "*=", "/=", "%=", "^=", "++", "--",

	"=", "==", "!", "!=", "&&", "||",

	"(", ")", "[", "]", "{", "}", "STRING", "INT", "FLOAT", "IDENTIFIER", "CHAR", "END OF FILE", "START OF FILE", "ERROR",
};

static const char* UNEXPECTED_TOKEN[] = {
	"Unexpected token: '+'", "Unexpected token: '-'", "Unexpected token: '*'", "Unexpected token: '/'", "Unexpected token: '%'", "Unexpected token: '^'", "Unexpected token: '~'", 
	"Unexpected token: '+='", "Unexpected token: '-='", "Unexpected token: '*='", "Unexpected token: '/='", "Unexpected token: '%='", "Unexpected token: '^='", "Unexpected token: '++'", "Unexpected token: '--'", 
	"Unexpected token: '='", "Unexpected token: '=='", "Unexpected token: '!'", "Unexpected token: '!='", "Unexpected token: '&&'", "Unexpected token: '||'", 
	"Unexpected token: '('", "Unexpected token: ')'", "Unexpected token: '['", "Unexpected token: ']'", "Unexpected token: '{'", "Unexpected token: '}'", 
	"Unexpected token: 'STRING'", "Unexpected token: 'INT'", "Unexpected token: 'FLOAT'", "Unexpected token: 'IDENTIFIER'", "Unexpected token: 'CHAR'", 
	"Unexpected token: 'END OF FILE'", "Unexpected token: 'START OF FILE'", "Unexpected token: 'ERROR'",
};

typedef struct ScannerToken {
	TokenType type;
	int line;
	char* posInSrc;
	size_t numChars;
} ScannerToken;
typedef struct ScannerState {
	char* buf;
	size_t bufCapacity;
	char* cur;
	int curLine;
	ScannerToken* tokBuf;
	size_t numToks;
	size_t toksCapacity;
	int hadError : 1;
	int isSrcHeap : 1;
} ScannerState;
ScannerToken scanner_error_token(ScannerState* state, char* msg, char* posInSrc, size_t numChars, int lineInSrc);
void scanner_error(ScannerState* state, char* msg, char* posInSrc, size_t numChars, int lineInSrc);
ScannerState* scanner_scan_full_source(char* src, size_t bufSize, int isHeap);
ScannerState* scanner_create_state(char* buf, size_t bufSize, int isHeap);
ScannerToken scanner_next_token(ScannerState* state);
ScannerToken scanner_create_token(ScannerState* state, TokenType type, size_t size);
char scanner_advance(ScannerState* state);
#define scanner_backtrack(state) state->cur-- 
uint8_t scanner_match(ScannerState* state, char matchAgainst);
char scanner_expect(ScannerState* state, char expectThis);
char scanner_peek(ScannerState* state);
uint8_t scanner_is_at_end(ScannerState* state);
void scanner_insert_token(ScannerState* state, ScannerToken token);
void scanner_print_token(ScannerToken tok);
void scanner_dump_print_tokens(ScannerState* state);
ScannerToken scanner_short_string(ScannerState* state, char first);
ScannerToken scanner_number(ScannerState* state, char first);
ScannerToken scanner_identifier(ScannerState* state, char first);
void scanner_short_comment(ScannerState* state);
void scanner_long_comment(ScannerState* state);
int scanner_is_alpha(char isThis);
int scanner_is_numeric(char isThis);
void scanner_free_state(ScannerState* state);