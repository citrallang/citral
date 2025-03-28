#pragma once
#include <stdint.h>




typedef enum TokenType {
	TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT, TOKEN_CARET,
	TOKEN_PLUSEQ, TOKEN_MINUSEQ, TOKEN_STAREQ, TOKEN_SLASHEQ, TOKEN_PERCENTEQ, TOKEN_CARETEQ,

	TOKEN_EQ, TOKEN_EQEQ, TOKEN_BANG, TOKEN_BANGEQ, TOKEN_AMPAMP, TOKEN_BARBAR,

	TOKEN_STRING, TOKEN_NUMBER, TOKEN_IDENTIFIER,

	TOKEN_EOF, TOKEN_START,
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
	size_t curLine;
	scannerToken* tokBuf;
	size_t numToks;
	size_t toksCapacity;
} scannerState;
scannerState* scanner_scan_source(char* src, size_t bufSize);
scannerState* scanner_create_state(char* buf, size_t bufSize);
scannerToken scanner_next_token(scannerState* state);
scannerToken scanner_create_token(scannerState* state, TokenType type, size_t size);
char scanner_advance(scannerState* state);
char scanner_match(scannerState* state, char matchAgainst);
//char scanner_expect(scannerState* state, char expectThis); //todo
char scanner_peek(scannerState* state);
uint8_t scanner_is_at_end(scannerState* state);
void scanner_insert_token(scannerState* state, scannerToken token);
void scanner_print_token(scannerToken tok);