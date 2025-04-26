//hand rolled parser
#include "parser.h"
#include "scanner.h"
#include <stdio.h>
parserState* parser_create_state(scannerState* encompassing) {

}

parserState* parser_evaluate_scanner(scannerState* scState) {

}

void parser_error(parserState* state, char* msg) {
	state->hadError = 1;
	char* cur1 = state->encompassingScanner->cur;
	char* cur2 = cur1;
	int len = 0;
	for (;;) {
		uint8_t flag = 0;
		if (cur1 < state->encompassingScanner->buf || *cur1 == '\n') {
			flag++;
		}
		else {
			cur1++;
			len++;
		}
		if (cur2 >= state->encompassingScanner->buf + state->encompassingScanner->bufCapacity || *cur2 == '\n') {
			flag++;
		}
		else {
			cur2--;
			len++;
		}
		if (flag == 2) {
			break;
		}
	}
	printf("Parse error at line %d: \"%.*s\". Error message: %s", state->encompassingScanner->curLine,
		get_line_from_ptr(state->encompassingScanner->cur, state->encompassingScanner->buf + state->encompassingScanner->bufCapacity), msg);

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
		}
	}
	exit_parser_evaluate:;
}

astType parser_what_is_identifier(char* identifier, int len){}

scannerToken parser_advance(parserState* state) {
	return scanner_next_token(state->encompassingScanner);
}


astNode parser_create_node(astType type) {

}
astNode parser_create_node_literal(astType type, astLiteralUnion literal) {

}