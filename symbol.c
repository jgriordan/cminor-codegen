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
			sprintf( str, "-%d(%%rbp)", s->which*8 + 48 );
			break;
		case SYMBOL_PARAM:
			if( s->which < 1 || s->which > 6 ){
				printf( "Only up to 6 function parameters are allowed\n" );
				codegen_fail();
			}
			sprintf( str, "-%d(%%rbp)", s->which*8 );
	}
	return str;
}
