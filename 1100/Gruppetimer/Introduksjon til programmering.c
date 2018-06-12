
/*
dette blir grått
0b 0000 0000
char -> 8bit / 1byte
short -> 16bit / 2byte
int -> 32bit / 4byte
long long -> 64bit / 8byte
float -> 32bit / 4byte
double -> 64bit / 8byte
*/
#include <stdio.h>

// type_på_returverdi navn_på_funksjon(type_på_parameter navn_på_parameter)
void teller(void)
{
	int i;
	// while
	i = 0;
	while(i < 10)
	{
		printf("i var %d.\n", i);
		i = i + 1;
	}
}

int min_funksjon(int mitt_tall)
{
	return mitt_tall + 7;
}

int main(void)
{
	int x;
	int i;
	int j;
	float f;
	x = 10;
	f = 1.4;

	for(j = 0; j < 5; j = j + 1)
	{
		x = min_funksjon(x);	
		printf("x er lik: %d\n", x);
	}
	
	// for
	//for (initialization; condition; for_every_iteration)

	// i = i + 1; i += 1; i++;

	for(i = 0; i < 10; i = i + 1)
	{
		printf("i var %d. x var lik %d.\n", i, x);
	}

	if(x == 10)
	{
		printf("Hei! ");
		printf("Tallet er 10\n");
		x = 20;
	}

	if(x == 20)
	{
		printf("\nHello!\n");
	}	
	else
	{
		printf("heisann!\n");
	}
}


	

