// register.c
// John Riordan

int register_alloc(){
	int i;
	for( i=0; i<16; i++ ){
		if( !used[i] ){
			used[i] = 1;
			return i;
		}
	}

	printf( "failure imminent: no available registers" );
	return -1;
}

void register_free( int r ){
	used[r] = 0;
}

const char* register_name( int r ){
	switch( r ){
		case 0:
			return "%rax";
		case 1:
			return "%rbx";
	}
}
