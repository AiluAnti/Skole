/* Author: Steffen Viken Valvaag <steffenv@cs.uit.no> */
#include "common.h"
#include "list.h"

void fatal_error(char *msg)
{
    fprintf(stderr, "fatal error: %s\n", msg);
    exit(1);
}

void tokenize_file(FILE *file, list_t *list)
{
    char *word;
    char buf[101];
    buf[100] = 0;
    
    while (!feof(file)) {
		/* Skip non-letters */
		fscanf(file, "%*[^a-zA-Z0-9'_]");
		/* Scan up to 100 letters */
		if (fscanf(file, "%100[a-zA-Z0-9'_]", buf) == 1) {
	    	word = strdup(buf);
	    	if (word == NULL)
				fatal_error("out of memory");
	    	list_addlast(list, word);
		}
    }
}
