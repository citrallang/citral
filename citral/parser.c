/*
first parser will be terrible intentionally
once ive finished reading a few more compiler textbooks i will come back and rewrite this
TODO
*/
#include "parser.h"
#include "scanner.h"

parserState* parser_create_state(scannerState* encompassing) {
	parserState* state = xmalloc(sizeof(parserState));
	state->encompassingScanner = encompassing;
	state->program = xmalloc(sizeof(astNode*) * 16);
	state->programCapacity = 16;
	state->programSize = 0;
	return state;
}

parserState* parser_evaluate_scanner(scannerState* scState) {
	parserState* state = parser_create_state(scState);
	parser_evaluate(state);
	return state;
}

void parser_evaluate(parserState* state) {
	
}