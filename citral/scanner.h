#pragma once
#include <stdint.h>




typedef enum TokenType {
	TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT, TOKEN_CARET, TOKEN_TILDE,
	TOKEN_PLUSEQ, TOKEN_MINUSEQ, TOKEN_STAREQ, TOKEN_SLASHEQ, TOKEN_PERCENTEQ, TOKEN_CARETEQ, TOKEN_PLUSPLUS, TOKEN_MINUSMINUS,

	TOKEN_EQ, TOKEN_EQEQ, TOKEN_BANG, TOKEN_BANGEQ, TOKEN_AMPAMP, TOKEN_BARBAR,
	
	TOKEN_OPENPAREN, TOKEN_CLOSEPAREN, TOKEN_OPENBRACK, TOKEN_CLOSEBRACK, TOKEN_OPENBRACE, TOKEN_CLOSEBRACE,

	TOKEN_STRING, TOKEN_INT, TOKEN_FLOAT, TOKEN_IDENTIFIER, TOKEN_CHAR,

	TOKEN_EOF, TOKEN_START, TOKEN_ERROR,
} TokenType;

typedef struct scannerToken {
	TokenType type;
	char* posInSrc;
	size_t numChars;
	int line;
} scannerToken;
typedef struct scannerState {
	char* buf;
	size_t bufCapacity;
	char* cur;
	int curLine;
	scannerToken* tokBuf;
	size_t numToks;
	size_t toksCapacity;
	int hadError : 1;
	int isSrcHeap : 1;
} scannerState;
void scanner_error(scannerState* state, char* msg, char* posInSrc, size_t numChars, int lineInSrc);
scannerState* scanner_scan_source(char* src, size_t bufSize, int isHeap);
scannerState* scanner_create_state(char* buf, size_t bufSize, int isHeap);
scannerToken scanner_next_token(scannerState* state);
scannerToken scanner_create_token(scannerState* state, TokenType type, size_t size);
char scanner_advance(scannerState* state);
#define scanner_backtrack(state) state->cur-- 
uint8_t scanner_match(scannerState* state, char matchAgainst);
char scanner_expect(scannerState* state, char expectThis); //todo
char scanner_peek(scannerState* state);
uint8_t scanner_is_at_end(scannerState* state);
void scanner_insert_token(scannerState* state, scannerToken token);
void scanner_print_token(scannerToken tok);
void scanner_dump_print_tokens(scannerState* state);
scannerToken scanner_short_string(scannerState* state, char first);
scannerToken scanner_number(scannerState* state, char first);
scannerToken scanner_identifier(scannerState* state, char first);
void scanner_short_comment(scannerState* state);
void scanner_long_comment(scannerState* state);
int scanner_is_alpha(char isThis);
int scanner_is_numeric(char isThis);
void scanner_free_state(scannerState* state);