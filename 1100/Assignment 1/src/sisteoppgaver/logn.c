#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int mylog2(unsigned int n) {
	int j, i;
	printf("Enter the number you want to find the log of: \n");
	scanf("%d", &n);

	j = n;
	while (n > 1) {
		n /= 2;
		i++;
	}
	printf("Log(2) av %d er: %d", j, i);

}

int main() {
	mylog2(100);
}