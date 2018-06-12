#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

float GradeAvg()
{
	int a, b, c, d, e, f;
	int aVal, bVal, cVal, dVal, eVal, numGrades;
	float average;

	aVal = 6;
	bVal = 5;
	cVal = 4;
	dVal = 3;
	eVal = 2;


	printf("\nHow many got the grade A: ");
	scanf("%d", &a);
	printf("\nHow many got the grade B: ");
	scanf("%d", &b);
	printf("\nHow many got the grade C: ");
	scanf("%d", &c);
	printf("\nHow many got the grade D: ");
	scanf("%d", &d);
	printf("\nHow many got the grade E: ");
	scanf("%d", &e);
	printf("\nHow many got the grade F: ");
	scanf("%d", &f);

	numGrades = a + b + c + d + e + f;

	a = a * aVal;
	b = b * bVal;
	c = c * cVal;
	d = d * dVal;
	e = e * eVal;

	average = (float)(a + b + c + d + e + f) / numGrades;

	return roundf(average * 100.0) / 100.0;
}

void StoreFile(float value)
{
	char toFile[100];
	printf("\nDo you want the result to be stored in a file?(y or n)");
	scanf("%s", toFile);
	if (strcmp(toFile, "y") == 0 || strcmp(toFile, "Y") == 0)
	{
		printf("\nWhat would you like the file to be called? : ");
		scanf("%s", toFile);
		snprintf(toFile, sizeof(toFile), "%s.txt", toFile);
		if (access(toFile, F_OK) != -1)
		{
			printf("\nFile already exists");
			FILE *f = fopen(toFile,"a");
			if (f == NULL)
			{
				printf("\nError opening file!");
				exit(1);
			}
		}
		else
		{
			printf("\nFile doesn't exist");
			FILE *f = fopen(toFile,"w");
			if (f == NULL)
			{
				printf("\nError opening file!");
				exit(1);
			}
		}
		fprintf(f, "Average grade is: %.2f\n", value);
		fclose(f);
	}
	else
	{
		printf("\nThe result will not be written to file");
	}
}

int herb()
{
	int type;
	printf("\nWhat herb are you farming?
			\n 1 = Guam
			\n 2 = Marrentill
			\n 3 = Harralander
			\n 4 = Ranarr
			\n 5 = Toadflax
			\n 6 = Irit leaf
			\n 7 = Avantoe
			\n 8 = Kwuarm
			\n 9 = Snapdragon
			\n 10 = Cadantine
			\n 11 = Lantadyme
			\n 12 = Dwarf weed
			\n 13 = Torstol");
	scanf("%d", type);

}



int main()
{

	StoreFile(GradeAvg());

	return 0;
}


