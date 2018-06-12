#include <stdio.h>
#include <stdlib.h>
//her trenger man bare en simpel for-loop og formelen for å regne fra celsius til Fahrenheit 
void degrees() {
int x = 0;
int y = 0;
	for(x=1;x<=50;x++) {
			y=x *9/5 + 32;
			printf("%d degrees Celcius = %d degrees Fahrenheit\n",x,y);
	}
}

int main() {
	degrees();
}