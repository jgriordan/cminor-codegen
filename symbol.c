// symbol.c
// John Riordan

#include "symbol.h"

struct symbol* symbol_create(symbol_t kind, struct type* type, char* name ){
	struct symbol* s = malloc( sizeof(*s) );
	s->kind = kind;
	s->type = type;
	s->name = name;
	return s;
}

void symbol_print( struct symbol* s ){
	switch( s->kind ){
		case SYMBOL_LOCAL:
			printf( "local %d", s->which );
			break;
		case SYMBOL_PARAM:
			printf( "param %d", s->which );
			break;
		case SYMBOL_GLOBAL:
			printf( "global %s", s->name );
			break;
	}
}

char* symbol_code( struct symbol* s ){
	static char str[256];
	switch( s->kind ){
		case SYMBOL_GLOBAL:
			sprintf( str, s->name );
			break;
		case SYMBOL_LOCAL:
			sprintf( str, "-%d(%%rbp)", s->which*8 );
			break;
		case SYMBOL_PARAM:
			switch( s->which ){
				case 1:
					sprintf( str, "%%rdi" );
					break;
				case 2:
					sprintf( str, "%%rsi" );
					break;
				case 3:
					sprintf( str, "%%rdx" );
					break;
				case 4:
					sprintf( str, "%%rcx" );
					break;
				case 5:
					sprintf( str, "%%r8" );
					break;
				case 6:
					sprintf( str, "%%r9" );
					break;
				default:
					printf( "too many parameters!\n" );
					codegen_failed++;
					break;
			}
			break;
	}
	return str;
}
