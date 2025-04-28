//hand rolled parser
#include "parser.h"
#include "scanner.h"
#include <stdio.h>
#include "hashmap.h"
parserState* parser_create_state(scannerState* encompassing) {
	parserState* state = xmalloc(sizeof(parserState));
	state->encompassingScanner = encompassing;
	state->program = xmalloc(sizeof(astNode) * 16);
	state->programCapacity = 16;
	return state;
}

parserState* parser_evaluate_scanner(scannerState* scState) {

}

void parser_error(parserState* state, char* msg) {
	if (state->hadError) {
		return; //later we can start resetting at statement borders or whatever
	}
	state->hadError = 1;
	char* start = state->encompassingScanner->cur-1;
	int distanceFromStart = 1;
	for (;;) {
		start--;
		if (start < state->encompassingScanner->buf || *start == '\n') {
			start++;
			break;
		}
		distanceFromStart++;
	}
	char* end = state->encompassingScanner->cur-1;
	for (;;) {
		end++;
		if (end > state->encompassingScanner->buf + state->encompassingScanner->bufCapacity
			|| *end == '\n') 
		{
			end--;
			break;
		}
		distanceFromStart++;
	}

	printf("Parse error at line %d: \"%.*s\". Error message: %s\n", state->encompassingScanner->curLine,
		distanceFromStart, start, msg);

}

void parser_evaluate(parserState* state) {
	scannerToken tok;
	for (;;) {
		tok = parser_advance(state);
		switch (tok.type) {
		case TOKEN_EOF:
			goto exit_parser_evaluate;
		case TOKEN_IDENTIFIER: {
			int chars = tok.numChars;
			char* pos = tok.posInSrc;
			astType typeOfIdentifier = parser_what_is_identifier(pos, chars);
			//if, for, break, return, variable, etc
			switch (typeOfIdentifier)
			{
			case AST_IDENTIFIER: {
				parser_error(state, "Expected statement, found identifier.");
				state->hadError = 1;
			}
			}
		}
		default: {
			parser_error(state, UNEXPECTED_TOKEN[tok.type]);
		}
		}
	}
	exit_parser_evaluate:;
}





astType parser_what_is_identifier(char* identifier, int len) {

}

scannerToken parser_advance(parserState* state) {
	return scanner_next_token(state->encompassingScanner);
}
	

astNode parser_create_node(astType type) {
	astNode node = {
		.left = NULL,
		.right = NULL,
		.literal = 0,
		.type = type
	};
	return node;
}
astNode parser_create_node_literal(astType type, astLiteralUnion literal) {
	astNode node = {
		.left = NULL,
		.right = NULL,
		.literal = literal,
		.type = type
	};
	return node;
}