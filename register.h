// register.h
// John Riordan

#include <stdio.h>

extern FILE* f;
extern int used[];
extern int marker;

int register_alloc();
void register_free(int r);
const char* register_name(int r);
void marker_print();
