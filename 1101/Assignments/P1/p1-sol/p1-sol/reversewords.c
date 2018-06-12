/* Author: Steffen Viken Valvaag <steffenv@cs.uit.no> */
#include "list.h"
#include "common.h"

#include <string.h>

int main(int argc, char **argv)
{
    list_t *words;
    list_iter_t *it;
    int i;

    words = list_create((cmpfunc_t) strcmp);
    for (i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
	    if (f == NULL) {
	        perror("fopen");
	        return 1;
	    }
	    tokenize_file(f, words);
	    fclose(f);
    }
    
    while (list_size(words) > 0) {
	    char *word = list_poplast(words);
	    printf("%s\n", word);
    }
    list_destroy(words);

    return 0;
}
