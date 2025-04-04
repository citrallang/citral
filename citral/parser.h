#pragma once
#include "scanner.h"
#include "allocator.h"
#include <stdint.h>
typedef enum astType {
	//control flow
	AST_WHILE, AST_FOR, AST_FOREACH, AST_IF, AST_SWITCH, AST_LOOP,

	//constants
	AST_FLOAT, AST_DOUBLE, AST_UI8, AST_CHAR, AST_I8, AST_UI16, AST_I16, AST_UI32, AST_I32, AST_UI64, AST_I64, AST_PTR, AST_STR,

	//resultant arithmetic
	AST_PLUS, AST_MINUS, AST_TIMES, AST_DIV, AST_MOD, AST_POW,


	AST_MODIFY, AST_LOCALDECL, AST_GLOBALDECL, 

	AST_ERROR,
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
	struct astNode* left;
	struct astNode* right;
	astLiteralUnion literal;
} astNode;

typedef struct parserState {
	scannerState* encompassingScanner;
	astNode** program;
	size_t programSize;
	size_t programCapacity;
} parserState;

parserState* parser_create_state(scannerState* encompassing);
parserState* parser_evaluate_scanner(scannerState* scState);
void parser_evaluate(parserState* state);
void parser_evaluate_ast_node(parserState* state, astNode* node);
void parser_error(parserState* state, char* msg);
void parser_scan_token(state);