#ifndef DRAWLINE_H_
#define DRAWLINE_H_

// Draw a line on the screen from x1,y1 to x2,y2 with a given pen color
void DrawLine(SDL_Surface *screen, int x1, int y1, int x2, int y2, unsigned int color);

// Read pixel x,y on the screen
unsigned int GetPixel(SDL_Surface *screen, int x, int y);

// Set pixel x,y on the screen
void SetPixel(SDL_Surface *screen, int x, int y, unsigned int color);

#endif /* DRAWLINE_H_ */
