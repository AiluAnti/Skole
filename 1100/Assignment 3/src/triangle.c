#include <stdlib.h>
#include <stdio.h>
#include "SDL.h"
#include "triangle.h"
#include "drawline.h"

#define TRIANGLE_PENCOLOR   0xBBBB0000
#define BOUNDINGBOX_PENCOLOR 0xFFFFFFFF


// Print triangle coordinates along with a message
void PrintTriangle(triangle_t *triangle, char *msg)
{
    printf("%s: %d,%d - %d,%d - %d,%d\n",
        msg,
        triangle->x1, triangle->y1, 
        triangle->x2, triangle->y2, 
        triangle->x3, triangle->y3);
}


// Return 0 if triangle coordinates are outside the screen boundary. 1 otherwise.
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
    // TODO: Replace the code below with code that scales each triangle coordinate. 
    // The scaling factor is specified in triangle->scale.
    // Remember that this function MUST write to the on-screen coordinates.
    // Do not alter the model coordinates.
    triangle->sx1 = (int)(triangle->x1 * triangle->scale);
    triangle->sx2 = (int)(triangle->x2 * triangle->scale);
    triangle->sx3 = (int)(triangle->x3 * triangle->scale);
    triangle->sy1 = (int)(triangle->y1 * triangle->scale);
    triangle->sy2 = (int)(triangle->y2 * triangle->scale);
    triangle->sy3 = (int)(triangle->y3 * triangle->scale);
}


// Move triangle to its screen position
void TranslateTriangle( triangle_t *triangle)
{
    // TODO: Insert code that moves the triangle on the screen.
    // The translation coordinates are specified in triangle->tx and triangle->ty.
    // Remember to use the on-screen coordinates (triangle->sx1, etc.)
	
    triangle->sx1=triangle->sx1 + triangle->tx;
    triangle->sx2=triangle->sx2 + triangle->tx;
    triangle->sx3=triangle->sx3 + triangle->tx;
    triangle->sy1=triangle->sy1 + triangle->ty;
    triangle->sy2=triangle->sy2 + triangle->ty;
    triangle->sy3=triangle->sy3 + triangle->ty;
}


// Calculate triangle bounding box
void CalculateTriangleBoundingBox(triangle_t *triangle)
{
    // TODO: Insert code that calculates the bounding box of a triangle.
    // Remember to use the on-screen coordinates (triangle->sx1, etc.)
    // The bounding box coordinates should be written to 
    // triangle->bx, triangle->by, triangle->bw, triangle->bh
	
	triangle->bx = triangle->sx1;
	if(triangle->bx > triangle->sx2){
		triangle->bx = triangle->sx2;
		}
	if(triangle->bx > triangle->sx3){
		triangle->bx = triangle->sx3;
		}
		
	triangle->by = triangle->sy1;
	if(triangle->by > triangle->sy2){
		triangle->by = triangle->sy2;
		}
	if(triangle->by > triangle->sy3){
		triangle->by = triangle->sy3;
		}
	
	int w = triangle->sx1;
	if(w < triangle->sx2){
		w = triangle->sx2;
		}
	if(w < triangle->sx3){
		w = triangle->sx3;
		}
	triangle->bw = w - triangle->bx;
	
	int h = triangle->sy1;
	if(h < triangle->sy2){
		h = triangle->sy2;
	}
	if(h < triangle->sy3){
		h = triangle->sy3;
	}
	triangle->bh = h - triangle->by;

	
}

// Fill triangle with a color
void FillTriangle(SDL_Surface *screen, triangle_t *triangle)
{
    // TODO: Insert code that fills the triangle with the color specified in triangle->fillcolor.
    // Hint: Draw the triangle with color TRIANGLE_PENCOLOR (this color can not
    // occur in e.g. the teapot or the example triangles).  Thus, if your 
    // approach to filling the triangle relies on looking for the edges of
    // the triangle on the screen (via the GetPixel function), you will find those
    // edges even if the triangle overlaps with a triangle that has already
    // been drawn on the screen.
	
		
		int x, y;
 
	for(y = triangle->by; y <= triangle->by + triangle->bh; y++){
		int startx = 0;
		int endx = 0;
  
		for(x = triangle->bx; x <= triangle->bx + triangle->bw; x++){
			if(GetPixel(screen, x, y) == TRIANGLE_PENCOLOR && startx == 0){
				startx = x;
				endx = x;
			}
			else if(GetPixel(screen, x, y) == TRIANGLE_PENCOLOR) {
				endx = x;
			}
  
		}
	DrawLine(screen, startx, y, endx, y, triangle->fillcolor);
 
	}

}

// Draw triangle on screen
void DrawTriangle(SDL_Surface *screen, triangle_t *triangle)
{
    int isOK;
    
    // Scale.
    ScaleTriangle(triangle);
    
    // Translate.
    TranslateTriangle(triangle);
    
    // Determine bounding box
    CalculateTriangleBoundingBox(triangle);
		//	DrawLine(screen, triangle->bx, triangle->by, triangle->bx + triangle->bw, triangle->by, BOUNDINGBOX_PENCOLOR);
		//	DrawLine(screen, triangle->bx, triangle->by, triangle->bx, triangle->by + triangle->bh, BOUNDINGBOX_PENCOLOR);
		//	DrawLine(screen, triangle->bx + triangle->bw, triangle->by ,triangle->bx + triangle->bw, triangle->by + triangle->bh, BOUNDINGBOX_PENCOLOR);
		//	DrawLine(screen, triangle->bx, triangle->by + triangle->bh, triangle->bx + triangle->bw, triangle->by + triangle->bh, BOUNDINGBOX_PENCOLOR);
			
    // Sanity check that triangle is within screen boundaries.
    isOK = SanityCheckTriangle(screen, triangle);
    if (isOK == 0) {
        PrintTriangle(triangle, "Triangle outside screen boundaries");
        return;
    }

    // TODO: Insert calls to DrawLine to draw the triangle.
    // Remember to use the on-screen coordinates (triangle->sx1, etc.)
			DrawLine(screen, triangle->sx1, triangle->sy1, triangle->sx2, triangle->sy2, TRIANGLE_PENCOLOR);
			DrawLine(screen, triangle->sx2, triangle->sy2, triangle->sx3, triangle->sy3, TRIANGLE_PENCOLOR);
			DrawLine(screen, triangle->sx3, triangle->sy3, triangle->sx1, triangle->sy1, TRIANGLE_PENCOLOR);
    // Fill triangle
		FillTriangle(screen, triangle);

    // Force screen update.  
    SDL_UpdateRect(screen, triangle->bx, triangle->by, triangle->bw, triangle->bh);

    // Force update of entire screen.  Comment/remove this call and uncomment the above call
    // when your CalculateTriangleBoundingBox function has been implemented.
    //SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
}
