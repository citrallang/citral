# citral, WIP

All the following are plans that may change, and likely are not implemented.

Citral is a general-purpose, typed (with inference) language written in C. It features a garbage collector, interoperability with several major languages (hopefully), and a twin compiler/interpreter optimized for (and will initially only work for) x86-64 Windows.



## TODO:

Compiler todos are located in cli.c. For now, a working interpreter would be the goal.

Syntax needs to be developed and a grammar needs to be created.

Standard library needs to be created (bulk of the work should be here, but a functional compiler needs to exist first)

Make an LSP & syntax highlighter.

Compiling to different instruction sets (mainly 32 bit x86, ARM)

Compiling to different operating systems (Non-intel macos, linux)
