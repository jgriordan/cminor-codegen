// decl.c
// John Riordan

#include "decl.h"

struct decl* decl_create( char* name, struct type* t, struct expr* v, struct stmt* c, struct decl* next ){
	struct decl* d = malloc( sizeof(*d) );
	d->name = name;
	d->type = t;
	d->value = v;
	d->code = c;
	d->symbol = 0;
	d->next = next;
	return d;
}

void decl_print( struct decl* d, int indent ){
	int i;
	if( !d ) return;
	for( i=0; i<indent; i++ )
		printf( "\t" );
	printf( "%s: ", d->name );
	type_print( d->type );
	if( d->code ){
		printf( " =\n{\n" );
		stmt_print( d->code, indent+1 );
		printf( "}\n\n" );
		decl_print( d->next, indent );
		return;
	}
	if( d->value ){
		printf( " = " );
		expr_print( d->value );
	}
	printf( ";\n" );
	decl_print( d->next, indent );
}

void decl_resolve( struct decl* d ){
	int level;
	struct symbol* s;
	if( !d ) return;
	level = scope_level();
	if( !level )
		s = symbol_create( SYMBOL_GLOBAL, d->type, d->name );
	else
		s = symbol_create( SYMBOL_LOCAL, d->type, d->name );
	scope_bind( d->name, s );
	expr_resolve( d->value );
	scope_enter();
	type_resolve( d->type );
	stmt_resolve( d->code );
	scope_leave();
	decl_resolve( d->next );
	d->symbol = s;
}

void decl_typecheck( struct decl* d ){
	struct type* t;
	if( !d ) return;
	if( d->value ){
		t = expr_typecheck( d->value );
		if( !type_equal( d->type, t ) ){
			printf( "type error: cannot assign " );
			type_print( t );
			printf( " (" );
			expr_print( d->value );
			printf( ") to " );
			type_print( d->type );
			printf( " (%s)\n", d->name );
			typecheck_failed++;
		}
	}
	if( d->code ){
		return_type = scope_lookup( d->name )->type->subtype;
		stmt_typecheck( d->code );
	}
	decl_typecheck( d->next );
}

void decl_codegen( struct decl* d ){
	int locals;
	if( !d ) return;
	if( d->symbol && d->symbol->kind == SYMBOL_GLOBAL && d->type ){
		switch( d->type->kind ){
			case TYPE_BOOLEAN:
			case TYPE_CHARACTER:
			case TYPE_INTEGER:
				fprintf( f, ".data\n.globl %s\n%s:\n", d->name, d->name );
				if( d->value ){
					//if( d->type->kind == TYPE_BOOLEAN || d->type->kind == TYPE_INTEGER ){
						fprintf( f, ".quad %d\n", d->value->literal_value );
					//} else {
					//	fprintf( f, ".quad %c\n", d->value->literal_value );
					//}
				} else {
					fprintf( f, ".quad 0\n" );
				}
				break;
			case TYPE_STRING:
				if( d->value )
					fprintf( f, ".data\n%s:\n.string \"%s\"\n", d->name, d->value->string_literal );
				else
					fprintf( f, ".data\n%s:\n.string \"\"\n", d->name );
				break;
			case TYPE_ARRAY:
				break;
			case TYPE_FUNCTION:
				if( d->code ){
					// preamble
					fprintf( f, ".text\n" );
					fprintf( f, ".globl %s\n", d->name );
					fprintf( f, "%s:\n", d->name );
					fprintf( f, "push %%rbp\n" );
					fprintf( f, "movq %%rsp, %%rbp\n" );
					for( locals = 1; locals <= 6; locals ++ ){
						fprintf( f, "pushq %s\n", param_reg( locals ) );
					}
					locals = stmt_count_locals( d->code );
					fprintf( f, "subq $%d, %%rsp\n", 8*locals );
					fprintf( f, "pushq %%rbx\n" );
					fprintf( f, "pushq %%r12\n" );
					fprintf( f, "pushq %%r13\n" );
					fprintf( f, "pushq %%r14\n" );
					fprintf( f, "pushq %%r15\n" );

					// function code
					stmt_codegen( d->code );

					fprintf( f, "movq $0, %%rax\n" );
					postamble();
				}
				break;
			default:
				break;
		}
	} else if( d->symbol && d->symbol->kind == SYMBOL_LOCAL && d->type ){
		switch( d->type->kind ){
			case TYPE_BOOLEAN:
			case TYPE_CHARACTER:
			case TYPE_INTEGER:
			case TYPE_STRING:
				if( d->value ){
					expr_codegen( d->value );
					fprintf( f, "movq %s, %s\n", register_name( d->value->reg ), symbol_code( d->symbol ) );
					register_free( d->value->reg );
				} else {
					fprintf( f, "movq $0, %s\n", symbol_code( d->symbol ) );
				}
				break;
			case TYPE_ARRAY:
				printf( "Local arrays unsupported!\n" );
				codegen_fail();
				return;
			case TYPE_FUNCTION:
				printf( "Local functions unsupported!\n" );
				codegen_fail();
				return;
			default:
				break;
		}
	}
	decl_codegen( d->next );
}
