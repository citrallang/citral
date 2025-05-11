#pragma once
#include "scanner.h"
#include "allocator.h"
#include <stdint.h>

typedef enum AstConstant {
	CONST_FLOAT, CONST_DOUBLE, CONST_UI8,  CONST_I8, CONST_UI16, CONST_I16, CONST_UI32, CONST_I32, CONST_UI64, CONST_I64, CONST_PTR, CONST_CHAR, CONST_STR,
} AstConstant;

typedef enum AstType {
	//statements
	AST_WHILE, AST_FOR, AST_FOREACH, AST_IF, AST_ELSE, AST_ELSEIF, AST_SWITCH, AST_CASE, AST_LOOP, AST_RETURN, AST_IMPORT,

	//expression statements
	AST_MODIFY, AST_LOCALDECL, AST_GLOBALDECL,
	AST_INC, AST_DEC,

	//various literals
	AST_NUMBER, AST_STRING, AST_IDENTIFIER,

	//resultant arithmetic
	AST_PLUS, AST_MINUS, AST_TIMES, AST_DIV, AST_MOD, AST_POW,



	AST_BLOCK,

	AST_ERROR, AST_NULL, AST_NOP, AST_EOF,
} AstType;



typedef union AstLiteralUnion {
	float asFloat;
	double asDouble;
	char asChar;
	uint8_t asUI8;
	uint16_t asUI16;
	uint32_t asUI32;
	uint64_t asUI64;
	int8_t asI8;
	int16_t asI16;
	int32_t asI32;
	int64_t asI64;
	char* asStr;
	void* asPtr;
} AstLiteralUnion;



typedef struct AstNode {
	AstType type;
	struct AstNode* left; //if type == AST_BLOCK then this is an array of statements
	struct AstNode* right;
	AstLiteralUnion literal;
} AstNode;

typedef struct ParserState {
	ScannerState* encompassingScanner;
	size_t scannerPos;

	AstNode** program;
	size_t programSize;
	size_t programCapacity;
	unsigned int hadError : 1;
} ParserState;

typedef struct ParserKeyword {
	char* literal;
	unsigned int literalLen;
	AstType whatAreYou;
} ParserKeyword;

typedef struct ParserType {
	
} ParserType;

typedef union ParserPrimitiveTypes {

} ParserPrimitiveTypes;

typedef struct ParserFunctionDeclaration {

} ParserFunctionDeclaration;

AstNode parser_create_node(AstType type);
AstNode parser_create_node_literal(AstType type, AstLiteralUnion literal);
ParserState* parser_create_state(ScannerState* encompassing);
ParserState* parser_evaluate_scanner(ScannerState* scState);
void parser_evaluate(ParserState* state);
void parser_evaluate_ast_node(ParserState* state, AstNode* node);
void parser_error(ParserState* state, char* msg);
AstNode parser_scan_token(ParserState* state);
ScannerToken parser_advance(ParserState* state);
AstType parser_what_is_identifier(char* identifier, int len);
void parser_initiate_keyword_list();
void parser_cleanup(ParserState* state);
void parser_start_for(ParserState* state);
uint8_t parser_expect_tok(ParserState* state, TokenType tok);
AstType parser_get_next_identifier(ParserState* state);
AstNode* parser_expression(ParserState* state);
void parser_decl_pass(ParserState* state);
void parser_import_pass(ParserState* state);
void parser_definition_pass(ParserState* state);