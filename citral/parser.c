/*
first parser will be terrible intentionally
once ive finished reading a few more compiler textbooks i will come back and rewrite this
TODO
*/
#include "parser.h"
#include "scanner.h"
#include <stdio.h>
parserState* parser_create_state(scannerState* encompassing) {
	parserState* state = xmalloc(sizeof(parserState));
	state->encompassingScanner = encompassing;
	state->program = xmalloc(sizeof(astNode*) * 16);
	state->programCapacity = 16;
	return state;
}

parserState* parser_evaluate_scanner(scannerState* scState) {
	parserState* state = parser_create_state(scState);
	parser_evaluate(state);
	return state;
}

void parser_error(parserState* state, char* msg) {
	
}

void parser_evaluate(parserState* state) {
	scannerState* scanner = state->encompassingScanner;
	while (parser_scan_token(state)) {
#ifdef PARSER_DEBUG
		//print debug info
#endif
	}
}

scannerToken parser_advance(parserState* state) {
	return state->encompassingScanner->tokBuf[state->scannerPos++];
}

int parser_scan_token(parserState* state) {
	scannerToken tok;
	switch ((tok = parser_advance(state)).type) {
	case TOKEN_EOF: {
		return 0;
	}
	}
	return 0;
}

astNode parser_create_node(astType type) {
	astNode node = {
		.type = type
	};
	return node;
}
astNode parser_create_node_literal(astType type, astLiteralUnion literal) {
	astNode node = parser_create_node(type);
	node.literal = literal;
	return node;
}