// misc.c
// John Riordan

#include "misc.h"

int marker_get(){
	return marker;
}

void marker_increment(){
	marker++;
}

void marker_print( int m ){
	fprintf( f, ".L%d:\n", m );
}

void postamble(){
	fprintf( f, "popq %%r15\n" );
	fprintf( f, "popq %%r14\n" );
	fprintf( f, "popq %%r13\n" );
	fprintf( f, "popq %%r12\n" );
	fprintf( f, "popq %%rbx\n" );
	fprintf( f, "movq %%rbp, %%rsp\n" );
	fprintf( f, "popq %%rbp\n" );
	fprintf( f, "ret\n" );
}

void codegen_fail(){
	printf( "Code Generation Failed!\n" );
	fclose( f );
	exit( 1 );
}

void before_fn_call(){
	fprintf( f, "pushq %%r10\n" );
	fprintf( f, "pushq %%r11\n" );
}

void after_fn_call(){
	fprintf( f, "popq %%r11\n" );
	fprintf( f, "popq %%r10\n" );
}

const char* param_reg( int p ){
	switch( p ){
		case 1: return "%rdi";
		case 2: return "%rsi";
		case 3: return "%rdx";
		case 4: return "%rcx";
		case 5: return "%r8";
		case 6: return "%r9";
	}
	printf( "Invalid Parameter\n" );
	codegen_fail();
	return "";
}
