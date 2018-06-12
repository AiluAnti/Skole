#include <stdio.h>
#include <stdlib.h>


//Lager en funksjon med antall linjer som parameter
void mytriangles(int numlines) {
	int x = 0;
	int i = 0;
		//for-loop som finner ut av antall linjer
		for(x=1;x<=numlines;x++) 
		{ 
			//printer ut riktig antall stjerner i forhold til linje den er på
			for(i=1;i<=x;i++) {
				printf("*");
			}
			printf("\n");
		}
}


int main() {
	mytriangles(35);
}