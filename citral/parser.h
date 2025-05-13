#pragma once
#include "scanner.h"
#include "allocator.h"
#include <stdint.h>
#include "hashmap.h"
typedef enum AstConstant {
	CONST_FLOAT, CONST_DOUBLE, CONST_UI8,  CONST_I8, CONST_UI16, CONST_I16, CONST_UI32, CONST_I32, CONST_UI64, CONST_I64, CONST_PTR, CONST_CHAR, CONST_STR,
} AstConstant;

typedef enum AstType {
	//statements
	AST_WHILE, AST_FOR, AST_FOREACH, AST_IF, AST_ELSE, AST_ELSEIF, AST_SWITCH, AST_CASE, AST_LOOP, AST_RETURN,

	//expression statements
	AST_MODIFY, AST_LOCALDECL, AST_GLOBALDECL,
	AST_INC, AST_DEC,

	//various literals
	AST_NUMBER, AST_STRING, AST_IDENTIFIER,

	//resultant arithmetic
	AST_PLUS, AST_MINUS, AST_TIMES, AST_DIV, AST_MOD, AST_POW,

	//special statements
	AST_CLASS, AST_IMPORT,

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

	struct ParserFunctionDeclaration* funcs;
	int numFuncs;
	int maxFuncs;

	unsigned int hadError : 1;
} ParserState;

typedef struct ParserKeyword {
	char* literal;
	unsigned int literalLen;
	AstType whatAreYou;
} ParserKeyword;



typedef union ParserPrimitiveTypes {
	void* asPtr;
	uint8_t as8Bit;
	uint16_t as16Bit;
	uint32_t as32Bit;
	uint64_t as64Bit;
	int8_t as8BitS;
	int16_t as16BitS;
	int32_t as32BitS;
	int64_t as64BitS;
	float asFlt;
	double asDbl;
} ParserPrimitiveTypes;

typedef struct ParserPrimitive {
	ParserPrimitiveTypes literal;
} ParserPrimitive;

/*
Primitives are stored directly inside ParserType.
In the case of a struct, class, or function "name" instead points to a ParserBigType.
*/

typedef enum ParserTypesE {
	PTYPE_POINTER, PTYPE_U8, PTYPE_U16, PTYPE_U32, PTYPE_U64, PTYPE_I8, PTYPE_I16, PTYPE_I32, PTYPE_I64, PTYPE_FLOAT, PTYPE_DOUBLE,


	PTYPE_USERSTRUCT,
	PTYPE_USERCLASS,
	PTYPE_FUNCTION,

	PTYPE_VOID, //normal void

	PTYPE_NOTHING, //error/empty type
} ParserTypesE;

typedef struct ParserType {
	char* name;
	int nameLen;
	ParserTypesE type;
} ParserType;


typedef struct ParserBigTypeSegment {
	ParserType type;
	int size; //if type is a struct, this can be bigger than 8
} ParserBigTypeSegment;

typedef struct ParserBigType {
	char* name;
	int nameLen;
	ParserTypesE type;

} ParserBigType;

typedef struct ParserFunctionDeclaration {
	ParserType* args;
	int nargs;
	int maxArgs;
	char* identifier;
	int identLen;
	ParserType retType;
} ParserFunctionDeclaration;

AstNode parser_create_node(AstType type);
AstNode parser_create_node_literal(AstType type, AstLiteralUnion literal);
ParserState* parser_create_state(ScannerState* encompassing);
ParserState* parser_evaluate_scanner(ScannerState* scState);
void parser_evaluate(ParserState* state);
void parser_evaluate_ast_node(ParserState* state, AstNode* node);
void parser_error(ParserState* state, char* msg);
void parser_warn(ParserState* state, char* msg);
AstNode parser_scan_token(ParserState* state);
ScannerToken parser_advance(ParserState* state);
AstType parser_what_is_identifier(char* identifier, int len);
void parser_initialize();
void parser_uninitialize();
void parser_cleanup(ParserState* state);
void parser_start_for(ParserState* state);
uint8_t parser_expect_tok(ParserState* state, TokenType tok);
AstType parser_get_next_identifier(ParserState* state);
AstNode* parser_expression(ParserState* state);
void parser_decl_pass(ParserState* state);
void parser_import_pass(ParserState* state);
void parser_definition_pass(ParserState* state);
void parser_add_full_type(ParserType type);
void parser_add_type(char* literal, ParserTypesE etype);
void parser_import(ParserState* state);
ParserType parser_what_is_type(char* typeName, int len);
void parser_declare_function(ParserFunctionDeclaration func);
uint8_t parser_is_legitimate_identifier(ParserState* state, ScannerToken tok);
void parser_push_argument_onto_function(ParserFunctionDeclaration* func, ParserType type);
void parser_decl(ParserState* state, ScannerToken tokType, ScannerToken tokName);
void parser_print_declarations();
void parser_print_other_err();
void parser_add_error_message(char* msg);
uint8_t parser_does_function_exist(ParserFunctionDeclaration* func);
void parser_decl(ParserState* state);

typedef struct ParserIdentifierInfo {
	AstType atype;
	ParserType ptype;
} ParserIdentifierInfo;

ParserIdentifierInfo parser_get_info(ScannerToken tok);

static HashTable parserFunctionTable = {
	.usePrimitiveHasher = 0,
};


static HashTable parserTypeTable = {
	.usePrimitiveHasher = 0,
};

static HashTable parser_reserved_keywords = {
	.usePrimitiveHasher = 0,
};

