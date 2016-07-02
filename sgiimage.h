#ifndef __SGIIMAGE_H__
#define __SGIIMAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned long *getLongImage(char *textureFile, int *xsize, int *ysize);
extern int longstoimage(unsigned long *lptr, long xsize, long ysize, long zsize, char *name);
extern unsigned long *longimagedata(char *name);
extern void sizeofimage(char *name, long *xsize, long *ysize);


#ifdef __cplusplus
}
#endif

#endif  /* __SGIIMAGE_H__ */
