/*
 * wordexample1.c
 *
 *  Created on: Oct 22, 2013
 *      Author: harald
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSTRINGSIZE 90

typedef struct word word_t;

struct word
{
	char *wordString;
	word_t *nextWord;
};

word_t *addWord(char *buf, int stringSize)
{
	char *newWordString;
	word_t *new;
	buf[stringSize++] = '\0';

	new = (word_t *) malloc(sizeof(word_t));
	if(new == NULL)
		goto error;
	new->nextWord = NULL;

	newWordString = (char *) malloc(sizeof(char)* stringSize);
	if(newWordString == NULL)
		goto error;

	strcpy(newWordString, buf);
	new->wordString = newWordString;

	return new;

	error:
		if(new != NULL)
			free(new);
		return NULL;
}

void printString(word_t *word)
{
	int i;

	i = 0;
	while(word != NULL)
	{
		if(word->wordString != NULL)
			printf("word after iteration %d is: %s\n", i, word->wordString);
		else
			printf("There was no word on iteration %d, something went terribly bad.\n", i);
		i++;
		word = word->nextWord;
	}
}

void freeWords(word_t *word)
{
	word_t *tmp;
	while(word != NULL)
	{
		tmp = word;
		if(word->wordString)
			free(word->wordString);
		word = word->nextWord;
		free(tmp);
	}
}

int main(int argc, char *argv[])
{
	int stringSize, res;
	char c, buf[MAXSTRINGSIZE];
	word_t *start, *current;
	start = current = NULL;

	stringSize = 0;
	while((c = getc(stdin)) != EOF)
	{
// 		Checking if end of word token. Space, new line or return character
		if((c != ' ') && (c != '\n') && (c != '\r'))
		{
//			Checking if the string is within it's boundaries, if so keep adding
			if(stringSize < MAXSTRINGSIZE)
				buf[stringSize++] = c;
			else
			{
				printf("Max string size is: %d, cannot have longer strings than that", MAXSTRINGSIZE - 1);
				goto end;
			}
		}
		else
		{
			if(start == NULL)
				start = current = addWord(buf, stringSize);
			else
			{
				current->nextWord = addWord(buf, stringSize);
				current = current->nextWord;
			}

//			Did storing word work out as intended?
			if(current == NULL)
			{
				printf("Current uninitilized at: %d, something went terribly bad.\n", __LINE__);
				goto end;
			}
//			Resetting stringSize
			stringSize = 0;
		}
	}

	printString(start);

	end:
		freeWords(start);
		return EXIT_SUCCESS;
}
