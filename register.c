// register.c
// John Riordan

#include "register.h"

int register_alloc(){
	int i;
	for( i=0; i<7; i++ ){
		if( !used[i] ){
			used[i] = 1;
			return i+1;
		}
	}

	printf( "failure imminent: no available scratch registers" );
	return -1;
}

void register_free( int r ){
	if( r )	used[r-1] = 0;
}

const char* register_name( int r ){
	switch( r ){
		case 1: return "%rbx";
		case 2: return "%r10";
		case 3:	return "%r11";
		case 4:	return "%r12";
		case 5:	return "%r13";
		case 6:	return "%r14";
		case 7:	return "%r15";
	}
	printf( "invalid register\n" );
	return 0;
}
