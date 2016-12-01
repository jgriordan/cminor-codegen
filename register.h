// register.h
// John Riordan

#include <stdio.h>

int used[7] = {0,0,0,0,0,0,0};

int register_alloc();
void register_free(int r);
const char* register_name(int r);
