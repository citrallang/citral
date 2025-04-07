#pragma once
#include "scanner.h"
#include "allocator.h"
#include <stdint.h>

typedef enum astConstant {
	CONST_FLOAT, CONST_DOUBLE, CONST_UI8,  CONST_I8, CONST_UI16, CONST_I16, CONST_UI32, CONST_I32, CONST_UI64, CONST_I64, CONST_PTR, CONST_CHAR, CONST_STR,
};

typedef enum astType {
	//statements
	AST_WHILE, AST_FOR, AST_FOREACH, AST_IF, AST_SWITCH, AST_LOOP, AST_RETURN,

	//expression statements
	AST_MODIFY, AST_LOCALDECL, AST_GLOBALDECL,
	AST_INC, AST_DEC,

	AST_CONSTANT,

	//resultant arithmetic
	AST_PLUS, AST_MINUS, AST_TIMES, AST_DIV, AST_MOD, AST_POW,



	AST_BLOCK,

	AST_ERROR, AST_NULL, AST_NOP, AST_EOF,
} astType;

typedef union astLiteralUnion {
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
} astLiteralUnion;



typedef struct astNode {
	astType type;
	struct astNode* left; //if type == AST_BLOCK then this is an array of statements
	struct astNode* right;
	astLiteralUnion literal;
} astNode;

typedef struct parserState {
	scannerState* encompassingScanner;
	size_t scannerPos;

	astNode* program;
	size_t programSize;
	size_t programCapacity;
} parserState;



astNode parser_create_node(astType type);
astNode parser_create_node_literal(astType type, astLiteralUnion literal);
parserState* parser_create_state(scannerState* encompassing);
parserState* parser_evaluate_scanner(scannerState* scState);
void parser_evaluate(parserState* state);
void parser_evaluate_ast_node(parserState* state, astNode* node);
void parser_error(parserState* state, char* msg);
astNode parser_scan_token(parserState* state);
scannerToken parser_advance(parserState* state);