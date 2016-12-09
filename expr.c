// expr.c
// John Riordan

#include "expr.h"

struct expr* expr_create( expr_t kind, struct expr* left, struct expr* right ){
	struct expr* e = malloc( sizeof(*e) );
	e->kind = kind;
	e->left = left;
	e->right = right;
	e->name = 0;
	e->symbol = 0;
	e->literal_value = 0;
	e->string_literal = 0;
	e->reg = 0;
	return e;
}

struct expr* expr_create_id( const char* n ){
	struct expr* e = malloc( sizeof(*e) );
	e->kind = EXPR_ID;
	e->left = 0;
	e->right = 0;
	e->name = n;
	e->symbol = 0;
	e->literal_value = 0;
	e->string_literal = 0;
	e->reg = 0;
	return e;
}

struct expr* expr_create_boolean_literal( int c ){
	struct expr* e = malloc( sizeof(*e) );
	e->kind = EXPR_BOOLEAN_LITERAL;
	e->left = 0;
	e->right = 0;
	e->name = 0;
	e->symbol = 0;
	e->literal_value = c;
	e->string_literal = 0;
	e->reg = 0;
	return e;
}


struct expr* expr_create_integer_literal( int c ){
	struct expr* e = malloc( sizeof(*e) );
	e->kind = EXPR_INTEGER_LITERAL;
	e->left = 0;
	e->right = 0;
	e->name = 0;
	e->symbol = 0;
	e->literal_value = c;
	e->string_literal = 0;
	e->reg = 0;
	return e;
}

struct expr* expr_create_character_literal( int c ){
	struct expr* e = malloc( sizeof(*e) );
	e->kind = EXPR_CHARACTER_LITERAL;
	e->left = 0;
	e->right = 0;
	e->name = 0;
	e->symbol = 0;
	e->literal_value = c;
	e->string_literal = 0;
	e->reg = 0;
	return e;
}

struct expr* expr_create_string_literal( const char* str ){
	struct expr* e = malloc( sizeof(*e) );
	e->kind = EXPR_STRING_LITERAL;
	e->left = 0;
	e->right = 0;
	e->name = 0;
	e->symbol = 0;
	e->literal_value = 0;
	e->string_literal = str;
	e->reg = 0;
	return e;
}

void expr_print( struct expr* e ){
	if( !e ) return;
	if( e->kind == EXPR_INDEX ) printf( "[" );
	else if( e->kind == EXPR_AR_INIT ) printf( "{" );
	expr_print( e->left );
	switch( e->kind ){
		case EXPR_ASGN: printf( " = " ); break;
		case EXPR_OR: printf( " || " ); break;
		case EXPR_AND: printf( " && " ); break;
		case EXPR_LE: printf( " <= " ); break;
		case EXPR_GE: printf( " >= " ); break;
		case EXPR_LT: printf( " < " ); break;
		case EXPR_GT: printf( " > " ); break;
		case EXPR_EQ: printf( " == " ); break;
		case EXPR_NE: printf( " != " ); break;
		case EXPR_ADD: printf( " + " ); break;
		case EXPR_SUB: printf( " - " ); break;
		case EXPR_MUL: printf( " * " ); break;
		case EXPR_DIV: printf( " / " ); break;
		case EXPR_MOD: printf( " %% " ); break;
		case EXPR_EXP: printf( " ^ " ); break;
		case EXPR_NOT: printf( "!" ); break;
		case EXPR_NEG: printf( "-" ); break;
		case EXPR_INCREMENT: printf( "++" ); break;
		case EXPR_DECREMENT: printf( "--" ); break;
		case EXPR_ID: printf( "%s", e->name ); break;
		case EXPR_CHARACTER_LITERAL:
			if( (char)e->literal_value == '\0' )
				printf( "\'\\0\'" );
			else if( (char)e->literal_value == '\n' )
				printf( "\'\\n\'" );
			else
				printf( "\'%c\'", (char)e->literal_value );
			break;
		case EXPR_STRING_LITERAL: printf( "%s", e->string_literal ); break;
		case EXPR_INTEGER_LITERAL: printf( "%d", e->literal_value ); break;
		case EXPR_BOOLEAN_LITERAL: if( e->literal_value ) printf( "true" ); else printf( "false" ); break;
		case EXPR_ARRAY: break;
		case EXPR_INDEX: printf( "]" ); break;
		case EXPR_CALL: printf( "(" ); break;
		case EXPR_PAREN: printf( "(" ); break;
		case EXPR_AR_INIT: if( e->right ) printf( "}, " ); break;
		case EXPR_LIST: if( e->left ) printf( ", " ); break;
	}
	expr_print( e->right );
	if( e->kind == EXPR_CALL || e->kind == EXPR_PAREN ) printf( ")" );
	else if( e->kind == EXPR_AR_INIT && !e->right ) printf( "}" );
}

int expr_list_equal_param_list( struct expr* e, struct param_list* p ){
	struct type* t;
	if( p && e ){
		if( e->left && e->kind == EXPR_LIST )
			t = expr_typecheck( e->left );
		else
			t = expr_typecheck( e );
		if( type_equal( t, p->type ) ){
			type_delete( t );
			if( e->kind == EXPR_LIST )
				return expr_list_equal_param_list( e->right, p->next );
			else
				return 1;
		}
		type_delete( t );
		return 0;
	}	
	else if( !p && !e )
		return 1;
	else
		return 0;
}

void expr_resolve( struct expr* e ){
	if( !e ) return;
	if( e->kind == EXPR_ID ){
		e->symbol = scope_lookup( e->name );
		if( !(e->symbol) ){
			printf( "resolve error: %s is not defined\n", e->name );
			resolve_failed++;
		} else if( resolve_print ) {
			switch( e->symbol->kind ){
				case SYMBOL_GLOBAL:
					printf( "%s resolves to global %s\n", e->name, e->symbol->name );
					break;
				case SYMBOL_LOCAL:
					printf( "%s resolves to local %d\n", e->name, e->symbol->which );
					break;
				case SYMBOL_PARAM:
					printf( "%s resolves to param %d\n", e->name, e->symbol->which );
					break;
			}
		}
	} else {
		expr_resolve( e->left );
		expr_resolve( e->right );
	}
}


struct type* expr_typecheck( struct expr* e ){
	struct type* lt;
	struct type* rt;
	struct type* result = 0;
	if( !e ) return 0;
	lt = expr_typecheck( e->left );
	rt = expr_typecheck( e->right );
	switch( e->kind ){
		case EXPR_ASGN:
			if( !type_equal( lt, rt ) ){
				printf( "type error: cannot assign " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( ") to " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ")\n" );
				typecheck_failed++;
			}
			result = type_copy( lt );
			break;
		case EXPR_OR:
			if( lt->kind != TYPE_BOOLEAN ){
				printf( "type error: left side of || (" );
				expr_print( e->left );
				printf( ") must be boolean, not " );
				type_print( lt );
				printf( "\n" );
				typecheck_failed++;
			}
			if( rt->kind != TYPE_BOOLEAN ){
				printf( "type error: right side of || (" );
				expr_print( e->right );
				printf( ") must be boolean, not " );
				type_print( rt );
				printf( "\n" );
				typecheck_failed++;
			}
			result = type_create( TYPE_BOOLEAN, 0, 0, 0 );
			break;
		case EXPR_AND:
			if( lt->kind != TYPE_BOOLEAN ){
				printf( "type error: left side of && (" );
				expr_print( e->left );
				printf( ") must be boolean, not " );
				type_print( lt );
				printf( "\n" );
				typecheck_failed++;
			}
			if( rt->kind != TYPE_BOOLEAN ){
				printf( "type error: right side of && (" );
				expr_print( e->right );
				printf( ") must be boolean, not " );
				type_print( rt );
				printf( "\n" );
				typecheck_failed++;
			}
			result = type_create( TYPE_BOOLEAN, 0, 0, 0 );
			break;
		case EXPR_LE:
		case EXPR_GE:
		case EXPR_LT:
		case EXPR_GT:
			if( lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER ){
				printf( "type error: cannot compare " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ") to " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( "), both must be integer\n" );
				typecheck_failed++;
			}
			result = type_create( TYPE_BOOLEAN, 0, 0, 0 );
			break;
		case EXPR_EQ:
		case EXPR_NE:
			if( lt->kind == TYPE_VOID || lt->kind == TYPE_ARRAY || lt->kind == TYPE_FUNCTION || rt->kind == TYPE_VOID || rt->kind == TYPE_ARRAY || rt->kind == TYPE_FUNCTION ){
				printf( "type error: cannot compare " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ") to " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( "), both must be primitive\n" );
				typecheck_failed++;
			} else if( !type_equal( lt, rt ) ){
				printf( "type error: cannot compare " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ") to " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( ")\n" );
				typecheck_failed++;
			}
			result = type_create( TYPE_BOOLEAN, 0, 0, 0 );
			break;
		case EXPR_ADD:
			if( lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER ){
				printf( "type error: cannot add " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ") with " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( "), both must be integer\n");
				typecheck_failed++;
			}
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_SUB:
			if( lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER ){
				printf( "type error: cannot subtract " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ") with " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( "), both must be integer\n");
				typecheck_failed++;
			}
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_MUL:
			if( lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER ){
				printf( "type error: cannot multiply " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ") with " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( "), both must be integer\n");
				typecheck_failed++;
			}
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_DIV:
			if( lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER ){
				printf( "type error: cannot divide " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ") with " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( "), both must be integer\n");
				typecheck_failed++;
			}
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_MOD:
			if( lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER ){
				printf( "type error: cannot modulus " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ") with " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( "), both must be integer\n");
				typecheck_failed++;
			}
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_EXP:
			if( lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER ){
				printf( "type error: cannot raise " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ") to the exponent of " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( "), both must be integer\n");
				typecheck_failed++;
			}
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_NOT:
			if( rt->kind != TYPE_BOOLEAN ){
				printf( "type error: cannot \"not\" " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( "), must be boolean\n" );
				typecheck_failed++;
			}
			result = type_create( TYPE_BOOLEAN, 0, 0, 0 );
			break;
		case EXPR_NEG:
			if( rt->kind != TYPE_INTEGER ){
				printf( "type error: cannot negate " );
				type_print( rt );
				printf( " (" );
				expr_print( e->right );
				printf( "), must be integer\n" );
				typecheck_failed++;
			}
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_INCREMENT:
			if( lt->kind != TYPE_INTEGER ){
				printf( "type error: cannot increment " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( "), must be integer\n" );
				typecheck_failed++;
			}
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_DECREMENT:
			if( lt->kind != TYPE_INTEGER ){
				printf( "type error: cannot decrement " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( "), must be integer" );
				typecheck_failed++;
			}
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_ID:
			result = type_copy( e->symbol->type );
			break;
		case EXPR_CHARACTER_LITERAL:
			result = type_create( TYPE_CHARACTER, 0, 0, 0 );
			break;
		case EXPR_STRING_LITERAL:
			result = type_create( TYPE_STRING, 0, 0, 0 );
			break;
		case EXPR_INTEGER_LITERAL:
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_BOOLEAN_LITERAL:
			result = type_create( TYPE_BOOLEAN, 0, 0, 0 );
			break;
		case EXPR_ARRAY:
		{
			struct expr* ex = e->right;
			struct type* t = lt->subtype;
			t = lt->subtype;
			while( ex->right ){
				ex = ex->right;
				t = t->subtype;
				if( !t ){
					printf( "type error: trying to dereference array %s too many times\n", e->left->name );
					typecheck_failed++;
					result = type_copy( lt->subtype );
					break;
				}
			}
			result = type_copy( t );
			break;
		}
		case EXPR_INDEX:
			if( lt->kind != TYPE_INTEGER ){
				printf( "type error: array index must be integer, not " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ")\n" );
				typecheck_failed++;
			}
			result = type_create( TYPE_INTEGER, 0, 0, 0 );
			break;
		case EXPR_CALL:
			if( lt->kind != TYPE_FUNCTION ){
				printf( "type error: can only call functions, not " );
				type_print( lt );
				printf( " (" );
				expr_print( e->left );
				printf( ")\n" );
				typecheck_failed++;
				result = type_copy( lt );
			} else {
				if( !expr_list_equal_param_list( e->right, lt->params ) ){
					printf( "type error: provided parameters (" );
					expr_print( e->right );
					printf( ") do not match parameters of %s(", e->left->name );
					param_list_print( lt->params );
					printf( ")\n" );
					typecheck_failed++;
				}
				result = type_copy( lt->subtype );
			}
			break;
		case EXPR_PAREN:
			result = type_copy( rt );
			break;
		case EXPR_AR_INIT:{
			int count = 1;
			struct expr* el;
			if( e->left->kind == EXPR_LIST ){
				struct type* t;
				int match = 1;
				el = e->left;
				result = expr_typecheck( el->left );
				while( el && el->left && match ){
					t = expr_typecheck( el->left );
					if( !type_equal( result, t ) )
						match = 0;
					type_delete( t );
					el = el->right;
					count++;
				}
				t = expr_typecheck( el );
				if( !type_equal( result, t ) )
					match = 0;
				if( !match && !dup_message ){
					printf( "type error: inconsistent types in array initialization list " );
					expr_print( e );
					printf( "\n" );
					typecheck_failed++;
				}
			} else {
				result = type_copy( lt );
				// we already did internal typechecking on the right branch, we just need to get the count from and check the result type
				el = e->left;
				if( el && el->right ){
					type_delete( rt );
					dup_message = 1;
					rt = expr_typecheck( el->right );
					dup_message = 0;
					if( !type_equal( lt, rt ) ){
						printf( "type error: mismatched types in array list intialization- " );
						type_print( lt );
						printf( " and " );
						type_print( rt );
						printf( "\n" );
						typecheck_failed++;
					}
				}
				while( el && el->right ){
					count ++;
					el = el->right;
				}
			}
			result = type_create( TYPE_ARRAY, 0, result, expr_create_integer_literal( count ) );
			break;
		}
		case EXPR_LIST:
			break;
	}
	type_delete( lt );
	type_delete( rt );
	return result;
}

void expr_relevant_cmp( expr_t kind ){
	switch( kind ){
		case EXPR_LE: fprintf( f, "jle " ); break;
		case EXPR_GE: fprintf( f, "jge " ); break;
		case EXPR_LT: fprintf( f, "jl " ); break;
		case EXPR_GT: fprintf( f, "jg " ); break;
		default:
			printf( "I've made a terrible mistake\n" );
			codegen_fail();
			break;
	}
}

void expr_codegen( struct expr* e ){
	int i;
	struct type* t;
	struct expr* e1;
	struct expr* e2;
	if( !e ) return;
	switch( e->kind ){
		case EXPR_ASGN:
			expr_codegen( e->right );
			if( e->left->kind == EXPR_ARRAY ){
				e1 = e->left;
				if( !e1->right || e1->right->kind != EXPR_INDEX ){
					printf( "Assigning to entire array is unsupported!\n" );
					codegen_fail();
				}
				expr_codegen( e1->right );
				fprintf( f, "movq %s, %s(,%s,8)\n", register_name( e->right->reg ), symbol_code( e1->left->symbol ), register_name( e1->right->reg ) );
				e->reg = e->right->reg;
			} else {
				fprintf( f, "movq %s, %s\n", register_name( e->right->reg ), symbol_code( e->left->symbol ) );
				e->reg = e->right->reg;
			}
			break;
		case EXPR_OR:
			expr_codegen( e->left );
			expr_codegen( e->right );
			i = marker_get();
			marker_increment();
			marker_increment();
			fprintf( f, "cmpq $0, %s\n", register_name( e->left->reg ) );
			register_free( e->left->reg );
			fprintf( f, "jne .L%d\n", i );
			fprintf( f, "cmpq $0, %s\n", register_name( e->right->reg ) );
			fprintf( f, "jne .L%d\n", i );
			fprintf( f, "jmp .L%d\n", i+1 );
			marker_print( i );
			fprintf( f, "movq $1, %s\n", register_name( e->right->reg ) );
			marker_print( i+1 );
			e->reg = e->right->reg;
			break;
		case EXPR_AND:
			expr_codegen( e->left );
			expr_codegen( e->right );
			i = marker_get();
			marker_increment();
			marker_increment();
			fprintf( f, "cmpq $0, %s\n", register_name( e->left->reg ) );
			register_free( e->left->reg );
			fprintf( f, "je .L%d\n", i );
			fprintf( f, "cmpq $0, %s\n", register_name( e->right->reg ) );
			fprintf( f, "je .L%d\n", i );
			fprintf( f, "jmp .L%d\n", i+1 );
			marker_print( i );
			fprintf( f, "movq $0, %s\n", register_name( e->right->reg ) );
			marker_print( i+1 );
			e->reg = e->right->reg;
			break;
		case EXPR_LE:
		case EXPR_GE:
		case EXPR_LT:
		case EXPR_GT:
			expr_codegen( e->left );
			expr_codegen( e->right );
			fprintf( f, "cmpq %s, %s\n", register_name( e->right->reg ), register_name( e->left->reg ) );
			e->reg = e->left->reg;
			register_free( e->right->reg );
			i = marker_get();
			marker_increment();
			marker_increment();
			expr_relevant_cmp( e->kind );
			fprintf( f, ".L%d\n", i );
			fprintf( f, "movq $0, %s\n", register_name( e->reg ) );
			fprintf( f, "jmp .L%d\n", i+1 );
			marker_print( i );
			fprintf( f, "movq $1, %s\n", register_name( e->reg ) );
			marker_print( i+1 );
			break;
		case EXPR_EQ:
		case EXPR_NE:
			t = expr_typecheck( e->left );
			expr_codegen( e->left );
			expr_codegen( e->right );
			switch( t->kind ){
				case TYPE_BOOLEAN:
				case TYPE_CHARACTER:
				case TYPE_INTEGER:
					fprintf( f, "cmpq %s, %s\n", register_name( e->left->reg ), register_name( e->right->reg ) );
					break;
				case TYPE_STRING:
					before_fn_call();
					fprintf( f, "movq %s, %%rdi\n", register_name( e->left->reg ) );
					fprintf( f, "movq %s, %%rsi\n", register_name( e->right->reg ) );
					fprintf( f, "call strcmp\n" );
					fprintf( f, "testq %%rax, %%rax\n" );
					after_fn_call();
					break;
				default:
					printf( "Typecheck should have already failed!\n" );
					codegen_fail();
					break;
			}
			register_free( e->left->reg );
			i = marker_get();
			marker_increment();
			marker_increment();
			if( e->kind == EXPR_EQ ) fprintf( f, "je .L%d\n", i );
			else fprintf( f, "jne .L%d\n", i );
			fprintf( f, "movq $0, %s\n", register_name( e->right->reg ) );
			fprintf( f, "jmp .L%d\n", i+1 );
			marker_print( i );
			fprintf( f, "movq $1, %s\n", register_name( e->right->reg ) );
			marker_print( i+1 );
			e->reg = e->right->reg;
			break;
		case EXPR_ADD:
			expr_codegen( e->left );
			expr_codegen( e->right );
			fprintf( f, "addq %s, %s\n", register_name( e->left->reg ), register_name( e->right->reg ) );
			register_free( e->left->reg );
			e->reg = e->right->reg;
			break;
		case EXPR_SUB:
			expr_codegen( e->left );
			expr_codegen( e->right );
			fprintf( f, "subq %s, %s\n", register_name( e->right->reg ), register_name( e->left->reg ) );
			register_free( e->right->reg );
			e->reg = e->left->reg;
			break;
		case EXPR_MUL:
			expr_codegen( e->left );
			expr_codegen( e->right );
			fprintf( f, "pushq %%rdx\n" );
			fprintf( f, "movq %s, %%rax\n", register_name( e->left->reg ) );
			fprintf( f, "imulq %s\n", register_name( e->right->reg ) );
			fprintf( f, "movq %%rax, %s\n", register_name( e->left->reg ) );
			fprintf( f, "popq %%rdx\n" );
			register_free( e->right->reg );
			e->reg = e->left->reg;
			break;
		case EXPR_DIV:
			expr_codegen( e->left );
			expr_codegen( e->right );
			fprintf( f, "pushq %%rdx\n" );
			fprintf( f, "movq %s, %%rax\n", register_name( e->left->reg ) );
			fprintf( f, "cqo\n" );
			fprintf( f, "idivq %s\n", register_name( e->right->reg ) );
			fprintf( f, "movq %%rax, %s\n", register_name( e->left->reg ) );
			fprintf( f, "popq %%rdx\n" );
			register_free( e->right->reg );
			e->reg = e->left->reg;
			break;
		case EXPR_MOD:
			expr_codegen( e->left );
			expr_codegen( e->right );
			fprintf( f, "pushq %%rdx\n" );
			fprintf( f, "movq %s, %%rax\n", register_name( e->left->reg ) );
			fprintf( f, "cqo\n" );
			fprintf( f, "idivq %s\n", register_name( e->right->reg ) );
			fprintf( f, "movq %%rdx, %s\n", register_name( e->left->reg ) );
			fprintf( f, "popq %%rdx\n" );
			register_free( e->right->reg );
			e->reg = e->left->reg;
			break;
		case EXPR_EXP:
			expr_codegen( e->left );
			expr_codegen( e->right );
			fprintf( f, "pushq %%rdx\n" );
			i = marker_get();
			marker_increment();
			marker_increment();
			marker_increment();
			marker_increment();
			fprintf( f, "cmpq $0, %s\n", register_name( e->right->reg ) );
			fprintf( f, "je .L%d\n", i+1 );
			fprintf( f, "jl .L%d\n", i+2 );
			fprintf( f, "movq %s, %%rax\n", register_name( e->left->reg ) );
			marker_print( i );
			fprintf( f, "subq $1, %s\n", register_name( e->right->reg ) );
			fprintf( f, "cmpq $0, %s\n", register_name( e->right->reg ) );
			fprintf( f, "je .L%d\n", i+3 );
			fprintf( f, "imulq %s\n", register_name( e->left->reg ) );
			fprintf( f, "jmp .L%d\n", i );
			marker_print( i+1 );
			fprintf( f, "movq $1, %%rax\n" );
			fprintf( f, "jmp .L%d\n", i+3 );
			marker_print( i+2 );
			fprintf( f, "movq $0, %%rax\n" );
			marker_print( i+3 );
			fprintf( f, "movq %%rax, %s\n", register_name( e->left->reg ) );
			fprintf( f, "popq %%rdx\n" );
			register_free( e->right->reg );
			e->reg = e->left->reg;
			break;
		case EXPR_NOT:
			expr_codegen( e->right );
			fprintf( f, "notq %s\n", register_name( e->right->reg ) );
			e->reg = e->right->reg;
			break;
		case EXPR_NEG:
			expr_codegen( e->right );
			fprintf( f, "negq %s\n", register_name( e->right->reg ) );
			e->reg = e->right->reg;
			break;
		case EXPR_INCREMENT:
			expr_codegen( e->left );
			e->reg = register_alloc();
			fprintf( f, "movq %s, %s\n", register_name( e->left->reg ), register_name( e->reg ) );
			fprintf( f, "addq $1, %s\n", register_name( e->left->reg ) );
			fprintf( f, "movq %s, %s\n", register_name( e->left->reg ), symbol_code( e->left->symbol ) );
			register_free( e->left->reg );
			break;
		case EXPR_DECREMENT:
			expr_codegen( e->left );
			e->reg = register_alloc();
			fprintf( f, "movq %s, %s\n", register_name( e->left->reg ), register_name( e->reg ) );
			fprintf( f, "subq $1, %s\n", register_name( e->left->reg ) );
			fprintf( f, "movq %s, %s\n", register_name( e->left->reg ), symbol_code( e->left->symbol ) );
			register_free( e->left->reg );
			break;
		case EXPR_ID:
			e->reg = register_alloc();
			if( e->symbol && e->symbol->kind == SYMBOL_GLOBAL && e->symbol->type && e->symbol->type->kind == TYPE_STRING ){
				fprintf( f, "leaq %s, %s\n", symbol_code( e->symbol ), register_name( e->reg ) );
			}
			else {
				fprintf( f, "movq %s, %s\n", symbol_code( e->symbol ), register_name( e->reg ) );
			}
			break;
		case EXPR_CHARACTER_LITERAL:
		case EXPR_INTEGER_LITERAL:
		case EXPR_BOOLEAN_LITERAL:
			e->reg = register_alloc();
			fprintf( f, "movq $%d, %s\n", e->literal_value, register_name( e->reg ) );
			break;
		case EXPR_STRING_LITERAL:
			e->reg = register_alloc();
			fprintf( f, ".data\n" );
			i = marker_get();
			marker_increment();
			marker_print( i );
			fprintf( f, ".string %s\n", e->string_literal );
			fprintf( f, ".text\n" );
			fprintf( f, "leaq .L%d, %s\n", i, register_name( e->reg ) );
			break;
		case EXPR_ARRAY:
			expr_codegen( e->right );
			e->reg = register_alloc();
			fprintf( f, "movq %s(,%s,8), %s\n", symbol_code( e->left->symbol ), register_name( e->right->reg ), register_name( e->reg ) );
			register_free( e->right->reg );
			break;
		case EXPR_INDEX:
			if( e->right ){
				printf( "Multi-dimensional arrays not supported!\n" );
				codegen_fail();
			}
			expr_codegen( e->left );
			e->reg = e->left->reg;
			break;	
		case EXPR_CALL:
			i = 1;
			e1 = e->right;
			while( e1 ){
				if( i > 6 ){
					printf( "Exceeded maximum of 6 function parameters\n" );
					codegen_fail();
				}
				if( e1->left && e1->kind == EXPR_LIST )
					e2 = e1->left;
				else
					e2 = e1;
				expr_codegen( e2 );
				fprintf( f, "movq %s, %s\n", register_name( e2->reg ), param_reg( i ) );
				register_free( e2->reg );
				e1 = e1->right;
				i++;
			}
			before_fn_call();
			fprintf( f, "call %s\n", e->left->name );
			after_fn_call();
			e->reg = register_alloc();
			fprintf( f, "movq %%rax, %s\n", register_name( e->reg ) );
			break;
		case EXPR_PAREN:
			expr_codegen( e->right );
			e->reg = e->right->reg;
			break;
		case EXPR_AR_INIT:
		case EXPR_LIST:
			printf( "Should never reach here!\n" );
			codegen_fail();
			break;
	}
}
