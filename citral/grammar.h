#pragma once
//grammar
/*
program = block
block = {statements
statements = statement [}statements]
statement = exprStmt | block | whileStmt | forStmt | ifStmt | switchStmt | returnStmt
exprStmt = functionCall | localDecl | globalDecl | mutation
functionCall = identifier()
			 | identifier.identifier()
identifier = [a-zA-Z][a-zA-Z0-9_]*
whileStmt = while expr: statement
forStmt = for expr, expr, expr: statement
		| for localDecl in expr:
ifStmt = if expr: statement
switchStmt = switch expr {(case expr:)* statement}
returnStmt = return expr
localDecl = local identifier (= expr)?
globalDecl = global identifier (= expr)?
mutation = identifier = expr
expr = grouping | exprStmt | arithmetic



*/