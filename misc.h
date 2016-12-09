// misc.h
// John Riordan

#include <stdio.h>
#include <stdlib.h>

extern FILE* f;
extern int marker;

int marker_get();
void marker_increment();
void marker_print(int m);
void postamble();
void codegen_fail();
void before_fn_call();
void after_fn_call();
const char* param_reg();
