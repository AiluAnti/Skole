#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include "list.h"

typedef struct fileinfo fileinfo_t;
typedef struct word word_t;

struct word {
    char *wordstring;
    list_t *filelist;
};

struct fileinfo {
    char *filenamestring;
};


// Print error message and exit
void fatal_error(char *msg)
{
    printf(msg);
    exit(1);
}


// Print word file list
void filelist_print(list_t *filelist)
{
    list_iterator_t *iterator;
    fileinfo_t *fileinfo;

    iterator = list_createiterator(filelist);
    fileinfo = list_next(iterator);
    printf("[ ");
    while (fileinfo != NULL) {
            printf("'%s' ", fileinfo->filenamestring);
            fileinfo = list_next(iterator);
    }
    printf("]\n");
 
    list_destroyiterator(iterator);
}


// Print entire wordlist
void wordlist_print(list_t *wordlist)
{
    word_t *word;
    fileinfo_t *fileinfo;
    list_iterator_t *iterator;

    // Create list iterator
    iterator = list_createiterator(wordlist);

    // Check if word is in wordlist
    word = list_next(iterator);
    while (word != NULL) {
        printf("Word: '%s'", word->wordstring);
        filelist_print(word->filelist);
        word = list_next(iterator);
    }

    list_destroyiterator(iterator);
}


// Read input file
char *readfile(size_t *filesize, char *filename)
{
    int retval;
    size_t numobjects;
    struct stat sb;
    char *filedata;
    FILE *f;

    // Initialize
    filedata = NULL;
    f = NULL;
    
    // Get file size
    retval = stat(filename, &sb);
    if (retval < 0)
        goto error;
    
    // Allocate memory for file contents
    filedata = malloc(sb.st_size);
    if (filedata == NULL)
        goto error;

    // Open input file
    f = fopen(filename, "r");
    if (f == NULL)
        goto error;

    // Read file contents
    numobjects = fread(filedata, 1, sb.st_size, f);
    if (numobjects != sb.st_size)
        goto error;

    // Close file
    fclose(f);

    // Return
    *filesize = sb.st_size;
    return filedata;
 error:
    if (filedata != NULL)
        free(filedata);
    if (f != NULL)
        fclose(f);
    return NULL;
}



// Copy string
char *dupstring(char *string, size_t size)
{
    char *wordstring;
    
    wordstring = malloc(size+1);
    if (wordstring == NULL)
        goto error;
    strncpy(wordstring, string, size);
    wordstring[size] = 0;

    return wordstring;
 error:
    return NULL;
}


// Insert filename into filelist
int filelist_insert(list_t *filelist, char *filenamestring)
{
    char *filename;
    fileinfo_t *fileinfo;
    list_iterator_t *iterator;

    // Create list iterator
    iterator = list_createiterator(filelist);

    // Check if filename is in filelist
    fileinfo = list_next(iterator);
    while (fileinfo != NULL) {
        if (strcmp(fileinfo->filenamestring, filenamestring) == 0) {
            break;
        }
        fileinfo = list_next(iterator);
    }
    
    // Create new file list entry if filename was not found
    if (fileinfo == NULL) {
        fileinfo = (fileinfo_t*)malloc(sizeof(fileinfo_t));
        if (fileinfo == NULL)
            goto error;
        fileinfo->filenamestring = strdup(filenamestring);
        list_addfirst(filelist, fileinfo);
    }

    list_destroyiterator(iterator);

    return 0;
 error:
    return -1;
}


// Insert word into wordlist
int wordlist_insert(list_t *wordlist, char *wordstring, char *filename)
{
    word_t *word;
    list_iterator_t *iterator;

    // Create list iterator
    iterator = list_createiterator(wordlist);

    // Check if word is in wordlist
    word = list_next(iterator);
    while (word != NULL) {
        if (strcmp(word->wordstring, wordstring) == 0) {
            filelist_insert(word->filelist, filename);
            break;
        }
        word = list_next(iterator);
    }

    // Insert word
    if (word == NULL) {
        word = malloc(sizeof(word_t));
        if (word == NULL)
            goto error;
        word->wordstring = wordstring;
        word->filelist   = list_create();
        if (word->filelist == NULL)
            goto error;
        list_addfirst(wordlist, word);
    }
    list_destroyiterator(iterator);

    // Insert filename
    filelist_insert(word->filelist, filename);

    return 0;
 error:
    return -1;

}


// Parse buffer and extract words
void wordlist_insertword(list_t *wordlist, char *filedata, 
                         size_t filesize, char *filename)
{
    int retval, idx, wordidx;
    char *wordstring;

    idx = 0;
    while (idx < filesize) {
        // skip whitespace
        while (idx < filesize && isspace(filedata[idx]))
               idx++;
        
        // Determine size of word
        wordidx = idx;
        while (idx < filesize && !isspace(filedata[idx]))
            idx++;

        // Skip empty word
        if ((wordidx-idx) == 0)
            continue;
        
        // Extract word
        wordstring = dupstring(&filedata[wordidx], idx-wordidx);
        if (wordstring == NULL)
            goto error;
        
        // Insert word into wordlist
        retval = wordlist_insert(wordlist, wordstring, filename);
        if (retval < 0)
            goto error;
    }
    
    return;
 error:
    return;
}


// Locate a given word and print contents of file list
void wordlist_lookup(list_t *wordlist, char *wordstring)
{
    word_t *word;
    list_iterator_t *iterator;

    iterator = list_createiterator(wordlist);
    
    word = list_next(iterator);
    while (word != NULL) {
        if (strcmp(word->wordstring, wordstring) == 0) {
            printf("Word '%s' occurs in file(s):\n", wordstring);
            filelist_print(word->filelist);
        }
        word = list_next(iterator);
    }

    list_destroyiterator(iterator);
}


void usage(char *execname)
{
    printf("Usage: %s <inputfile>\n", execname);
    exit(1);
}


int main(int argc, char **argv)
{
    int i;
    size_t filesize;
    char *filedata;
    char *filename;
    list_t *wordlist;
    
    if (argc < 2)
        usage(argv[0]);

    wordlist = list_create();
    if (wordlist == NULL)
        fatal_error("Unable to create wordlist\n");

    for (i = 1; i < argc; i++) {
        filedata = readfile(&filesize, argv[i]);
        if (filedata == NULL)
            fatal_error("Unable to read input file\n");
        wordlist_insertword(wordlist, filedata, filesize, argv[i]);
    }

    //    wordlist_print(wordlist);
    wordlist_lookup(wordlist, "word");

    return 0;
}
