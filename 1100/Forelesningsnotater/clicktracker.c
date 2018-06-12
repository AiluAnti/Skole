#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"

typedef struct click_t click_t;
typedef struct cookie_t cookie_t;

struct cookie_t {
        char            *cookie;
        int             numclicks;
};

struct click_t {
//        click_t         *next;
        char            *uri;
        int             numclicks;
        list_t          *cookielist;
};


list_t *clicklist;

static void terminate(char *message)
{
    printf("%s\n", message);
    exit(-1);
}

static void CookieRegister(click_t *click, char *cookie)
{
    cookie_t            *current;
    list_iterator_t     *iter;
        
    // Search for cookie
    iter = list_createiterator(click->cookielist);
    current = list_next(iter);
    while (current != NULL) {
        if (strcmp(cookie, current->cookie) == 0)
            break;
        current = list_next(iter);
    }

    // Create new cookie if not found
    if (current == NULL) {
        current = malloc(sizeof(cookie_t));
        if (current == NULL)
            terminate("Out of memory");
        current->cookie = strdup(cookie);
        list_addfirst(click->cookielist, current);
    }

    // Update cookie click counter
    current->numclicks += 1;
}



static void PrintClicks(void)
{
    click_t         *click;
    cookie_t        *cookie;
    list_iterator_t *listiter;
    list_iterator_t *cookieiter;

    listiter = list_createiterator(clicklist);
    click = list_next(listiter);
    while (click != NULL) {
        printf("URI : %s\n", click->uri);
        cookieiter = list_createiterator(click->cookielist);
        cookie = list_next(cookieiter);
        while (cookie != NULL) {
            printf("  Cookie: %s\n", cookie->cookie);
            cookie = list_next(cookieiter);
        }
        click = list_next(listiter);
    }
}




int ClickNumCookie(char *cookie)
{
    int             clicknum;
    click_t         *click;
    cookie_t        *currentcookie;
    list_iterator_t *listiter;
    list_iterator_t *cookieiter;

    clicknum = 0;
    listiter = list_createiterator(clicklist);
    click = list_next(listiter);
    while (click != NULL) {
        cookieiter = list_createiterator(click->cookielist);
        currentcookie = list_next(cookieiter);
        while (currentcookie != NULL) {
            if (strcmp(cookie, currentcookie->cookie) == 0) {
                clicknum += currentcookie->numclicks;
            }
            currentcookie = list_next(cookieiter);
        }
        click = list_next(listiter);
    }
    return clicknum;
}


int ClickNumURI(char *uri)
{
    click_t             *current;
    list_iterator_t     *iter;
        
    // Search for uri
    iter = list_createiterator(clicklist);
    current = list_next(iter);
    while (current != NULL) {
        if (strcmp(uri, current->uri) == 0)
            break;
        current = list_next(iter);
    }

    if (current != NULL)
        return current->numclicks;
    else
        return -1;
}



void ClickRegister(char *uri, char *cookie)
{
    click_t             *current;
    list_iterator_t     *iter;
        
    // Search for uri
    iter = list_createiterator(clicklist);
    current = list_next(iter);
    while (current != NULL) {
        if (strcmp(uri, current->uri) == 0)
            break;
        current = list_next(iter);
    }

    if (current == NULL) {
        // create new uri entry
        current = malloc(sizeof(click_t));
        if (current == NULL)
            terminate("out of memory");
        current->uri = strdup(uri);
        current->cookielist = list_create();

        list_addfirst(clicklist, current);
     }
        
     // Update click counter
     current->numclicks = 1;
 
     // Register cookie
     CookieRegister(current, cookie);
}


int main(int argc, char **argv)
{
    // Create clicklist
    clicklist = list_create();


    ClickRegister("www.vg.no", "my own cookie");
    ClickRegister("www.dagbladet.no", "my own cookie2");
    ClickRegister("www.aftenposten.no", "my own cookie3");
    ClickRegister("www.aftenposten.no", "my own cookie4");
        
    PrintClicks();

}
