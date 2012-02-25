//
//  glpng.h
//  OpenGLViewer
//
//  Created by Jonathan Hatchett on 09/11/2011.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef OpenGLViewer_glpng_h
#define OpenGLViewer_glpng_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TEXTURE_LOAD_OK
#define TEXTURE_LOAD_OK		0
#endif
#ifndef TEXTURE_LOAD_ERROR
#define TEXTURE_LOAD_ERROR	1
#endif

int LoadPNG24(unsigned char ** pixelBuffer, const char *filename, unsigned int *width, unsigned int *height);
	
int ClosePNG24(unsigned char ** pixelBuffer);

int CreatePNG24(unsigned char ** pixelBuffer, unsigned int width, unsigned int height);

int WritePNG24(unsigned char ** pixelBuffer, const char *filename, unsigned int width, unsigned int height);

#ifdef __cplusplus
}
#endif

#endif
