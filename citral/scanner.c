#include "scanner.h"
#include "allocator.h"
#include <stdio.h>
char scanner_advance(scannerState* state) {
	if (scanner_is_at_end(state)) {
		return 0;
	}
	return *(state->cur++);
}

char scanner_peek(scannerState* state) {
	return *state->cur;
}

char scanner_match(scannerState* state, char matchAgainst) {
	char t = *state->cur;
	if (t == matchAgainst) {
		state->cur++;
		return t;
	}
	return 0;
}

char scanner_expect(scannerState* state, char expectThis) {
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

uint8_t scanner_is_at_end(scannerState* state) {
	if ((state->cur - state->buf) >= (long long)state->bufCapacity) {
		return 1;
	}
	return 0;
}

scannerToken scanner_create_token(scannerState* state, TokenType tokenType, size_t size) {
	scannerToken tok = {
		tokenType,
		state->cur - size,
		size,
		state->curLine
	};
	return tok;
}

scannerState* scanner_spawn_state() {
	scannerState* state = xmalloc(sizeof(scannerState));
	return state;
}

void scanner_free_state(scannerState* state) {
	free(state->tokBuf);
	free(state->buf);
	free(state);
}

scannerState* scanner_create_state(char* buf, size_t bufSize) {
	scannerState* state = xmalloc(sizeof(scannerState));
	state->buf = buf;
	state->bufCapacity = bufSize;
	state->cur = buf;
	state->toksCapacity = 16;
	state->tokBuf = xmalloc(sizeof(scannerToken) * 16);
	state->curLine = 1;
	return state;
}



void scanner_error(scannerState* state, char* msg, char* posInSrc, size_t numChars, int lineInSrc) {
	printf(msg);
	printf("\n");
	state->hadError = 1;
	scannerToken tok = {
		.line = lineInSrc,
		.numChars = numChars,
		.posInSrc = posInSrc,
		.type = TOKEN_ERROR
	};
	scanner_insert_token(state, tok);
}

scannerToken scanner_next_token(scannerState* state) {
scanner_start_of_next_token:
	char next = scanner_advance(state);
	switch (next) {
	case '\0': {
		scannerToken tok = scanner_create_token(state, TOKEN_EOF, 0);
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

void scanner_insert_token(scannerState* state, scannerToken token) {
	if (!state->tokBuf) {
		perror("Bad state.");
		exit(3);
	}
	if (state->numToks == state->toksCapacity) {
		xxrealloc(&state->tokBuf, state->toksCapacity * 2);
	}
	state->tokBuf[state->numToks++] = token;
}

scannerState* scanner_scan_source(char* src, size_t bufSize) {
	scannerState* state = scanner_create_state(src, bufSize);
	scannerToken last = {
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

void scanner_dump_print_tokens(scannerState* state) {
	for (int i = 0; i < state->numToks; i++) {
		scanner_print_token(state->tokBuf[i]);
	}
}

void scanner_print_token(scannerToken tok) {
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
	printf("Line %d\t%.*s\n", tok.line, (unsigned int)tok.numChars, tok.posInSrc);
}

int scanner_is_alpha(char isThis) {
	return isThis >= 'A' && isThis <= 'z';
}
int scanner_is_numeric(char isThis) {
	return isThis >= '0' && isThis <= '9';
}

scannerToken scanner_short_string(scannerState* state, char first) {
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

scannerToken scanner_number(scannerState* state, char current) {
	uint8_t wasFloat = 0;
	int len = 1;
	while (current = scanner_advance(state)) {
		
	}
	//perror("scanner_number is unimplemented");
	//exit(4);
}

scannerToken scanner_identifier(scannerState* state, char current) {
	int len = 2;

	while (current = scanner_advance(state)) {
		if (!scanner_is_alpha(current) && !scanner_is_numeric(current) && current != '_') {
			break;
		}
		len++;
	}
	return scanner_create_token(state, TOKEN_IDENTIFIER, len);
}

void scanner_short_comment(scannerState* state) {
	do {} while (scanner_advance(state) != '\n' && !scanner_is_at_end(state));
	state->curLine++;
}
void scanner_long_comment(scannerState* state) {
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
