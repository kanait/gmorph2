/*
 * image.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include "sgiimage.h"

/*
 * save Displayed Image into iris RGB File
 */
int
saveRgbImage(
	     char *fileName,
	     long imgWidth,
	     long imgHeight
	     )
{
  unsigned long *image;
  
#ifdef DEBUG
  fprintf(stderr, "[saveRgbImage %s %d %d]\n", fileName, imgWidth, imgHeight);
#endif
  
  image = malloc(imgWidth * imgHeight * sizeof(unsigned long));
  
  glReadBuffer(GL_FRONT);
  glReadPixels(0, 0, imgWidth, imgHeight, GL_RGBA, GL_UNSIGNED_BYTE, image);
     
  /* rgbWriteImageFile(filename, imgWidth, imgHeight, image); */
  longstoimage(image, imgWidth, imgHeight, 4, fileName);
  free(image);

#ifdef DEBUG
  fprintf(stderr, "[saveRgbImage end]\n");
#endif
  return (0);
}
	     
	     
	     
