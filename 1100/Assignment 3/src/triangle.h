#ifndef TRIANGLE_H_
#define TRIANGLE_H_

typedef struct triangle triangle_t;

struct triangle {
    // Model coordinates
    int x1, y1;
    int x2, y2;
    int x3, y3;

    // Fill color
    unsigned int fillcolor;
    
    // Scale factor
    float scale;

    // Translate to these coordinates before drawing on screen
    int tx, ty;
    
    // Bounding box of on-screen coordinates
    int bx, by;         // upper left corner of box
    int bw, bh;         // Width and height of box
    
    // On-screen coordinates
    int sx1, sy1;
    int sx2, sy2;
    int sx3, sy3;
};

// Draw triangle on screen.
void DrawTriangle(SDL_Surface *screen, triangle_t *triangle);


#endif /*TRIANGLE_H_*/
