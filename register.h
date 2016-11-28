// register.h
// John Riordan

int used[16] = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int register_alloc();
void register_free(int r);
const char* register_name(int r);
