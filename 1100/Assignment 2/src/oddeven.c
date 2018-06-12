#include <stdio.h>
#include <stdlib.h>

void mynumbers() {
	int x=0;
	//loop som går gjennom tallene fra 1-50
	for(x=1;x<=50;x++) {
		//hvis et tall mod2 = 0, printes et av alternativene under ut, avhengig av om tallet er delelig på å 5 eller ikke.
		if(x%2==0) {
			if(x%5==0) {
				printf("%d is even and 5 is a prime factor\n", x);
			} 
			else {
				printf("%d is even and 5 is not a prime factor\n", x);
			}
		}
		// her er samme teknikk som for partall brukt, med nested if's
		else {
			if(x%5==0) {
				printf("%d is odd and 5 is a prime factor\n", x);
			} 
			else {
				printf("%d is odd and 5 is not a prime factor\n", x);
			}
		}
	}	
}

//her calles funksjonen
int main() {
	mynumbers();
}