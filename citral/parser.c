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

static char* otherErrMsg;
void parser_print_other_err(void) {
	if (otherErrMsg)
		fprintf(stderr, "%s", otherErrMsg);
}

void parser_add_error_message(char* msg) {
	otherErrMsg = msg;
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

	fprintf(stderr, "Parse error at line %d: \"%.*s\". Error message: ", state->encompassingScanner->curLine,
		distanceFromStart, start);
	parser_print_other_err();
	fprintf(stderr, "%s\n", msg);

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
	parser_initialize(state);
	parser_decl_pass(state);
#ifdef PARSER_DEBUG
	parser_print_declarations();
#endif
	//todo: parse imports



	//todo: parse function definitions
}


void parser_add_keyword_to_list(ParserState* state, ParserKeyword word) {
	ParserKeyword* dynamic = xmalloc(sizeof(ParserKeyword));
	memcpy(dynamic, &word, sizeof(ParserKeyword));
	insert_pointers_to_hashtable(state->keywords, dynamic->literal, dynamic, dynamic->literalLen, sizeof(ParserKeyword)); //todo: improve this locality
}

void parser_add_str(ParserState* state, char* literal, AstType type) {
	ParserKeyword word = {
		.literal = literal,
		.literalLen = (unsigned int)strlen(literal),
		.whatAreYou = type
	};
	parser_add_keyword_to_list(state, word);
}

void parser_add_full_type(ParserState* state, ParserType type) {
	ParserType* dynamic = xmalloc(sizeof(ParserType));
	memcpy(dynamic, &type, sizeof(ParserType));
	insert_pointers_to_hashtable(state->types, dynamic->name, dynamic, dynamic->nameLen, sizeof(ParserType));
}

void parser_add_type(ParserState* state, char* literal, ParserTypesE etype) {
	ParserType type = {
		.type = etype,
		.name = literal,
		.nameLen = (unsigned int)strlen(literal)
	};
	parser_add_full_type(state, type);
}

AstType parser_get_next_identifier(ParserState* state) {
	ScannerToken nextToken = parser_advance(state);
	if (nextToken.type != TOKEN_IDENTIFIER) {
		return AST_NOP;
	}
	return parser_what_is_identifier(state, nextToken.posInSrc, nextToken.numChars);
}


void parser_initialize(ParserState* state) {
	state->precedenceTable[AST_UNARY_MINUS] = 3;
	state->precedenceTable[AST_INTEGER] = 1;
	state->precedenceTable[AST_FLOAT] = 1;
	state->precedenceTable[AST_STRING] = 1;
	state->precedenceTable[AST_PLUS] = 6;
	state->precedenceTable[AST_MINUS] = 6;
	state->precedenceTable[AST_TIMES] = 7;
	state->precedenceTable[AST_DIV] = 7;
	state->precedenceTable[AST_MOD] = 7;

	state->functions = xmalloc(sizeof(HashTable));
	state->keywords = xmalloc(sizeof(HashTable));
	state->types = xmalloc(sizeof(HashTable));

	state->keywords->nodes = xmalloc(sizeof(HashNode) * 32);
	state->keywords->maxNodes = 32;

	parser_add_str(state, "for", AST_FOR);
	parser_add_str(state, "foreach", AST_FOREACH);
	parser_add_str(state, "local", AST_LOCALDECL);
	parser_add_str(state, "global", AST_GLOBALDECL);
	parser_add_str(state, "import", AST_IMPORT);
	parser_add_str(state, "while", AST_WHILE);
	parser_add_str(state, "return", AST_RETURN);
	parser_add_str(state, "switch", AST_SWITCH);
	parser_add_str(state, "case", AST_CASE);
	parser_add_str(state, "else", AST_ELSE);
	parser_add_str(state, "elseif", AST_ELSEIF);

	state->types->nodes = xmalloc(sizeof(HashNode) * 32);
	state->types->maxNodes = 32;
	state->types->numNodes = 0;
	

	parser_add_type(state, "i8", PTYPE_I8);
	parser_add_type(state, "i16", PTYPE_I16);
	parser_add_type(state, "i32", PTYPE_I32);
	parser_add_type(state, "i64", PTYPE_I64);
	parser_add_type(state, "float", PTYPE_FLOAT);
	parser_add_type(state, "f32", PTYPE_FLOAT);
	parser_add_type(state, "double", PTYPE_DOUBLE);
	parser_add_type(state, "f64", PTYPE_DOUBLE);
	parser_add_type(state, "u8", PTYPE_U8);
	parser_add_type(state, "u16", PTYPE_U16);
	parser_add_type(state, "u32", PTYPE_U32);
	parser_add_type(state, "u64", PTYPE_U64);
	parser_add_type(state, "char", PTYPE_I8);
	parser_add_type(state, "uchar", PTYPE_U8);
	parser_add_type(state, "void", PTYPE_VOID);

	state->functions->nodes = xmalloc(sizeof(HashNode) * 32);
	state->functions->maxNodes = 32;
	state->functions->numNodes = 0;
}

AstType parser_what_is_identifier(ParserState* state, char* identifier, size_t len) {
	ParserKeyword* kw = hashtable_lookup_string_ptr(state->keywords, identifier, (unsigned int)len);
	if (kw == NULL)
		return AST_IDENTIFIER;
	return kw->whatAreYou;
}

ParserType parser_what_is_type(ParserState* state, char* typeName, size_t len) {
	ParserType* type = hashtable_lookup_string_ptr(state->types, typeName, (unsigned int)len);
	if (type == NULL) {
		ParserType ret = {
			.type = PTYPE_NOTHING,
		};
		return ret;
	}
	return *type;
}

ScannerToken parser_advance(ParserState* state) {
#ifdef SCANNER_DEBUG
	ScannerToken next = scanner_next_token(state->encompassingScanner);
	scanner_print_token(next);
	return next;
#else
	return scanner_next_token(state->encompassingScanner);
#endif
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

static void __parser_free_node(AstNode* node) {
	if (node == NULL) return;
	__parser_free_node(node->left);
	__parser_free_node(node->right);
	free(node);
}

static void NOTHING(void* NOTHING) {}

void parser_cleanup(ParserState* state) {
	scanner_free_state(state->encompassingScanner);
	//free(state->program);
	for (int i = 0; i < state->programSize; i++) {
		__parser_free_node(state->program[i]);
	}
	for (unsigned int i = 0; i < state->functions->maxNodes; i++) {
		if (state->functions->nodes[i].isFull) { 
			ParserFunctionDeclaration* ptr = (ParserFunctionDeclaration*)state->functions->nodes[i].val.asPtr;
			NOTHING(ptr->args); //we get an uninitialized memory warning if i dont do this. genuinely. fuck you MSVC.
			free(ptr->args);
			free(state->functions->nodes[i].val.asPtr);
			HashNode empty = { .hash = 0 };
			state->functions->nodes[i] = empty;
		}
	}
	free(state->functions->nodes);
	free(state->functions);

	for (unsigned int i = 0; i < state->types->maxNodes; i++) {
		if (state->types->nodes[i].isFull) {
			free(state->types->nodes[i].val.asPtr);
			HashNode empty = { .hash = 0 };
			state->types->nodes[i] = empty;
		}
	}
	free(state->types->nodes);
	free(state->types);
	for (unsigned int i = 0; i < state->keywords->maxNodes; i++) {
		if (state->keywords->nodes[i].isFull) {
			free(state->keywords->nodes[i].val.asPtr);
			HashNode empty = {.hash = 0};
			state->keywords->nodes[i] = empty;
		}
	}
	
	free(state->keywords->nodes);
	free(state->keywords);
	free(state->program);
	free(state);
}

//0: failure
uint8_t parser_expect_tok(ParserState* state, TokenType tok) {
	if (parser_advance(state).type != tok) {
		return 0;
	}
	return 1;
}

void parser_start_for(ParserState* state) {
	//if (parser_expect_tok(state, TOKEN_OPENPAREN)) {
	//	if (parser_get_next_identifier(state) == AST_LOCALDECL) {
	//		if (parser_get_next_identifier(state) == AST_IDENTIFIER) {
	//			if (parser_expect_tok(state, TOKEN_EQ)) {
	//				
	//			}
	//			else {
	//				parser_error(state, "Expected '=' after 'for (local IDENTIFIER '");
	//			}
	//		}
	//		else {
	//			parser_error(state, "Expected identifier after 'for (local '");
	//		}
	//	}
	//	else 
	//		if (!parser_expect_tok(state, TOKEN_SEMICOLON))
	//			parser_error(state, "Expected 'local' or ';' after 'for ('");

	//}
	//else {
	//	parser_error(state, "Expected '(' after 'for'");
	//};
}
#define newAstNode() (AstNode*)xmalloc(sizeof(AstNode))
//recursively call, only accept operations with lower precedence shoutout pratt my goat
AstNode* parser_begin_expression(ParserState* state, int maxPrecedence) {
	AstNode* top = newAstNode();
	AstNode* final = NULL;
	ScannerToken nextTok = parser_advance(state);
	AstType type = scannerTokenToAstType[nextTok.type];
	//AstType type = AST_NOP;
	//switch (nextTok.type) {
	//case TOKEN_INT: 
	//{
	//	type = AST_INTEGER;
	//	break;
	//}
	//case TOKEN_FLOAT: 
	//{
	//	type = AST_FLOAT;
	//	break;
	//}
	//case TOKEN_IDENTIFIER: {
	//	ParserIdentifierInfo info = parser_get_info(state, nextTok);
	//	type = info.atype;
	//	break;
	//}
	//case TOKEN_MINUS: {
	//	type = AST_UNARY_MINUS;
	//	break;
	//}
	//case TOKEN_BANG: {
	//	type = AST_UNARY_NOT;
	//	break;
	//}
	//case TOKEN_OPENPAREN: {
	//	type = AST_GROUPING;
	//	break;
	//}
	//}

	int precedence = state->precedenceTable[type];
	if (precedence == 0) {
		parser_add_error_message(" in expression.");
		parser_error(state, UNEXPECTED_TOKEN[nextTok.type]);
		return NULL;
	}
	if (precedence > maxPrecedence) {
		return NULL;
	}
	top->type = type;
	switch (type) {
	case AST_UNARY_MINUS: {
		top->left = parser_begin_expression(state, precedence);
		final = parser_inner_expression(state, maxPrecedence, top);
		if (final == 0)
			final = top;
	}
	case AST_FLOAT:
	case AST_INTEGER: {
		AstNode* number = parser_get_number(state, nextTok);
		final = parser_inner_expression(state, maxPrecedence, number);
	}
	}
	
	return final;
}

AstNode* parser_inner_expression(ParserState* state, int maxPrecedence, AstNode* left) {
	ScannerToken midToken = parser_advance(state);
	int precedence = state->precedenceTable[midToken.type];
	if (precedence > maxPrecedence) {
		return NULL;
	}
	if (precedence == 0) {
		parser_add_error_message(" in expression.");
		parser_error(state, UNEXPECTED_TOKEN[midToken.type]);
		return NULL;
	}
	AstNode* middle = newAstNode();
	switch (midToken.type) {
	case TOKEN_MINUS:
	case TOKEN_PLUS:
	case TOKEN_STAR:
	case TOKEN_SLASH:
	case TOKEN_PERCENT: {
		middle->type = scannerTokenToAstType[midToken.type];
		middle->left = left;
		middle->right = parser_begin_expression(state, precedence);
		if (middle->right == NULL) {
			parser_error(state, "Unfinished expression");
			return NULL;
		}
	}
	}
	if (middle == NULL) {
		parser_error(state, "Irregular expression");
		return NULL;
	}
}


int max_depth(AstNode* node) {
	if (node == NULL) {
		return 0;
	}
	int first = max_depth(node->right);
	int second = max_depth(node->left);
	return max(first, second) + 1;
}

void parser_print_ast(ParserState* state, int indentation, AstNode* top) {
	for (int i = 0; i < indentation; i++) {
		printf("\t");
	}
	printf("%s", astTypeToString[top->type]);
	switch (top->type) {
	case AST_INTEGER: {
		printf("%lld\n", top->literal.asI64);
	}
	case AST_FLOAT:
	{
		printf("%lf", top->literal.asDouble);
	}
	}
}

AstNode* parser_get_number(ParserState* state, ScannerToken num) {
	AstNode* node = newAstNode();
	switch (num.type) {
	case TOKEN_INT: {
		node->type = AST_INTEGER;
		node->literal.asI64 = strtol(num.posInSrc, num.posInSrc + num.numChars, 10);
		break;
	}
	case TOKEN_FLOAT: {
		node->type = AST_FLOAT;
		node->literal.asDouble = strtod(num.posInSrc, num.posInSrc + num.numChars, 10);
		break;
	}
	default: {
		parser_error(state, "Number expected, found nothing");
	}
	}
	return node;
}

//todo: parse things like classes and type declarations here
void parser_decl_pass(ParserState* state) {
	uint8_t scopeLevel = 0;
	for (;;) {
		ScannerToken tok = parser_advance(state);
		switch (tok.type) {
		case TOKEN_OPENBRACE: {
			scopeLevel++;
			break;
		}
		case TOKEN_CLOSEBRACE: {
			scopeLevel--;
			break;
		}
		case TOKEN_IDENTIFIER: {
			AstType atype = parser_what_is_identifier(state, tok.posInSrc, tok.numChars);
			ParserType ptype = parser_what_is_type(state, tok.posInSrc, tok.numChars);
			switch (atype) {
			case AST_GLOBALDECL: {
				parser_global(state);
				break;
			}
			case AST_CLASS: {
				//todo: parse classes
				break;
			}
			case AST_IMPORT: {
				//todo: parse imports
				break;
			}
			case AST_IDENTIFIER: {
				break;
			}
			default: {
				parser_error(state, "Unexpected token in top scope.");
			}
			}
			switch (ptype.type) {
			case PTYPE_NOTHING: {
				if (!parser_assert_legitimate_identifier(state, tok)) {
					parser_error(state, "Illegitimate identifier.");
					goto pdeclpasscontinue;
				}
				ScannerToken type = {
					.type = TOKEN_NOTHING
				};
				if (parser_expect_tok(state, TOKEN_OPENPAREN)) {
					if (scopeLevel) {
						goto pdeclpasscontinue;
					}
					parser_decl(state, type, tok);
				}
				else {
					parser_error(state, "No open parentheses to start function.");
					goto pdeclpasscontinue;
				}
				break;
			}
			default: {
				ScannerToken name = parser_advance(state);
				if (name.type != TOKEN_IDENTIFIER) {
					parser_error(state, (char*)UNEXPECTED_TOKEN[name.type]);
					goto pdeclpasscontinue;
				}
				if (!parser_assert_legitimate_identifier(state, name)) {
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
			if (scopeLevel) {
				parser_error(state, "Unexpected EOF. Imbalanced {}.");
			}
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
uint8_t parser_assert_legitimate_identifier(ParserState* state, ScannerToken tok) {
	AstType type = parser_what_is_identifier(state, tok.posInSrc, tok.numChars);
	if (type != AST_IDENTIFIER) {
		return 0;
	}
	ParserTypesE etype = parser_what_is_type(state, tok.posInSrc, tok.numChars).type;
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

uint8_t parser_does_function_exist(ParserFunctionDeclaration* func) {
	//TODO
	return 0;
}

//precon: the type, name, and opening parentheses have been consumed. if type.type == TOKEN_NOTHING, the function is assumed to have a "void" return type (which can change if a clear return type is established)
void parser_decl(ParserState* state, ScannerToken tokType, ScannerToken tokName) {
	ParserType type = {
		.type = PTYPE_NOTHING
	};
	if (tokType.type != TOKEN_NOTHING) {
		type = parser_what_is_type(state, tokType.posInSrc, tokType.numChars);
		if (type.type == PTYPE_NOTHING) {
			parser_error(state, "Given return type is not actually a type");
			return;
		}
	}
	AstType ident = parser_what_is_identifier(state, tokName.posInSrc, tokName.numChars);
	if (ident != AST_IDENTIFIER) {
		parser_error(state, "Illegal function name");
		return;
	}
	ParserFunctionDeclaration decl = {
		.identifier = tokName.posInSrc,
		.identLen = (unsigned int)tokName.numChars,
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
			ParserType type = parser_what_is_type(state, tok.posInSrc, tok.numChars);
			
			if (type.type != PTYPE_NOTHING) {
				ScannerToken tok = parser_advance(state);
				if (parser_assert_legitimate_identifier(state, tok)) {
					ScannerToken commaOrClose = parser_advance(state);
					if (commaOrClose.type == TOKEN_CLOSEPAREN) {
						parser_push_argument_onto_function(&decl, type);
						goto parser_decl_finished;
					}
					if (commaOrClose.type != TOKEN_COMMA) {
						parser_error(state, "Expected ',' or ')' after function argument.");
						return;
					}
				}
				else {
					parser_error(state, "Illegitimate identifier as argument name. Ensure not violating any reserved types/keywords");
					return;
				}
			}
			else {
				ScannerToken commaOrClose = parser_advance(state);
				if (commaOrClose.type == TOKEN_CLOSEPAREN) {
					parser_push_argument_onto_function(&decl, type);
					goto parser_decl_finished;
				}
				if (commaOrClose.type != TOKEN_COMMA) {
					parser_error(state, "Expected ',' or ')' after function argument.");
					return;
				}
			}
			parser_push_argument_onto_function(&decl, type);
			break;
		}
		case TOKEN_COMMA: {
			goto parser_decl_finished;
		}
		default: {
			parser_add_error_message((char*)UNEXPECTED_TOKEN[tok.type]);
			parser_error(state, ", Expected ',' ')' or 'IDENTIFIER'.");
			return;
		}
		}
	}
parser_decl_finished:
	parser_declare_function(state, decl);
	
}

void parser_declare_function(ParserState* state, ParserFunctionDeclaration func) {
	ParserFunctionDeclaration* dynamic = xmalloc(sizeof(ParserFunctionDeclaration));
	memcpy(dynamic, &func, sizeof(ParserFunctionDeclaration));
	insert_pointers_to_hashtable(state->functions, dynamic->identifier, dynamic, dynamic->identLen, sizeof(ParserFunctionDeclaration));
}

void parser_print_declarations(ParserState* state) {
	for (unsigned int i = 0; i < state->functions->maxNodes; i++) {
		ParserFunctionDeclaration* node = state->functions->nodes[i].val.asPtr;
		if (node != NULL) {
			if (node->nargs == 1) {
				if (node->args[0].name == NULL)
					printf("Function %.*s has argument [INFERRED]. ", node->identLen, node->identifier);
				else
					printf("Function %.*s has argument %.*s. ", node->identLen, node->identifier, node->args[0].nameLen, node->args[0].name);
			}
			else if (node->nargs > 1) {
				printf("Function %.*s has arguments ", node->identLen, node->identifier);
				for (int k = 0; k < node->nargs - 1; k++) {
					if (node->args[k].name == NULL) {
						printf("[INFERRED], ");
					}
					else {
						printf("%.*s ", node->args[k].nameLen, node->args[k].name);
					}
				}
				if (node->args[node->nargs - 1].name == NULL)
					printf("and [INFERRED]. ");
				else
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

ParserIdentifierInfo parser_get_info(ParserState* state, ScannerToken tok) {
	ParserIdentifierInfo info = {
		.atype = parser_what_is_identifier(state, tok.posInSrc, tok.numChars),
		.ptype = parser_what_is_type(state, tok.posInSrc, tok.numChars)
	};
	return info;
}


void parser_global(ParserState* state) {
	ScannerToken typeTok;
	ScannerToken nameTok;
	ParserIdentifierInfo typeInfo;
	ParserIdentifierInfo nameInfo;
	{
		ScannerToken nextTok = parser_advance(state);
		ParserIdentifierInfo info = parser_get_info(state, nextTok);
		if (info.ptype.type == PTYPE_NOTHING) {
			//implicit value
			nameTok = nextTok;
			nameInfo = info;
			ScannerToken _ = {
				.type = TOKEN_NOTHING
			};
			typeTok = _;
			ParserIdentifierInfo t = {
				.atype = AST_NOP,
				.ptype = {
					.type = PTYPE_NOTHING
				}
			};
			typeInfo = t;
		}
		else {
			nameTok = parser_advance(state);
			typeInfo = info;
			typeTok = nextTok;
			nameInfo = parser_get_info(state, nameTok);
		}
	}
	if (!parser_assert_legitimate_identifier(state, nameTok)) {
		parser_error(state, "Illegal variable name.");
	}

	ScannerToken nextTok = parser_advance(state);
	switch (nextTok.type) {
	case TOKEN_EQ: {

	}
	case TOKEN_SEMICOLON: {
		break;
	}
	default: {

	}
	}
}