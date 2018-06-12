#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


unsigned char freemap[1024];


int bitalloc(unsigned char *bitmap, int bitmapnbytes)
{
  int byteidx, bitidx;
  
  // Find byte with at least one bit == 0
  for (byteidx = 0; byteidx < bitmapnbytes; byteidx++) {
      if (bitmap[byteidx] != 0xff)
	  break;
  }
  
  // If no 0 bits found, return
  if (byteidx >= bitmapnbytes)
      return -1;

  // Find index of first 0 bit within byte
  for (bitidx = 0; bitidx < 8; bitidx++) {
      if ((bitmap[byteidx] & (1 << bitidx)) == 0) {
	bitmap[byteidx] |= (1 << bitidx);
	break;
      }
  }
  
  return byteidx*8 + bitidx;
}


void bitfree(unsigned char *bitmap, int bitmapnbytes, int freebit)
{
  int byteidx, bitidx;
  
  // Calculate byte and bit index
  byteidx = freebit/8;
  bitidx = freebit % 8;
  
  // Check that input is correct
  assert(byteidx < bitmapnbytes);

  // Any value != 0 will trigger the assert 
  assert(bitmap[byteidx] & (1 << bitidx));

  // Set bit to 0
  bitmap[byteidx] &= ~(1 << bitidx);
}






int main(int argc, char **argv)
{
  int myfreebit;
  
  myfreebit = bitalloc(freemap, 1024);
  printf("%d is free (should be 0)\n", myfreebit);
  myfreebit = bitalloc(freemap, 1024);
  printf("%d is free (should be 1)\n", myfreebit);
  
  // Free bit 1
  bitfree(freemap, 1024, myfreebit);
  myfreebit = bitalloc(freemap, 1024);
  printf("%d is free (should be 1)\n", myfreebit);
  
  
  // This is wrong. Assert will catch the error
  bitfree(freemap, 1024, 100000);
}
