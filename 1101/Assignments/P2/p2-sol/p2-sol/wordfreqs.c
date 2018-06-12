/* Author: Steffen Viken Valvaag <steffenv@cs.uit.no> */
#include "common.h"
#include "list.h"
#include "map.h"

#include <string.h>

#include <stdlib.h>
#include <stdio.h>

static map_t *freqs;

/*
 * Compares words by frequencies in descending order, i.e. the
 * most frequent word comes first.  Uses regular lexicographical
 * ordering as a tie-breaker for equally frequent words.
 * Assumes that the freqs map has been initialized.
 */
static int compare_words(void *worda, void *wordb)
{
	int *freqa = map_get(freqs, worda);
	int *freqb = map_get(freqs, wordb);
	
	if (*freqa < *freqb) {
		return 1;
	}
	else if (*freqa > *freqb) {
		return -1;
	}
	else {
		return strcmp(worda, wordb);
	}
}

static int *newint(int v)
{
    int *p = malloc(sizeof(v));
    if (p == NULL)
        fatal_error("out of memory");
    *p = v;
    return p;
}

int main(int argc, char **argv)
{
    list_t *words, *uniquewords;
    list_iter_t *it;
    int i;

    words = list_create(compare_strings);
    uniquewords = list_create(compare_words);
    freqs = map_create(compare_strings, hash_string);
    for (i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        if (f == NULL) {
            perror("fopen");
            return 1;
        }
        tokenize_file(f, words);
        fclose(f);
    }

    it = list_createiter(words);
    while (list_hasnext(it)) {
        char *word = list_next(it);
        if (map_haskey(freqs, word)) {
            int *freq = map_get(freqs, word);
            *freq += 1;            
        }
        else {
            list_addlast(uniquewords, word);
            map_put(freqs, word, newint(1));
        }
    }
    list_destroyiter(it);
    
    list_sort(uniquewords);
    it = list_createiter(uniquewords);
    while (list_hasnext(it)) {
        char *word = list_next(it);
        int *freq = map_get(freqs, word);
        printf("%s %d\n", word, *freq);
    }
    list_destroyiter(it);
    
    list_destroy(words);
    list_destroy(uniquewords);
    map_destroy(freqs);
    
    // Note: Leaks malloc()ed frequencies
    return 0;
}
