#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

/* Callback function that's called once each result row*/
int callback(void *arb, int argc, char **argv, char **colN){
  int i;
  printf("{");
  for(i = 0; i < argc; i++){
    if (i == argc - 1) {
      printf("\"%s\" : \"%s\" ", colN[i], argv[i] ? argv[i] : "NULL");
      printf("},\n");
    }
    else
      printf("\"%s\" : \"%s\", ", colN[i], argv[i] ? argv[i] : "NULL");
  }
  return 0;
}

int main(int argc, char* argv[])
{

    sqlite3 *db;
    //char *zErrMsg = 0;
    int rc, i;
    char *quit = ".quit";
    char *help = ".help";
    char c;

    /*Open database */
    rc = sqlite3_open(argv[1], &db);
    if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
    }

  /* Main loop */
  while (1) {
    i = 0;
    int size = 10;
    char * buf = malloc(size * sizeof(char *));

    printf("sql-json>");

    /* Fetch SQL statement */
    while((c = fgetc(stdin)) != ';') {
      if (c == '\n') {
        if (buf[0] != '.') {
          continue;
        }
        else {
          break;
        }
      }
      buf[i] = c;
      i++;
      if ( i >= size) {
        buf = realloc(buf, size * (sizeof(char *)) + (sizeof(char *)));
        size += sizeof(char);
      }
    }


    buf[i] = '\0';
    /* user commands */
    if (strcmp(buf, quit) == 0) {
      free(buf);
      sqlite3_close(db);
      return 0;
    }
    else if (strcmp(buf, help) == 0) {
        free(buf);
        printf(".quit - exits shell \n Each command needs to end with \";\" in order to be considered valid\n");
    }
    else {
      /* Execute Sql Statement */
      printf("[");
      rc  = sqlite3_exec(db, buf, callback, NULL, NULL);
      printf("]");
      if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error!\n");
      }
      else {
        fprintf(stdout, "Operation done successfully\n");
      }
      free(buf);
      buf = NULL;
    }
  }
  return 0;
}
