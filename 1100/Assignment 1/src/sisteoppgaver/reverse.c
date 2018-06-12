#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void reversewords()
{
   int num, newNum = 0;
 
   printf("Enter a number to reverse\n");	
   scanf("%d", &num);						//Look for number input
 
   while (num != 0)							//While num is not 0, do the following
   {				
      newNum *= 10;							
      newNum += num%10;						//Use modulo to find out the rightmost digit
      num    /= 10;
   }
 
   printf("The reverse of entered number is = %d\n", newNum);

}
int main() {
	reversewords();
}