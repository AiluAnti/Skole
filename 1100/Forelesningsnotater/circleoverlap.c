#include <stdio.h>
#include <stdlib.h>
#include <math.h>


// Define an "alias" for the data structure describing a circle
// The alias allows you to refer to the data structure by writing
// cirle_t instead of struct circle.
typedef struct circle circle_t;

// Define a data structure describing a circle
struct circle {
    int x;
    int y;
    int radius;
};



int isoverlap(circle_t *a, circle_t *b)
{
    float dx;
    float dy;
    float distance;
    float mindistance;

    // Calculate the distance between the circles by use of pythagoras
    dy = a->y - b->y;
    dx = a->x - b->x;
    distance = sqrtf(dy*dy + dx*dx);

    // Radius of circle a + radius of circle b defines
    // the minimum distance between a and b before we have overlap
    mindistance = a->radius + b->radius;

    // If distance less than the sum of radii, we have overlap
    if (distance < mindistance)
        return 1;
    else
        return 0;
}


void testoverlap(circle_t *circles, int numcircles)
{
    int i;
    int j;

    // Traverse array and check for overlap between all circles.
    // Note the nested for loop.  First we check circle 0 against
    // circle 1..(numcircles-1).  Then we check circle 1 against
    // circle 2..(numcircles-1) etc.
    // Also note that even if circles is a variable containing the memory address
    // of a circles_t data structure, we can treat circles as an array.
    // This is because of the duality of arrays and pointers in C:
    // a pointer is assumed to hold the memory address of the first
    // element in an array.
    for (i = 0; i < (numcircles-1); i = i+1) {
        for (j = i+1; j < numcircles; j = j+1) {
            if (isoverlap(&circles[i], &circles[j])) {
                printf("Circle %d and %d overlap\n", i, j);
            } else {
                printf("Circle %d and %d does NOT overlap\n", i, j);
            }
        }
    }
}


int main(void)
{
    circle_t somecircles[3];

    // Initialize array.
    // Note that here we use . to refer to fields of the data structure.
    // This is because somecircles is a variable.
    somecircles[0].x = 100;
    somecircles[0].y = 100;
    somecircles[0].radius = 10;

    somecircles[1].x = 150;
    somecircles[1].y = 100;
    somecircles[1].radius = 50;

    somecircles[2].x = 220;
    somecircles[2].y = 220;
    somecircles[2].radius = 40;


    // Note that the argument to testoverlap is the memory address of the somecircles array
    testoverlap(somecircles, 3);

}
