// register.h
// John Riordan

#include <stdio.h>

extern int used[];

int register_alloc();
void register_free(int r);
const char* register_name(int r);
