#pragma once
#include "scanner.h"
#include "allocator.h"
#include <stdint.h>
#include "hashmap.h"
typedef enum AstConstant {
	CONST_FLOAT, CONST_DOUBLE, CONST_UI8,  CONST_I8, CONST_UI16, CONST_I16, CONST_UI32, CONST_I32, CONST_UI64, CONST_I64, CONST_PTR, CONST_CHAR, CONST_STR,
} AstConstant;
// the comments here fall apart a bit
typedef enum AstType {
	//statements
	AST_WHILE=0, AST_FOR, AST_FOREACH, AST_IF, AST_ELSE, AST_ELSEIF, AST_SWITCH, AST_CASE, AST_LOOP, AST_RETURN,

	//expression statements
	AST_ASSIGN, AST_LOCALDECL, AST_GLOBALDECL,
	AST_INC, AST_DEC,
	AST_PLUSEQ, AST_MINUSEQ, AST_TIMESEQ, AST_SLASHEQ, AST_MODEQ, AST_POWEQ,

	//various literals
	AST_INTEGER, AST_FLOAT, AST_STRING, AST_IDENTIFIER, AST_CHAR,

	//resultant arithmetic
	AST_PLUS, AST_MINUS, AST_TIMES, AST_DIV, AST_MOD, AST_POW,

	AST_EQUALS, AST_NOT_EQUAL, AST_AND, AST_OR,


	AST_INDEX,


	//special statements
	AST_CLASS, AST_IMPORT,

	//prefix operators
	AST_UNARY_MINUS, AST_UNARY_NOT,

	AST_BLOCK, AST_GROUPING, AST_EXPRESSION,

	AST_ERROR, AST_NULL, AST_NOP, AST_EOF,
} AstType;

static char* astTypeToString[64] = {
	"while", "for", "foreach", "if", "else", "elseif", "switch", "case", "loop", "return",

	"=", "local", "global",
	"++", "--", "+=",
	"-=", "*=", "/=", "%=", "^=",

	"INTEGER", "FLOAT", "STRING", "IDENTIFIER", "CHAR",

	"+", "-", "*", "/", "%", "^",
	"==", "!=", "&&", "||", "INDEX",
	"class",
	"import",

	"-", "!", "BLOCK", "GROUPING", "EXPR", "ERROR", "NULL", "NOP", "EOF", 
};

//#define EMPTY_AST_TYPE (1 >> (sizeof(AstType) - 1))
#define EMPTY_AST_TYPE AST_NOP
static AstType scannerTokenToAstType[64] = {
	AST_PLUS, AST_MINUS, AST_TIMES, AST_DIV, AST_MOD, AST_POW, EMPTY_AST_TYPE, EMPTY_AST_TYPE,
	AST_PLUSEQ, AST_MINUSEQ, AST_TIMESEQ, AST_SLASHEQ, AST_MODEQ, AST_POWEQ, AST_INC, AST_DEC,

	AST_ASSIGN, AST_EQUALS, AST_UNARY_NOT, AST_NOT_EQUAL, AST_AND, AST_OR,

	AST_GROUPING, EMPTY_AST_TYPE, AST_INDEX, EMPTY_AST_TYPE, EMPTY_AST_TYPE, EMPTY_AST_TYPE, EMPTY_AST_TYPE,

	AST_STRING, AST_INTEGER, AST_FLOAT, AST_IDENTIFIER, AST_CHAR,

	EMPTY_AST_TYPE, EMPTY_AST_TYPE, EMPTY_AST_TYPE, EMPTY_AST_TYPE
};



//typedef enum TokenType {
//	TOKEN_PLUS = 0, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT, TOKEN_CARET, TOKEN_TILDE, TOKEN_COMMA,
//	TOKEN_PLUSEQ, TOKEN_MINUSEQ, TOKEN_STAREQ, TOKEN_SLASHEQ, TOKEN_PERCENTEQ, TOKEN_CARETEQ, TOKEN_PLUSPLUS, TOKEN_MINUSMINUS,
//
//	TOKEN_EQ, TOKEN_EQEQ, TOKEN_BANG, TOKEN_BANGEQ, TOKEN_AMPAMP, TOKEN_BARBAR,
//
//	TOKEN_OPENPAREN, TOKEN_CLOSEPAREN, TOKEN_OPENBRACK, TOKEN_CLOSEBRACK, TOKEN_OPENBRACE, TOKEN_CLOSEBRACE, TOKEN_SEMICOLON,
//
//	TOKEN_STRING, TOKEN_INT, TOKEN_FLOAT, TOKEN_IDENTIFIER, TOKEN_CHAR,
//
//	TOKEN_EOF, TOKEN_START, TOKEN_ERROR, TOKEN_NOTHING,
//} TokenType;

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

	HashTable* functions;
	HashTable* keywords;
	HashTable* types;
	HashTable* globals;

	struct ParserFunctionDeclaration* funcs;
	int numFuncs;
	int maxFuncs;

	int precedenceTable[64];

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
void parser_evaluate(ParserState* state);
void parser_error(ParserState* state, char* msg);
ScannerToken parser_advance(ParserState* state);
AstType parser_what_is_identifier(ParserState* state, char* identifier, size_t len);
void parser_initialize(ParserState* state);
void parser_cleanup(ParserState* state);
void parser_start_for(ParserState* state);
uint8_t parser_expect_tok(ParserState* state, TokenType tok);
AstType parser_get_next_identifier(ParserState* state);
AstNode* parser_begin_expression(ParserState* state, int maxPrecedence, int infixPrecedence);
AstNode* parser_inner_expression(ParserState* state, int maxPrecedence, AstNode* left);
AstNode* parser_get_number(ParserState* state, ScannerToken num);

void parser_decl_pass(ParserState* state);
void parser_add_full_type(ParserState* state, ParserType type);
void parser_add_type(ParserState* state, char* literal, ParserTypesE etype);
void parser_import(ParserState* state);
ParserType parser_what_is_type(ParserState* state, char* typeName, size_t len);
void parser_declare_function(ParserState* state, ParserFunctionDeclaration func);
uint8_t parser_assert_legitimate_identifier(ParserState* state, ScannerToken tok);
void parser_push_argument_onto_function(ParserFunctionDeclaration* func, ParserType type);
void parser_decl(ParserState* state, ScannerToken tokType, ScannerToken tokName);
void parser_print_declarations(ParserState* state);
void parser_print_other_err();
void parser_add_error_message(char* msg);
uint8_t parser_does_function_exist(ParserFunctionDeclaration* func);
void parser_global(ParserState* state);
void parser_add_global(ParserState* state, char* identifier, int identLen, AstNode* value);
void parser_print_ast(ParserState* state, int indentation, AstNode* top);
void parser_backtrack(ParserState* state);
typedef struct ParserIdentifierInfo {
	AstType atype;
	ParserType ptype;
} ParserIdentifierInfo;

ParserIdentifierInfo parser_get_info(ParserState* state, ScannerToken tok);

