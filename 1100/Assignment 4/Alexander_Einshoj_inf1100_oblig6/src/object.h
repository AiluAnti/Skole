#ifndef OBJECT_H_
#define OBJECT_H_

typedef struct object object_t;

struct object {
    float       scale;          // Object scale
    float       rotation;       // Object rotation
    float       tx, ty;         // Position on screen
    
    float       speedx, speedy; // Object speed in x and y direction
    unsigned int ttl;           // Time till object should be removed from screen
    
    int         numtriangles;   // Number of triangles in model
    triangle_t  *model;         // Model triangle array

    SDL_Surface *screen;        // SDL screen
};


/* Interfaces */
object_t *CreateObject(SDL_Surface *screen, triangle_t *triangles, int numtriangles);
void DestroyObject(object_t *object);
void DrawObject(object_t *object);


#endif /*OBJECT_H_*/
