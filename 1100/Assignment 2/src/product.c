#include <stdio.h>
#include <stdlib.h>


int product(int x) {
	return x*5;
}

int main() {
	int i = product(5);
	printf("The product of 5 and 5 is %d\n",i);
}

