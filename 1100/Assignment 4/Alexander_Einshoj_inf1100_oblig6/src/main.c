#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "SDL.h"
#include "drawline.h"
#include "triangle.h"
#include "list.h"
#include "teapot_data.h"
#include "sphere_data.h"
#include "object.h"


#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)


// Clear screen by filling it with 0
void ClearScreen(SDL_Surface *screen)
{
    SDL_Rect rect;
    
    // Define a rectangle covering the entire screen
    rect.x = 0;
    rect.y = 0;
    rect.w = screen->w;
    rect.h = screen->h;
    
    // And fill screen with 0
    SDL_FillRect(screen, &rect, 0);
}


// Add some speed to an object
/*void AccelerateObject(object_t *a, float boost, float maxspeed)
{
    float s;
    float news;
    
    // Calculate lenght of speed vector
    s = sqrtf(a->speedx * a->speedx + a->speedy * a->speedy);

    // Boost speed
    news = s * boost;
    if (news < 0.0)
        news = 0.0;
    if (news > maxspeed)
        news = maxspeed;    
    a->speedx *= news/s;
    a->speedy *= news/s;
}*/

// Function calculating how much each ball should rotate
void Rotate(object_t *obj)
{
    float rad = 500.0 * obj->scale;
    obj->rotation += 360.0*(obj->speedx / (2 * 3.141592 * rad));
}

// Own implementation for moving an object, as opposed to AccelerateObject() commented out above
void MoveObject(SDL_Surface *screen, object_t *obj)
{
    if ((obj->tx + (500.0 * obj->scale)) > (float)screen->w)
    {
        obj->tx = (float)screen->w - (500.0 * obj->scale);
        obj->speedx = -fabs(obj->speedx) * 0.8;
    }

    if ((obj->ty + (500.0 * obj->scale)) > (float)screen->h)
    {
        obj->ty = (float)screen->h - (500.0 * obj->scale);
        obj->speedy = -fabs(obj->speedy) * 0.7;
        obj->speedx *= 0.98;
    }

    if ((obj->tx - (500.0 * obj->scale)) < 0.0)
    {
        obj->tx =(500.0 * obj->scale);
        obj->speedx = fabs(obj->speedx) * 0.8;
    }

    if ((obj->ty - (500.0 * obj->scale)) < 0.0)
    {
        obj->ty =(500.0 * obj->scale);
        obj->speedy = fabs(obj->speedy) * 0.8;
    }


    obj->tx += obj->speedx;
    obj->ty += obj->speedy;
    Rotate(obj);
    if(obj->speedy < 10)
        obj->speedy += 0.3;
}


void BouncingBalls(SDL_Surface *screen, list_t *list)
{
    SDL_Event event;
    unsigned int start, currentTime, lastTime = 0;
    int done = 0, fps = 300;
    // Here the magic happens
    while (done == 0) 
    {
        ClearScreen(screen);
        start = SDL_GetTicks();
        while(list_size(list) < 50 && random() %100 > 95)
        {
            list_addlast(list, CreateObject(screen, sphere_model, SPHERE_NUMTRIANGLES));
        }
        list_iterator_t *iter = list_createiterator(list);
        object_t *ball;
        while ((ball = list_next(iter)) != NULL)
        {
            DrawObject(ball);
            MoveObject(screen, ball);
            currentTime = SDL_GetTicks();
            if(fabs(ball->speedx) <= 1.0)
            {
                if (currentTime > lastTime + 5000)
                {
                    list_remove(list, ball);
                    DestroyObject(ball);
                    lastTime = currentTime;
                }
            }
        }
        list_destroyiterator(iter);
        
        SDL_UpdateRect(screen, 0, 0, 0, 0);
        if(1000/fps> SDL_GetTicks() - start)
            SDL_Delay(1000/fps - (SDL_GetTicks() - start));
        while (SDL_PollEvent(&event)) 
        {
            switch (event.type) 
            {
            case SDL_QUIT:
                done = 1;
                break;  
            }
        }
    }   

}


// First function run in your program
int main(int argc, char **argv)
{
    int retval;
    SDL_Surface *screen;

    list_t *balls = list_create();



    // Initialize SDL   
    retval = SDL_Init(SDL_INIT_VIDEO);
    if (retval == -1) {
        printf("Unable to initialize SDL\n");
        exit(1);
    }
    
    // Create a 1024x768x32 window
    screen = SDL_SetVideoMode(1024, 768, 32, 0);     
    if (screen == NULL) {
        printf("Unable to get video surface: %s\n", SDL_GetError());    
        exit(1);
    }

    // Start bouncing some balls
    BouncingBalls(screen, balls);

    

    // Shut down SDL    
    SDL_Quit();

    // Wait a little bit jic something went wrong (so that printfs can be read)
    SDL_Delay(5000);
    
    return 0;
}
