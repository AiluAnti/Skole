#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "SDL.h"
#include "drawline.h"

// Read pixel x,y on the screen
unsigned int GetPixel(SDL_Surface *screen, int x, int y)
{
    unsigned int *bufp;

    if (x >= screen->w ||  x < 0 ||
        y >=screen->h || y < 0) {
         printf("Accessing pixel outside of screen, check translation or scale\n");
         return 0;
    }

    // Set pixel
    bufp = (unsigned int*)screen->pixels + y*screen->pitch/4 + x;
    return *bufp;
}

// Set pixel x,y on the screen
void SetPixel(SDL_Surface *screen, int x, int y, unsigned int color)
{
    unsigned int *bufp;

    if (x >= screen->w ||  x < 0 ||
        y >=screen->h || y < 0) {
         printf("Plotting pixel outside of screen, check translation or scale\n");
         return;
    }

    // Set pixel
    bufp = (unsigned int*)screen->pixels + y*screen->pitch/4 + x;
    *bufp = color; 	
}

// Draw a line on the screen from x1,y1 to x2,y2
void DrawLine(SDL_Surface *screen, int x1, int y1, int x2, int y2, unsigned int color)
{
    int fraction;
    int x, dx, stepx;
    int y, dy, stepy;
    
    
    // The below code implements the classic Bresenham algorithm
    
    dx = x2 - x1;
    dy  = y2 - y1;
    	
    if (dy < 0) {
        dy = -dy;
        stepy = -1;
    } else {
        stepy = 1;	
    }
    
    if (dx < 0) {
        dx = -dx;
        stepx = -1;	
    } else {
        stepx = 1;
    }
    
    dy = dy*2;
    dx = dx*2;
    x = x1;
    y = y1;
    SetPixel(screen, x, y, color);
    if (dx > dy) {
        fraction = dy - (dx/2);
        while (x != x2) {
            if (fraction >= 0) {
                y = y + stepy;
                fraction = fraction - dx;	
            }
            x = x + stepx;
            fraction = fraction + dy;
            SetPixel(screen, x, y, color);
        }	
    } else {
        fraction = dx - (dy/2);
        while (y != y2) {
            if (fraction >= 0) {
                x = x + stepx;
                fraction = fraction - dy;	
            }
            y = y + stepy;
            fraction = fraction + dx;
            SetPixel(screen, x, y, color);
        }	
    }
}

