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

void parser_warn(ParserState* state, char* msg) {
	if (state->hadError) {
		return; //later we can start resetting at statement borders or whatever
	}
	char* start = state->encompassingScanner->cur - 1;
	int distanceFromStart = 1;
	for (;;) {
		start--;
		if (start < state->encompassingScanner->buf || *start == '\n') {
			start++;
			break;
		}
		distanceFromStart++;
	}
	char* end = state->encompassingScanner->cur - 1;
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

	fprintf(stdout, "Warning at line %d: \"%.*s\". Message: %s\n", state->encompassingScanner->curLine,
		distanceFromStart, start, msg);
}

void parser_evaluate(ParserState* state) {
	parser_initialize();
	ScannerToken tok;
	parser_decl_pass(state);
#ifdef PARSER_DEBUG
	parser_print_declarations();
#endif
//	ScannerToken tok;
//	for (;;) {
//		tok = parser_advance(state);
//#ifdef SCANNER_DEBUG
//		scanner_print_token(tok);
//#endif
//		switch (tok.type) {
//		case TOKEN_EOF:
//			goto exit_parser_evaluate;
//		case TOKEN_IDENTIFIER: {
//			int chars = tok.numChars;
//			char* pos = tok.posInSrc;
//			AstType typeOfIdentifier = parser_what_is_identifier(pos, chars);
//			//if, for, break, return, variable, etc
//			switch (typeOfIdentifier)
//			{
//			case AST_IDENTIFIER: {
//				parser_error(state, "Expected statement, found identifier.");
//				break;
//			}
//			case AST_FOR: {
//				parser_start_for(state);
//				break;
//			}
//			default:{}
//			}
//			break;
//		}
//		default: {
//			parser_error(state, UNEXPECTED_TOKEN[tok.type]);
//		}
//		}
//	}
//	exit_parser_evaluate:;
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

void parser_add_full_type(ParserType type) {
	ParserType* dynamic = xmalloc(sizeof(ParserType));
	memcpy(dynamic, &type, sizeof(ParserType));
	insert_pointers_to_hashtable(&parserTypeTable, dynamic->name, dynamic, dynamic->nameLen, sizeof(ParserType));
}

void parser_add_type(char* literal, ParserTypesE etype) {
	ParserType type = {
		.type = etype,
		.name = literal,
		.nameLen = strlen(literal)
	};
	parser_add_full_type(type);
}

AstType parser_get_next_identifier(ParserState* state) {
	ScannerToken nextToken = parser_advance(state);
	if (nextToken.type != TOKEN_IDENTIFIER) {
		return AST_NOP;
	}
	return parser_what_is_identifier(nextToken.posInSrc, nextToken.numChars);
}

int started = 0;
void parser_initialize() {
	if (started) {
		return;
	}
	printf("initializing\n");
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

	parserTypeTable.nodes = xmalloc(sizeof(HashNode) * 16);
	parserTypeTable.maxNodes = 16;
	parserTypeTable.numNodes = 0;
	

	parser_add_type("i8", PTYPE_I8);
	parser_add_type("i16", PTYPE_I16);
	parser_add_type("i32", PTYPE_I32);
	parser_add_type("i64", PTYPE_I64);
	parser_add_type("float", PTYPE_FLOAT);
	parser_add_type("f32", PTYPE_FLOAT);
	parser_add_type("double", PTYPE_DOUBLE);
	parser_add_type("f64", PTYPE_DOUBLE);
	parser_add_type("u8", PTYPE_U8);
	parser_add_type("u16", PTYPE_U16);
	parser_add_type("u32", PTYPE_U32);
	parser_add_type("u64", PTYPE_U64);
	parser_add_type("char", PTYPE_I8);
	parser_add_type("uchar", PTYPE_U8);
	parser_add_type("void", PTYPE_VOID);

	parserFunctionTable.nodes = xmalloc(sizeof(HashNode) * 8);
	parserFunctionTable.maxNodes = 8;
	parserFunctionTable.numNodes = 0;
}

AstType parser_what_is_identifier(char* identifier, int len) {
	ParserKeyword* kw = hashtable_lookup_string_ptr(&parser_reserved_keywords, identifier, len);
	if (kw == NULL)
		return AST_IDENTIFIER;
	return kw->whatAreYou;
}

ParserType parser_what_is_type(char* typeName, int len) {
	ParserType* type = hashtable_lookup_string_ptr(&parserTypeTable, typeName, len);
	if (type == NULL) {
		ParserType ret = {
			.type = PTYPE_NOTHING,
		};
		return ret;
	}
	return *type;
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

AstNode* parser_expression(ParserState* state) {
	 
}

void parser_decl_pass(ParserState* state) {
	for (;;) {
		ScannerToken tok = parser_advance(state);
		switch (tok.type) {
		case TOKEN_IDENTIFIER: {
			AstType atype = parser_what_is_identifier(tok.posInSrc, tok.numChars);
			ParserType ptype = parser_what_is_type(tok.posInSrc, tok.numChars);
			switch (ptype.type) {
			case PTYPE_NOTHING: {
				if (!parser_is_legitimate_identifier(state, tok)) {
					parser_error(state, "Illegitimate identifier.");
					goto pdeclpasscontinue;
				}
				ScannerToken type = {
					.type = TOKEN_NOTHING
				};
				if (parser_expect_tok(state, TOKEN_OPENPAREN))
					parser_decl(state, type, tok);
				else {
					parser_error(state, "No open parentheses to start function.");
					goto pdeclpasscontinue;
				}
				break;
			}
			default: {
				ScannerToken name = parser_advance(state);
				if (name.type != TOKEN_IDENTIFIER) {
					parser_error(state, UNEXPECTED_TOKEN[name.type]);
					goto pdeclpasscontinue;
				}
				if (!parser_is_legitimate_identifier(state, name)) {
					parser_error(state, "Illegitimate identifier.");
				}
				if (parser_expect_tok(state, TOKEN_OPENPAREN))
					parser_decl(state, tok, name);
				else {
					parser_error(state, "No open parentheses to start function.");
					goto pdeclpasscontinue;
				}
			}
			}
			break;
		}
		case TOKEN_EOF: {
			return;
		}
		}
	pdeclpasscontinue:;
	}
}

//precon: The "import" token has been consumed
void parser_import(ParserState* state) {
	if (parser_expect_tok(state, TOKEN_STRING)) {
		//todo: parse the filename and import it
	}
	else {
		parser_error(state, "Expected \"<FILE>\" near 'import'");
	}
}

//0: illegitimate
uint8_t parser_is_legitimate_identifier(ParserState* state, ScannerToken tok) {
	AstType type = parser_what_is_identifier(tok.posInSrc, tok.numChars);
	if (type != AST_IDENTIFIER) {
		return 0;
	}
	ParserTypesE etype = parser_what_is_type(tok.posInSrc, tok.numChars).type;
	if (etype != PTYPE_NOTHING) {
		return 0;
	}
	return 1;
}

void parser_push_argument_onto_function(ParserFunctionDeclaration* func, ParserType type) {
	if (++func->nargs > func->maxArgs) {
		func->args = xrealloc(func->args, sizeof(ParserType) * func->maxArgs * 2);
	}
	func->args[func->nargs - 1] = type;
}

//precon: the type, name, and opening parentheses have been consumed. if type.type == TOKEN_NOTHING, the function is assumed to have a "void" return type (which can change if a clear return type is established)
void parser_decl(ParserState* state, ScannerToken tokType, ScannerToken tokName) {
	ParserType type;
	if (tokType.type == TOKEN_NOTHING) {
		type.type = PTYPE_NOTHING;
	}
	else {
		type = parser_what_is_type(tokType.posInSrc, tokType.numChars);
		if (type.type == TOKEN_NOTHING) {
			parser_error(state, "Given return type is not actually a type");
			return;
		}
	}
	AstType ident = parser_what_is_identifier(tokName.posInSrc, tokName.numChars);
	if (ident != AST_IDENTIFIER) {
		parser_error(state, "Illegal function name");
		return;
	}
	ParserFunctionDeclaration decl = {
		.identifier = tokName.posInSrc,
		.identLen = tokName.numChars,
		.retType = type,
		.args = xmalloc(sizeof(ParserType) * 4),
		.maxArgs = 4,
	};
	for (;;) {
		ScannerToken tok = parser_advance(state);
		switch (tok.type) {
		case TOKEN_CLOSEPAREN: {
			goto parser_decl_finished;
		}
		case TOKEN_IDENTIFIER: {
			ParserType type = parser_what_is_type(tok.posInSrc, tok.numChars);
			
			if (type.type != TOKEN_NOTHING) {
				ScannerToken tok = parser_advance(state);
				if (parser_is_legitimate_identifier(state, tok)) {
					ScannerToken commaOrClose = parser_advance(state);
					if (commaOrClose.type == TOKEN_CLOSEPAREN) {
						parser_push_argument_onto_function(&decl, type);
						goto parser_decl_finished;
					}
					if (commaOrClose.type != TOKEN_COMMA) {
						parser_error(state, "Expected ',' after function argument.");
						return;
					}
				}
				else {
					parser_error(state, "Illegitimate identifier as argument name. Ensure not violating any reserved types/keywords");
					return;
				}
			}
			parser_push_argument_onto_function(&decl, type);
			break;
		}
		default: {
			parser_error(state, UNEXPECTED_TOKEN[tok.type]);
			return;
		}
		}
	}
parser_decl_finished:
	parser_declare_function(decl);
	
}

void parser_declare_function(ParserFunctionDeclaration func) {
	ParserFunctionDeclaration* dynamic = xmalloc(sizeof(ParserFunctionDeclaration));
	memcpy(dynamic, &func, sizeof(ParserFunctionDeclaration));
	insert_pointers_to_hashtable(&parserFunctionTable, dynamic->identifier, dynamic, dynamic->identLen, sizeof(ParserFunctionDeclaration));
}

void parser_print_declarations() {
	for (int i = 0; i < parserFunctionTable.maxNodes; i++) {
		ParserFunctionDeclaration* node = parserFunctionTable.nodes[i].val.asPtr;
		if (node != NULL) {
			if (node->nargs == 1) {
				printf("Function %.*s has argument %.*s. ", node->identLen, node->identifier, node->args[0].nameLen, node->args[0].name);
			}
			else if (node->nargs > 1) {
				printf("Function %.*s has arguments ", node->identLen, node->identifier);
				for (int k = 0; k < node->nargs - 1; k++) {
					if (node->args[k].name == NULL) {
						printf("inferred, ");
					}
					else {
						printf("%.*s ", node->args[k].nameLen, node->args[k].name);
					}
				}
				printf("and %.*s. ", node->args[node->nargs - 1].nameLen, node->args[node->nargs - 1].name);
			}
			else
				printf("Function %.*s has no arguments. ", node->identLen, node->identifier);
			if (node->retType.type != PTYPE_NOTHING) {
				printf("It returns a(n) %.*s.", node->retType.nameLen, node->retType.name);
			}
			else {
				printf("It returns an inferred value.");
			}
			printf("\n");
		}
	}
}