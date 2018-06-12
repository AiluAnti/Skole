#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "SDL.h"
#include "drawline.h"
#include "triangle.h"
#include "object.h"

#define TRIANGLE_PENCOLOR   0xBBBB0000

// Create new object
object_t *CreateObject(SDL_Surface *screen, triangle_t *model, int numtriangles)
{
    object_t *obj = malloc(sizeof(object_t));
    obj->scale = 0.1;
    obj->rotation = 0.0;
    obj->tx = 0.0;
    obj->ty = 0.0;
    obj->speedx = 3.0 + (random() % 30);
    obj->speedy = 3.0 + (random() % 30);
    obj->ttl = 4000;
    obj->numtriangles = numtriangles;
    obj->model = malloc(sizeof(triangle_t) * numtriangles);
    obj->screen = screen;
    
    memcpy(obj->model, model, sizeof(triangle_t) * numtriangles);

    return obj;
}

// Free memory used by object
void DestroyObject(object_t *object)
{
    free(object);
}


// Draw object on screen
void DrawObject(object_t *object)
{

	int i;
	for (i = 0; i<object->numtriangles; i++)
    {
    	object->model[i].tx = object->tx;
    	object->model[i].ty = object->ty;
    	object->model[i].scale = object->scale;
    	object->model[i].rotation = object->rotation;
    	DrawTriangle(object->screen, &object->model[i]);
    }
} 
