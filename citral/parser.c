/*
first parser will be terrible intentionally
once ive finished reading a few more compiler textbooks i will come back and rewrite this
TODO
*/
#include "parser.h"
#include "scanner.h"
#include <stdio.h>
parserState* parser_create_state(scannerState* encompassing) {

}

parserState* parser_evaluate_scanner(scannerState* scState) {

}

void parser_error(parserState* state, char* msg) {
	
}

void parser_evaluate(parserState* state) {
	
}

scannerToken parser_advance(parserState* state) {

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

}


astNode parser_create_node(astType type) {

}
astNode parser_create_node_literal(astType type, astLiteralUnion literal) {

}