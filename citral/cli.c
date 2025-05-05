#include "allocator.h"
#include <stdio.h>
#include "scanner.h"
#include "config.h"
#include "parser.h"

char* cli_get_str() {
	char* buf = xmalloc(16);
	size_t bufSize = 16;
	size_t posInBuf = 0;
	char c;
	for (;;) {
		c = fgetc(stdin);
		if (c == '\n') {
			xxrealloc(&buf, posInBuf + 1);
			buf[posInBuf] = 0;
			if (posInBuf == 0) {
				
			}
			return buf;
		}
		buf[posInBuf] = c;
		if ((posInBuf+2) > bufSize) {
			xxrealloc(&buf, bufSize * 2);
			bufSize = bufSize * 2;
		}
		posInBuf++;
	}
}

void load_source(char* src, int isHeap) {
	//todo: scan buf into a ScannerState
	ScannerState* scanState = scanner_create_state(src, strlen(src), isHeap);
	ParserState* parseState = parser_create_state(scanState);
	parser_evaluate(parseState);
#ifdef PARSER_DEBUG
	//parser_print_ast(parseState);
#endif
	//todo: parse the ScannerState into a valid AST
	//todo: ast optimization pass 1: prune unreachable code
	//todo: convert the AST into a DAG
	//todo: dag optimization pass 1: constant folding
	//todo: dag optimization pass 2: loop unrolling
	//todo: dag optimization pass 3: function inlining
	//todo: convert the DAG into a linear bytecode sequence
	//todo: bytecode optimization 1: operation depowering
	//todo: convert the linear bytecode into our assembly-esque IL
	//todo: conservative register allocation
	//todo: asm optimization pass 1: register reuse
	//todo: asm optimization pass 2: redundant instruction reduction 
	//todo: asm optimization pass 3: pattern recognizing chunk optimization
	//todo: asm optimization pass 4: aggressive register allocation decisions. find ways to reuse registers
	//todo: convert to asm
	//todo: assemble
	//todo: link to standard library (i need to make a standard library)
	//todo: run code
	parser_cleanup(parseState);
}

void repl() {
	for (;;) {
		printf("Please enter a source string.\n");
		char** strings = xmalloc(sizeof(char*) * 16);
		size_t numStrs = 1;
		size_t maxStrs = 16;
		size_t totalStrLen = 0;
		printf("> ");
		char* firstStr = cli_get_str();
		while (firstStr[0] == 0) {
			printf("> ");
			firstStr = cli_get_str();
		}
		totalStrLen += strlen(firstStr) + 1;
		strings[0] = firstStr;
		while (1) {
			printf(">> ");
			char* str = cli_get_str();
			if (numStrs >= maxStrs) {
				xxrealloc(&strings, sizeof(char*) * maxStrs * 2);
			}
			if (str[0] == '\0') {
				break;
			}
			int len = (int)strlen(str);
			totalStrLen += len + 1;
			strings[numStrs] = str;
			numStrs++;
		}
		char* buf = xmalloc(totalStrLen+1);
		size_t inc = 0;
		for (int i = 0; i < numStrs; i++) {
			for (int j = 0; j < strlen(strings[i])+1; j++) {
				char toInsert = strings[i][j];
				if (toInsert == '\0') {
					toInsert = '\n';
				}
				buf[inc] = toInsert;
				inc++;
			}
			free(strings[i]);
		}
		buf[inc] = '\0';
		load_source(buf, 1);
		free(strings);
	}
}

void tests() {
	load_source("your mom", 0);
}

int main() {
	while (1) {
		printf("Type \"test\" to perform all tests.\nType \"repl\" to enter the repl.\nType \"file\" followed by a file path to load said file\n");
		char* str = cli_get_str();
		if (!strcmp(str, "test")) {
			tests();
			break;
		}
		else if (!strcmp(str, "repl")) {
			repl();
			break;
		}
		else if (!strcmp(str, "file")) {
			printf("Unimplemented.\n");
		}
	}
	repl();
}
//