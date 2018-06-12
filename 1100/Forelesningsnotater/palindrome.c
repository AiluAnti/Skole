#include <stdio.h>
#include <stdlib.h>



int ispalindrome(char *word)
{
    int len, left, right;


    // Determine word length
    len = 0; 
    while (word[len] != 0)
        len++;

    // Determine if palindrome by scanning character by character
    left = 0;
    right = len-1;
    while (left < right) {
        if (word[left] != word[right]) {
            return 0;
        }
        left = left + 1;
        right = right - 1;
    }

    return 1;
}




int main(void)
{
    int yes;

    yes = ispalindrome("redder");
    if (yes == 1) {
        printf("'redder' is a palindrome\n");
    } else {
        printf("'redder' is NOT a palindrome\n");
    }

    yes = ispalindrome("test");
    if (yes == 1) {
        printf("'test' is a palindrome\n");
    } else {
        printf("'test' is NOT a palindrome\n");
    }

    return 0;
}
