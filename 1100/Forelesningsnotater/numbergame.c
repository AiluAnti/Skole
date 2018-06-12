
// You might need the two statements below to test the code
// on your own machine. What they mean will be covered in 
// later lectures.
#include <stdio.h>
#include <stdlib.h>


void mygame(void)
{
	int mynumber;
	int usernumber;
	
	// Seed random generator and pick a random number between 0..99
	srand(133);
	mynumber = rand() % 100;
	//printf("mynumber = %d\n", mynumber);

	printf("I have a number between 0..99, can you guess what it is?\n");
	while (1) {
		// Ask user to enter a guess
		printf("Enter a number:");
		scanf("%d", &usernumber);

		//printf("input number is %d\n", usernumber);

		// If guess was correct, game is over
		if (usernumber == mynumber) {
			printf("You found my number\n");
			return;
		}

		// Inform user whether our number is higher or lower than the guess
		if (mynumber > usernumber) {
			printf("My number is higher\n");
		} else {
			printf("My number is lower\n");
		}
	}
}

char readfromuser(void)
{
	char c;
	
	// Read user input until a 'h', 'l', or 'c' is typed
	while (1) {
		scanf("%c", &c);
		if (c == 'h' || c == 'l' || c == 'c')
			return c;
	}
}


void mygame2(void)
{
	char c;
	int myguess;
	int low;
	int high;

	printf("Think of a number between 0..99 and let me try to guess what it is\n");
	
	low = 0;
	high = 99;
	while (1) {
		// Guess the number in the middle of the search range
		myguess = low + (high-low)/2;
	
		// Ask if guess is correct or higher/lower
		printf("I'm guessing %d, is it correct? (h/l/c)\n", myguess);
		c = readfromuser();
		//printf("'%c' was read\n", c);

		if (c == 'c') {
			printf("I found it!\n");
			return;
		}
		
		// Update search range.
		if (c == 'h') {
			// If user number is higher than guess, ignore lower search range
			low = myguess + 1;
		} else {
			// If user number is lower than guess, ignore higher search range
			high = myguess - 1;
		}
	}
}


int main(void)
{
//	mygame();
	mygame2();
	return 0;
}
