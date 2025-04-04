/*
first parser will be terrible intentionally
once ive finished reading a few more compiler textbooks i will come back and rewrite this
TODO
*/
#include "parser_old.h"
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
	while (1) {
		if (parser_scan_token(state).type == AST_EOF) {
			return;
		}
#ifdef PARSER_DEBUG
		//print debug info
#endif
	}
}

scannerToken parser_advance(parserState* state) {
	return state->encompassingScanner->tokBuf[state->scannerPos++];
}

//typedef enum TokenType {
//	TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT, TOKEN_CARET, TOKEN_TILDE,
//	TOKEN_PLUSEQ, TOKEN_MINUSEQ, TOKEN_STAREQ, TOKEN_SLASHEQ, TOKEN_PERCENTEQ, TOKEN_CARETEQ, TOKEN_PLUSPLUS, TOKEN_MINUSMINUS,
//
//	TOKEN_EQ, TOKEN_EQEQ, TOKEN_BANG, TOKEN_BANGEQ, TOKEN_AMPAMP, TOKEN_BARBAR,
//
//	TOKEN_OPENPAREN, TOKEN_CLOSEPAREN, TOKEN_OPENBRACK, TOKEN_CLOSEBRACK, TOKEN_OPENBRACE, TOKEN_CLOSEBRACE,
//
//	TOKEN_STRING, TOKEN_INT, TOKEN_FLOAT, TOKEN_IDENTIFIER, TOKEN_CHAR,
//
//	TOKEN_EOF, TOKEN_START, TOKEN_ERROR,
//} TokenType;
astNode parser_scan_token(parserState* state) {
	scannerToken tok;
	switch ((tok = parser_advance(state)).type) {
	case TOKEN_EOF: {
		return parser_create_node(AST_EOF);
	}
	case TOKEN_START: {
		return parser_create_node(AST_NOP);
	}
	case TOKEN_PLUS: {
		astNode node = parser_create_node(AST_PLUS);
		astNode left = parser_scan_token(state);
		
	}
	}
	return parser_create_node(AST_NOP);
}

uint8_t parser_ensure_expr(astNode node) {

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