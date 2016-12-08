// stmt.c
// John Riordan

#include "stmt.h"

struct stmt* stmt_create( stmt_kind_t kind, struct decl* d, struct expr* init_expr, struct expr* e, struct expr* next_expr, struct stmt* body, struct stmt* else_body, struct stmt* next ){
	struct stmt* s = malloc( sizeof(*s) );
	s->kind = kind;
	s->decl = d;
	s->init_expr = init_expr;
	s->expr = e;
	s->next_expr = next_expr;
	s->body = body;
	s->else_body = else_body;
	s->next = next;
	return s;
}

void stmt_indent( int indent ){
	int i;
	for( i=0; i<indent; i++ )
		printf( "\t" );
}

void stmt_print( struct stmt* s, int indent ){
	if( !s ) return;
	switch( s->kind ){
		case STMT_DECL:
			decl_print( s->decl, indent );
			break;
		case STMT_EXPR:
			stmt_indent( indent );
			expr_print( s->expr );
			printf( ";\n" );
			break;
		case STMT_IF_ELSE:
			stmt_indent( indent );
			printf( "if( " );
			expr_print( s->expr );
			printf( " )\n" );
			if( s->body ){
				if( s->body->kind == STMT_BLOCK )
					stmt_print( s->body, indent );
				else
					stmt_print( s->body, indent+1 );
			}
			if( s->else_body ){
				stmt_indent( indent );
				printf( "else\n" );
				if( s->else_body->kind == STMT_BLOCK )
					stmt_print( s->else_body, indent );
				else
					stmt_print( s->else_body, indent+1 );
			}
			break;
		case STMT_FOR:
			stmt_indent( indent );
			printf( "for( " );
			expr_print( s->init_expr );
			printf( "; " );
			expr_print( s->expr );
			printf( "; " );
			expr_print( s->next_expr );
			printf( " )\n" );
			if( s->body->kind == STMT_BLOCK )
				stmt_print( s->body, indent );
			else
				stmt_print( s->body, indent+1 );
			break;
		case STMT_PRINT:
			stmt_indent( indent );
			printf( "print" );
			if( s->expr ){
				printf( " " );
				expr_print( s->expr );
			}
			printf( ";\n" );
			break;
		case STMT_RETURN:
			stmt_indent( indent );
			printf( "return" );
			if( s->expr ){
				printf( " " );
				expr_print( s->expr );
			}
			printf( ";\n" );
			break;
		case STMT_BLOCK:
			stmt_indent( indent );
			printf( "{\n" );
			stmt_print( s->body, indent+1 );
			stmt_indent( indent );
			printf( "}\n" );
			break;
	}
	stmt_print( s->next, indent );
}

void stmt_resolve( struct stmt* s ){
	if( !s ) return;
	decl_resolve( s->decl );
	expr_resolve( s->init_expr );
	expr_resolve( s->expr );
	expr_resolve( s->next_expr );
	// scope_enter();
	stmt_resolve( s->body );
	// scope_leave();
	// scope_enter();
	stmt_resolve( s->else_body );
	// scope_leave();
	stmt_resolve( s->next );
}

void stmt_typecheck( struct stmt* s ){
	struct type* t;
	if( !s ) return;
	switch( s->kind ){
		case STMT_DECL:
			decl_typecheck( s->decl );
			break;
		case STMT_EXPR:
		case STMT_PRINT:
			expr_typecheck( s->expr );
			break;
		case STMT_IF_ELSE:
			t = expr_typecheck( s->expr );
			if( t->kind != TYPE_BOOLEAN ){
				printf( "type error: expression (" );
				expr_print( s->expr );
				printf( ") in if must be boolean, not " );
				type_print( t );
				printf( "\n" );
				typecheck_failed++;
			}
			stmt_typecheck( s->body );
			stmt_typecheck( s->else_body );
			break;
		case STMT_FOR:
			expr_typecheck( s->init_expr );
			expr_typecheck( s->expr );
			expr_typecheck( s->next_expr );
			stmt_typecheck( s->body );
			break;
		case STMT_RETURN:
			t = expr_typecheck( s->expr );
			if( !type_equal( t, return_type ) && !(!t && return_type->kind == TYPE_VOID) ){
				printf( "type error: return of " );
				type_print( t );
				printf( " (" );
				expr_print( s->expr );
				printf( ") does not match the function return type of " );
				type_print( return_type );
				printf( "\n" );
				typecheck_failed++;
			}
			break;
		case STMT_BLOCK:
			stmt_typecheck( s->body );
			break;
	}
	stmt_typecheck( s->next );
}

void stmt_codegen( struct stmt* s ){
	struct expr* e;
	int i;
	if( !s ) return;
	switch( s->kind ){
		case STMT_DECL:
			decl_codegen( s->decl );
			break;
		case STMT_EXPR:
			expr_codegen( s->expr );
			register_free( s->expr->reg );
			break;
		case STMT_IF_ELSE:
			expr_codegen( s->expr );
			fprintf( f, "cmpq $0, %s\n", register_name( s->expr->reg ) );
			i = marker_get();
			marker_increment();
			marker_increment();
			fprintf( f, "je .L%d\n", i );
			stmt_codegen( s->body );
			fprintf( f, "jmp .L%d\n", i+1 );
			marker_print( i );
			stmt_codegen( s->else_body );
			marker_print( i+1 );
			register_free( s->expr->reg );
			break;
		case STMT_FOR:
			expr_codegen( s->init_expr );
			register_free( s->init_expr->reg );
			i = marker_get();
			marker_increment();
			marker_increment();
			marker_print( i );
			expr_codegen( s->expr );
			fprintf( f, "cmpq $0, %s\n", register_name( s->expr->reg ) );
			register_free( s->expr->reg );
			fprintf( f, "je .L%d", i+1 );
			stmt_codegen( s->body );
			expr_codegen( s->next_expr );
			register_free( s->next_expr->reg );
			fprintf( f, "jmp .L%d\n", i );
			marker_print( i+1 );
			break;
		case STMT_PRINT:
			e = s->expr;
			if( !e ) return;
			if( !e->right ) stmt_codegen_print( e );
			while( e->right && e->right->kind == EXPR_LIST ){
				stmt_codegen_print( e->left );
				e = e->right;
			}
			stmt_codegen_print( e->left );
			stmt_codegen_print( e->right );
			break;
		case STMT_RETURN:
			if( s->expr ){
				expr_codegen( s->expr );
				fprintf( f, "movq %s, %%rax\n", register_name( s->expr->reg ) );
				register_free( s->expr->reg );
			}
			postamble();
			break;
		case STMT_BLOCK:
			stmt_codegen( s->body );
			break;
	}
	stmt_codegen( s->next );
}

void stmt_codegen_print( struct expr* e ){
	struct type* t;
	int i;
	if( !e ) return;
	expr_codegen( e );
	t = expr_typecheck( e );
	switch( t->kind ){
		case TYPE_BOOLEAN:
			i = marker_get();
			fprintf( f, ".data\n" );
			marker_increment();
			marker_print( i );
			fprintf( f, ".string \"true\"\n" );
			marker_increment();
			marker_print( i+1 );
			fprintf( f, ".string \"false\"\n" );
			fprintf( f, ".text\n" );
			fprintf( f, "cmpq $0, %s\n", register_name( e->reg ) );
			fprintf( f, "je .L%d\n", i+2 );
			fprintf( f, "leaq .L%d, %%rdi\n", i );
			fprintf( f, "jmp .L%d\n", i+3 );
			marker_increment();
			marker_print( i+2 );
			fprintf( f, "leaq .L%d, %%rdi\n", i+1 );
			marker_increment();
			marker_print( i+3 );
			fprintf( f, "movq $0, %%rax\n" );
			before_fn_call();
			fprintf( f, "call printf\n" );
			after_fn_call();
			break;
		case TYPE_CHARACTER:
			fprintf( f, "movq %s, %%rdi\n", register_name( e->reg ) );
			before_fn_call();
			fprintf( f, "call putchar\n" );
			after_fn_call();
			break;
		case TYPE_INTEGER:
			fprintf( f, "movq %s, %%rsi\n", register_name( e->reg ) );
			fprintf( f, ".data\n" );
			i = marker_get();
			marker_increment();
			marker_print( i );
			fprintf( f, ".string \"%%d\"\n" );
			fprintf( f, ".text\n" );
			fprintf( f, "leaq .L%d, %%rdi\n", i );
			fprintf( f, "movq $0, %%rax\n" );
			before_fn_call();
			fprintf( f, "call printf\n" );
			after_fn_call();
			break;
		case TYPE_STRING:
			fprintf( f, "movq %s, %%rdi\n", register_name( e->reg ) );
			fprintf( f, "movq $0, %%rax\n" );
			before_fn_call();
			fprintf( f, "call printf\n" );
			after_fn_call();
			break;
		default:
			break;
	}
	register_free( e->reg );
}

int stmt_count_locals( struct stmt* s ){
	if( !s ) return 0;
	if( s->kind == STMT_DECL ) return 1 + stmt_count_locals( s->body ) + stmt_count_locals( s->else_body ) + stmt_count_locals( s->next );
	else return stmt_count_locals( s->body ) + stmt_count_locals( s->else_body ) + stmt_count_locals( s->next );
}
