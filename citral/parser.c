//hand rolled parser
#include "parser.h"
#include "scanner.h"
#include <stdio.h>
#include "hashmap.h"
#include "config.h"
ParserState* parser_create_state(ScannerState* encompassing) {
	ParserState* state = xmalloc(sizeof(ParserState));
	state->encompassingScanner = encompassing;
	state->program = xmalloc(sizeof(AstNode) * 16);
	state->programCapacity = 16;
	return state;
}

ParserState* parser_evaluate_scanner(ScannerState* scState) {

}

void parser_error(ParserState* state, char* msg) {
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

	fprintf(stderr, "Parse error at line %d: \"%.*s\". Error message: %s\n", state->encompassingScanner->curLine,
		distanceFromStart, start, msg);

}

void parser_evaluate(ParserState* state) {
	parser_initiate_keyword_list();
	ScannerToken tok;
	for (;;) {
		tok = parser_advance(state);
#ifdef SCANNER_DEBUG
		scanner_print_token(tok);
#endif
		switch (tok.type) {
		case TOKEN_EOF:
			goto exit_parser_evaluate;
		case TOKEN_IDENTIFIER: {
			int chars = tok.numChars;
			char* pos = tok.posInSrc;
			AstType typeOfIdentifier = parser_what_is_identifier(pos, chars);
			//if, for, break, return, variable, etc
			switch (typeOfIdentifier)
			{
			case AST_IDENTIFIER: {
				parser_error(state, "Expected statement, found identifier.");
				break;
			}
			case AST_FOR: {
				parser_start_for(state);
				break;
			}
			default:{}
			}
			break;
		}
		default: {
			parser_error(state, UNEXPECTED_TOKEN[tok.type]);
		}
		}
	}
	exit_parser_evaluate:;
}
#define MAX_KEYWORDS 256
HashNode keywords[MAX_KEYWORDS]; //we can increase this later if necessary
HashTable parser_reserved_keywords = {
	.nodes = &keywords,
	.maxNodes = MAX_KEYWORDS,
	.numNodes = 0,
	.usePrimitiveHasher = 0,
};

void parser_add_keyword_to_list(ParserKeyword word) {
	ParserKeyword* dynamic = xmalloc(sizeof(ParserKeyword));
	memcpy(dynamic, &word, sizeof(ParserKeyword));
	insert_pointers_to_hashtable(&parser_reserved_keywords, dynamic->literal, dynamic, dynamic->literalLen, sizeof(ParserKeyword)); //todo: improve this locality
}

void parser_add_str(char* literal, AstType type) {
	ParserKeyword word = {
		.literal = literal,
		.literalLen = strlen(literal),
		.whatAreYou = type
	};
	parser_add_keyword_to_list(word);
}

AstType parser_get_next_identifier(ParserState* state) {
	ScannerToken nextToken = parser_advance(state);
	if (nextToken.type != TOKEN_IDENTIFIER) {
		return AST_NOP;
	}
	return parser_what_is_identifier(nextToken.posInSrc, nextToken.numChars);
}

int started = 0;
void parser_initiate_keyword_list() {
	if (started) {
		return;
	}
	started = 1;
	parser_add_str("for", AST_FOR);
	parser_add_str("foreach", AST_FOREACH);
	parser_add_str("local", AST_LOCALDECL);
	parser_add_str("global", AST_GLOBALDECL);
	parser_add_str("import", AST_IMPORT);
	parser_add_str("while", AST_WHILE);
	parser_add_str("return", AST_RETURN);
	parser_add_str("switch", AST_SWITCH);
	parser_add_str("case", AST_CASE);
	parser_add_str("else", AST_ELSE);
	parser_add_str("elseif", AST_ELSEIF);
}

AstType parser_what_is_identifier(char* identifier, int len) {
	ParserKeyword* kw = hashtable_lookup_string_ptr(&parser_reserved_keywords, identifier, len);
	if (kw == NULL)
		return AST_IDENTIFIER;
	return kw->whatAreYou;
}

ScannerToken parser_advance(ParserState* state) {
	return scanner_next_token(state->encompassingScanner);
}
	

AstNode parser_create_node(AstType type) {
	AstNode node = {
		.left = NULL,
		.right = NULL,
		.literal = 0,
		.type = type
	};
	return node;
}
AstNode parser_create_node_literal(AstType type, AstLiteralUnion literal) {
	AstNode node = {
		.left = NULL,
		.right = NULL,
		.literal = literal,
		.type = type
	};
	return node;
}

void __parser_free_node(AstNode* node) {
	if (node == NULL) return;
	__parser_free_node(node->left);
	__parser_free_node(node->right);
	free(node);
}

void parser_cleanup(ParserState* state) {
	scanner_free_state(state->encompassingScanner);
	//free(state->program);
	for (int i = 0; i < state->programSize; i++) {
		__parser_free_node(state->program[i]);
	}
	free(state->program);
}

//0: failure
uint8_t parser_expect_tok(ParserState* state, TokenType tok) {
	if (parser_advance(state).type != tok) {
		return 0;
	}
	return 1;
}

void parser_start_for(ParserState* state) {
	if (parser_expect_tok(state, TOKEN_OPENPAREN)) {
		if (parser_get_next_identifier(state) == AST_LOCALDECL) {
			if (parser_get_next_identifier(state) == AST_IDENTIFIER) {
				if (parser_expect_tok(state, TOKEN_EQ)) {
					//TODO: PARSE EXPRESSION HERE
				}
				else {
					parser_error(state, "Expected '=' after 'for (local IDENTIFIER '");
				}
			}
			else {
				parser_error(state, "Expected identifier after 'for (local '");
			}
		}
		else 
			if (!parser_expect_tok(state, TOKEN_SEMICOLON))
				parser_error(state, "Expected 'local' or ';' after 'for ('");

	}
	else {
		parser_error(state, "Expected '(' after 'for'");
	};
}