/*
 * word_example.c
 *
 *  Created on: Oct 29, 2014
 *      Author: harald
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct word word_t;

struct word{
	char *word;
	word_t *next_word;
};

word_t *new_word(char *string, word_t *prev_word)
{
	word_t *word;
	char *allocated_space_for_string = malloc(sizeof(char) * 30);
	strcpy(allocated_space_for_string, string);

	word = malloc(sizeof(word_t));
	word->word = allocated_space_for_string;
	word->next_word = NULL;
	if(prev_word != NULL)
		prev_word->next_word = word;

	return word;
}

int main (int argc, char *argv[])
{
	word_t *my_new_word1, *my_new_word2, *my_new_word3, *current, *tmp;
	my_new_word1 = new_word("hei", NULL);
	my_new_word2 = new_word("på", my_new_word1);
	my_new_word3 = new_word("deg", my_new_word2);

	for(current = my_new_word1; current != NULL; current = current->next_word)
	{
		printf("%s ", current->word);
	}
	printf("\n");

	current = my_new_word1;
	while(current)
	{
		tmp = current;
		current = current->next_word;
		free(tmp->word);
		free(tmp);
	}
}


/*


int main (int argc, char *argv[])
{
	char word1[30];
	char word2[30];
	char word3[30];

	word_t *current;

	strcpy(word1, "Hei");
	word_t my_word1, my_word2, my_word3;
	my_word1.word = word1;

	strcpy(word2, "på");
	my_word2.word = word2;
	my_word1.next_word = &my_word2;

	strcpy(word3, "deg");
	my_word3.word = word3;
	my_word2.next_word = &my_word3;

	my_word3.next_word = NULL;

	for(current = &my_word1; current != NULL; current = current->next_word)
	{
		printf("%s ", current->word);
	}
	printf("\n");

}
*/
