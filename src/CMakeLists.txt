
add_executable(yezik
    main.c
        
    error.h
    error.c

    lexer.h
    lexer.c

    ast.h
    ast.c

    parser.h
    parser.c

    generator.h
    generator.c

    emitter.h
    emitter.c

    compiler.h
    compiler.c
)
