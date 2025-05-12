#include "scanner.h"
#include "allocator.h"
#include <stdio.h>
char scanner_advance(ScannerState* state) {
	if (scanner_is_at_end(state)) {
		return 0;
	}
	return *(state->cur++);
}

char scanner_peek(ScannerState* state) {
	return *state->cur;
}

uint8_t scanner_match(ScannerState* state, char matchAgainst) {
	char t = *state->cur;
	uint8_t eq = t == matchAgainst;
	state->cur += eq;
	return eq;
}

char scanner_expect(ScannerState* state, char expectThis) {
	char response = scanner_advance(state);
	if (response != expectThis) {
		char* buf = xmalloc(128);
		if (response == '\n') {
			snprintf(buf, 128, "Expected %c after %c at line %d, found \\n.", expectThis, *(state->cur - 2), state->curLine);
			state->curLine++;
			scanner_error(state, buf, state->cur - 2, 1, state->curLine);
		}
		else {
			snprintf(buf, 128, "Expected %c after %c at line %d, found %c.", expectThis, *(state->cur - 2), state->curLine, response);
			scanner_error(state, buf, state->cur - 2, 2, state->curLine);
		}
		free(buf);
	}
	return response;
}

uint8_t scanner_is_at_end(ScannerState* state) {
	if ((state->cur - state->buf) >= (long long)state->bufCapacity) {
		return 1;
	}
	return 0;
}

ScannerToken scanner_create_token(ScannerState* state, TokenType tokenType, size_t size) {
	ScannerToken tok = {
		.type = tokenType,
		.line = state->curLine,
		.posInSrc = state->cur - size,
		.numChars = size,
	};
	return tok;
}

ScannerState* scanner_spawn_state() {
	ScannerState* state = xmalloc(sizeof(ScannerState));
	return state;
}

void scanner_free_state(ScannerState* state) {
	free(state->tokBuf);
	if (state->isSrcHeap)
		free(state->buf);
	free(state);
}

ScannerState* scanner_create_state(char* buf, size_t bufSize, int isHeap) {
	ScannerState* state = xmalloc(sizeof(ScannerState));
	state->buf = buf;
	state->bufCapacity = bufSize;
	state->cur = buf;
	state->toksCapacity = 16;
	state->tokBuf = xmalloc(sizeof(ScannerToken) * 16);
	state->curLine = 1;
	state->isSrcHeap = isHeap;
	return state;
}



void scanner_error(ScannerState* state, char* msg, char* posInSrc, size_t numChars, int lineInSrc) {
	scanner_insert_token(state, scanner_error_token(state, msg, posInSrc, numChars, lineInSrc));
}

ScannerToken scanner_error_token(ScannerState* state, char* msg, char* posInSrc, size_t numChars, int lineInSrc) {
	fprintf(stderr, msg);
	fprintf(stderr, "\n");
	state->hadError = 1;
	ScannerToken tok = {
		.line = lineInSrc,
		.numChars = numChars,
		.posInSrc = posInSrc,
		.type = TOKEN_ERROR
	};
	return tok;
}
ScannerToken scanner_next_token(ScannerState* state) {
scanner_start_of_next_token:
	char next = scanner_advance(state);
	switch (next) {
	case '\0': {
		ScannerToken tok = scanner_create_token(state, TOKEN_EOF, 0);
		return tok;
	}
	case '+': {
		if (scanner_match(state, '+')) {
			return scanner_create_token(state, TOKEN_PLUSPLUS, 2);
		}
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_PLUSEQ, 2);
		}
		return scanner_create_token(state, TOKEN_PLUS, 1);
	}
	case '-': {
		if (scanner_match(state, '-')) {
			return scanner_create_token(state, TOKEN_MINUSMINUS, 2);
		}
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_MINUSEQ, 2);
		}
		return scanner_create_token(state, TOKEN_MINUS, 1);
	}
	case '/': {
		if (scanner_match(state, '/')) {
			scanner_short_comment(state);
			goto scanner_start_of_next_token;
		}
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_SLASHEQ, 2);
		}
		if (scanner_match(state, '*')) {
			scanner_long_comment(state);
			goto scanner_start_of_next_token;
		}
		return scanner_create_token(state, TOKEN_SLASH, 1);
	}
	case '*': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_STAREQ, 2);
		}
		return scanner_create_token(state, TOKEN_STAR, 1);
	}
	case '%': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_PERCENTEQ, 2);
		}
		return scanner_create_token(state, TOKEN_PERCENT, 1);
	}
	case '=': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_EQEQ, 2);
		}
		return scanner_create_token(state, TOKEN_EQ, 1);
	}
	case '^': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_CARETEQ, 2);
		}
		return scanner_create_token(state, TOKEN_CARET, 1);
	}
	case '~': {
		return scanner_create_token(state, TOKEN_TILDE, 1);
	}
	case '&': {
		if (scanner_expect(state, '&') == '&') {
			return scanner_create_token(state, TOKEN_AMPAMP, 2);
		}
		goto scanner_start_of_next_token;
	}
	case ';': {
		return scanner_create_token(state, TOKEN_SEMICOLON, 1);
	}
	case ',': {
		return scanner_create_token(state, TOKEN_COMMA, 1);
	}
	case '!': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_BANGEQ, 2);
		}
		return scanner_create_token(state, TOKEN_BANG, 1);
	}
	case '(': {
		return scanner_create_token(state, TOKEN_OPENPAREN, 1);
	}
	case ')': {
		return scanner_create_token(state, TOKEN_CLOSEPAREN, 1);
	}
	case '[': {
		return scanner_create_token(state, TOKEN_OPENBRACK, 1);
	}
	case ']': {
		return scanner_create_token(state, TOKEN_CLOSEBRACK, 1);
	}
	case '{': {
		return scanner_create_token(state, TOKEN_OPENBRACE, 1);
	}
	case '}': {
		return scanner_create_token(state, TOKEN_CLOSEBRACE, 1); 
	}
	case '\t':
	case ' ':
	{
		goto scanner_start_of_next_token;
	}
	case '\n': {
		state->curLine++;
		goto scanner_start_of_next_token;
	}

	case '"': {
		return scanner_short_string(state, next);
	}
	case '\'': {
		if (scanner_advance(state) == '\\')
			scanner_advance(state);
		scanner_expect(state, '\'');
		return scanner_create_token(state, TOKEN_CHAR, 3);
	}
	default: {
		if (scanner_is_alpha(next)) {
			return scanner_identifier(state, next);
		}
		if (scanner_is_numeric(next)) {
			return scanner_number(state, next);
		}
		char* buf = xmalloc(128);
		snprintf(buf, 128, "Unrecognized character at line %d. Character: %c %d\n", state->curLine, next, next);
		scanner_error(state, buf, state->cur - 1, 1, state->curLine);
		free(buf);
		goto scanner_start_of_next_token;
	}
	}
}

void scanner_insert_token(ScannerState* state, ScannerToken token) {
	if (!state->tokBuf) {
		perror("Bad state.");
		exit(3);
	}
	if (state->numToks == state->toksCapacity) {
		xxrealloc(&state->tokBuf, state->toksCapacity * 2);
	}
	state->tokBuf[state->numToks++] = token;
}



ScannerState* scanner_scan_full_source(char* src, size_t bufSize, int isHeap) {
	ScannerState* state = scanner_create_state(src, bufSize, isHeap);
	ScannerToken last = {
		.type = TOKEN_START,
		.posInSrc = NULL,
		.numChars = 0,
	};
	scanner_insert_token(state, last);
	while (last.type != TOKEN_EOF) {
		last = scanner_next_token(state);
		scanner_insert_token(state, last);
	}
	return state;
}

void scanner_dump_print_tokens(ScannerState* state) {
	for (int i = 0; i < state->numToks; i++) {
		scanner_print_token(state->tokBuf[i]);
	}
}

void scanner_print_token(ScannerToken tok) {
	printf("SCANNER\t");
	switch (tok.type) {
	case TOKEN_START:{
		printf("Start of file\n");
		return;;
	}
	case TOKEN_EOF: {
		printf("End of file\n");
		return;
	}
	case TOKEN_ERROR: {
		printf("Line %d\tERROR\t%.*s\n", tok.line, (unsigned int)tok.numChars, tok.posInSrc);
		return;
	}
	default: {
		break;
	}
	}
	printf("Line %d\t%.*s\t%s\n", tok.line, (unsigned int)tok.numChars, tok.posInSrc, TOKEN_NAMES[tok.type]);
}

int scanner_is_alpha(char isThis) {
	return isThis >= 'A' && isThis <= 'z';
}
int scanner_is_numeric(char isThis) {
	return isThis >= '0' && isThis <= '9';
}

ScannerToken scanner_short_string(ScannerState* state, char first) {
	int len = 2;
	while ((first = scanner_advance(state)) != '"') {
		if (first == '\n' || scanner_is_at_end(state)) {
			char* buf = xmalloc(256 + len);
			snprintf(buf, 256+len, "Unterminated short string at line %d. String: %.*s", state->curLine, len, state->cur - len);
			scanner_error(state, buf, state->cur - len, len, state->curLine);
			return scanner_create_token(state, TOKEN_ERROR, len);
		}
		len++;
	}
	return scanner_create_token(state, TOKEN_STRING, len);
}

ScannerToken scanner_number(ScannerState* state, char current) {
	uint8_t wasFloat = 0;
	int len = 1;
	if (current == '.') {
		wasFloat = 1;
	}
	while (current = scanner_advance(state)) {
		if (!scanner_is_numeric(current) && current != '.') {
			if (*(scanner_backtrack(state)) == '.') {
				char* buf = xmalloc(128);
				snprintf(buf, 128, "Number ended with '.' at line %d.", state->curLine);
				return scanner_error_token(state, buf, state->cur, 1, state->curLine);
			}
			return scanner_create_token(state, TOKEN_INT + wasFloat, len);
		}
		wasFloat = wasFloat || current == '.';
		len++;
	}
	return scanner_create_token(state, TOKEN_ERROR, 0);
}

ScannerToken scanner_identifier(ScannerState* state, char current) {
	int len = 1;

	while (current = scanner_advance(state)) {
		if (!scanner_is_alpha(current) && !scanner_is_numeric(current) && current != '_') {
			break;
		}
		len++;
	}
	scanner_backtrack(state);
	return scanner_create_token(state, TOKEN_IDENTIFIER, len);
}

void scanner_short_comment(ScannerState* state) {
	do {} while (scanner_advance(state) != '\n' && !scanner_is_at_end(state));
	state->curLine++;
}
void scanner_long_comment(ScannerState* state) {
	int lineAtStart = state->curLine;
	char* start = state->cur - 2;
	while (!scanner_is_at_end(state)) {
		if (scanner_match(state, '*') && scanner_match(state, '/')) {
			return;
		}
		if (scanner_match(state, '\n')) {
			state->curLine++;
		}
		else {
			scanner_advance(state);
		}
	}
	char* buf = xmalloc(128);
	snprintf(buf, 128, "Unterminated long comment at line %d", lineAtStart);
	scanner_error(state, buf, start, 2, lineAtStart);
	free(buf);
}
