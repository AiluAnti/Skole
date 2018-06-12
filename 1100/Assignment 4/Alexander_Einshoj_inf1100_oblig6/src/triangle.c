#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "SDL.h"
#include "triangle.h"
#include "drawline.h"

#define MIN(x,y)    (x < y ? x : y)
#define MIN3(x,y,z) MIN(MIN(x,y), MIN(x,z))
#define MAX(x,y)    (x > y ? x : y)
#define MAX3(x,y,z) MAX(MAX(x,y), MAX(x,z))

#define TRIANGLE_PENCOLOR   0xBBBB0000

int doConsoleDump;

void PrintTriangle(triangle_t *triangle, char *msg)
{
    printf("%s: %d,%d - %d,%d - %d,%d\n",
        msg,
        triangle->x1, triangle->y1, 
        triangle->x2, triangle->y2, 
        triangle->x3, triangle->y3);
}

int SanityCheckTriangle(SDL_Surface *screen, triangle_t *triangle)
{
    if (triangle->sx1 < 0 || triangle->sx1 >= screen->w ||
        triangle->sx2 < 0 || triangle->sx2 >= screen->w ||
        triangle->sx3 < 0 || triangle->sx3 >= screen->w ||
        triangle->sy1 < 0 || triangle->sy1 >= screen->h ||
        triangle->sy2 < 0 || triangle->sy2 >= screen->h ||
        triangle->sy3 < 0 || triangle->sy3 >= screen->h) {
        return 0;
    } else {
        return 1;
    }
}


// Scale triangle
void ScaleTriangle(triangle_t *triangle)
{
    // Scale triangle    
    triangle->sx1 = (int)((float)triangle->x1*triangle->scale);
    triangle->sx2 = (int)((float)triangle->x2*triangle->scale);
    triangle->sx3 = (int)((float)triangle->x3*triangle->scale);
    triangle->sy1 = (int)((float)triangle->y1*triangle->scale);
    triangle->sy2 = (int)((float)triangle->y2*triangle->scale);
    triangle->sy3 = (int)((float)triangle->y3*triangle->scale);
}


// Move triangle to its screen position
void TranslateTriangle(triangle_t *triangle)
{
    triangle->sx1 += triangle->tx;
    triangle->sx2 += triangle->tx;
    triangle->sx3 += triangle->tx;
    
    triangle->sy1 += triangle->ty;
    triangle->sy2 += triangle->ty;
    triangle->sy3 += triangle->ty;
}


// Calculate triangle bounding box
void CalculateTriangleBoundingBox(triangle_t *triangle)
{
    // Calculate upper left corner of bounding box
    triangle->bx = MIN3(triangle->sx1, triangle->sx2, triangle->sx3);
    triangle->by = MIN3(triangle->sy1, triangle->sy2, triangle->sy3);
    
    // Calculate width and height of bounding box
    triangle->bw = MAX3(triangle->sx1, triangle->sx2, triangle->sx3) - triangle->bx + 1;
    triangle->bh = MAX3(triangle->sy1, triangle->sy2, triangle->sy3) - triangle->by + 1;
}


// Fill triangle with a color
void FillTriangle(SDL_Surface *screen, triangle_t *triangle)
{
    int x, y;
    int startfill, stopfill;

    // Scan triangle bounding box line by line, starting from upper left corner.
    // For each line, set pixels in between first and last pixel set.
    for (y = 0; y < triangle->bh; y++) {
        // Determine first and last pixel set on line.
        // Only consider pixels drawn with TRIANGLE_DRAW_PEN color
        startfill = stopfill = -1;
        for (x = 0; x < triangle->bw; x++) {
              if (GetPixel(screen, triangle->bx + x, triangle->by + y)== TRIANGLE_PENCOLOR) {
                if (startfill == -1)
                    startfill = x;
                else
                    stopfill = x;
            }            
        }
       
        // Set pixels between startfill and stopfill. 
        if (startfill != -1) {
            if (stopfill == -1)             // might be only a single pixel set
                stopfill = startfill;
            while (startfill <= stopfill) {
                SetPixel(screen, triangle->bx + startfill, triangle->by + y, triangle->fillcolor);
                startfill++;
            }
        }
    }
}


// Rotate triangle
void RotateTriangle(triangle_t *triangle)
{
    static float rotation = 0;
    static float sinr = 0, cosr = 1;
    float sx1, sx2, sx3;
    float sy1, sy2, sy3;

    // Cache sin and cos calculations to reduce overhead
    // INFO: Local variables that are declared static persist across function calls.
    if (triangle->rotation != rotation) {
        rotation = triangle->rotation;
        sinr = sinf(rotation*M_PI/180.0);
        cosr = cosf(rotation*M_PI/180.0);
    }


    // Copy original coordinates    
    sx1 = (float)triangle->sx1;
    sx2 = (float)triangle->sx2;
    sx3 = (float)triangle->sx3;
    sy1 = (float)triangle->sy1;
    sy2 = (float)triangle->sy2;
    sy3 = (float)triangle->sy3;

    // Rotate
    triangle->sx1 = (int)(sx1*cosr - sy1*sinr);
    triangle->sx2 = (int)(sx2*cosr - sy2*sinr);
    triangle->sx3 = (int)(sx3*cosr - sy3*sinr);
    triangle->sy1 = (int)(sx1*sinr + sy1*cosr);
    triangle->sy2 = (int)(sx2*sinr + sy2*cosr);
    triangle->sy3 = (int)(sx3*sinr + sy3*cosr);
}



// Draw triangle on screen
void DrawTriangle(SDL_Surface *screen, triangle_t *triangle)
{
    int isOK;
    
    // Scale
    ScaleTriangle(triangle);

    // Rotate triangle
    RotateTriangle(triangle);
    
    // Translate
    TranslateTriangle(triangle);
    
    // Determine bounding box
    CalculateTriangleBoundingBox(triangle);

    // Sanity check that triangle is within screen boundaries
    isOK = SanityCheckTriangle(screen, triangle);
    if (isOK == 0) {
        if (doConsoleDump == 1)
            PrintTriangle(triangle, "Triangle outside screen boundaries");
        return;
    }

    // Draw triangle
    DrawLine(screen, 
        triangle->sx1, triangle->sy1,
        triangle->sx2, triangle->sy2,
        TRIANGLE_PENCOLOR);
    DrawLine(screen, 
        triangle->sx2, triangle->sy2,
        triangle->sx3, triangle->sy3,
        TRIANGLE_PENCOLOR);
    DrawLine(screen, 
        triangle->sx3, triangle->sy3,
        triangle->sx1, triangle->sy1,
        TRIANGLE_PENCOLOR);

    // Fill triangle
    FillTriangle(screen, triangle);
}
