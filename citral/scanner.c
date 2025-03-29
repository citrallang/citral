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

uint8_t scanner_is_at_end(scannerState* state) {
	if ((state->cur - state->buf) > (long long)state->bufCapacity) {
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

scannerState* scanner_create_state(char* buf, size_t bufSize) {
	scannerState* state = xmalloc(sizeof(scannerState));
	state->buf = buf;
	state->bufCapacity = bufSize;
	state->cur = buf;
	state->numToks = 0;
	state->toksCapacity = 16;
	state->tokBuf = xmalloc(sizeof(scannerToken) * 16);
	state->curLine = 1;
	return state;
}


//typedef enum TokenType {
//	TOKEN_AMPAMP, TOKEN_BARBAR,
//
//	TOKEN_EOF,
//} TokenType;

void scanner_error(scannerState* state) {

}

scannerToken scanner_next_token(scannerState* state) {
	scanner_start_of_next_token:
	char next = scanner_advance(state);
	if (next == 0) {
		return;
	}
	switch (next) {
	case '+': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_PLUSEQ, 2);
		}
		return scanner_create_token(state, TOKEN_PLUS, 1);
	}
	case '-': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_MINUSEQ, 2);
		}
		return scanner_create_token(state, TOKEN_MINUS, 1);
	}
	case '/': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_SLASHEQ, 2);
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
	case '^': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_CARETEQ, 2);
		}
		return scanner_create_token(state, TOKEN_CARET, 1);
	}
	case '=': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_EQEQ, 2);
		}
		return scanner_create_token(state, TOKEN_EQ, 1);
	}
	case '!': {
		if (scanner_match(state, '=')) {
			return scanner_create_token(state, TOKEN_BANGEQ, 2);
		}
		return scanner_create_token(state, TOKEN_BANG, 1);
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

	default: {
		printf("Unrecognized character at line %llu. Character: %c\n", state->curLine, *state->cur);
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
	while (last.type != TOKEN_EOF) {
		last = scanner_next_token(state);
		scanner_insert_token(state, last);
	}
}

void scanner_dump_print_tokens(scannerState* state) {
	for (int i = 0; i < state->numToks; i++) {
		scanner_print_token(state->tokBuf[i]);
	}
}

void scanner_print_token(scannerToken tok) {
	printf("Line %d\t%.*s\n", tok.line, (unsigned int)tok.numChars, tok.posInSrc);
}