#include <stdio.h>
#include <stdlib.h>
// Declaring that assembly function is provided elsewhere
extern void asm_function();
 

int main(int argc, char **argv) {
    int a = 0;
    int b = 0;
    asm_function(&a, 100000, &b);

}
