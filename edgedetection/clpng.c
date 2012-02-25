//
//  glpng.c
//  OpenGLViewer
//
//  Created by Jonathan Hatchett on 09/11/2011.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//
#include "clpng.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

#include <png.h>

/** loadTexture
 *     loads a png file into an opengl texture object, using cstdio , libpng, and opengl.
 * 
 *     \param filename : the png file to be loaded
 *     \param width : width of png, to be updated as a side effect of this function
 *     \param height : height of png, to be updated as a side effect of this function
 * 
 *     \return GLuint : an opengl texture id.  Will be 0 if there is a major error,
 *                                     should be validated by the client of this function.
 * 
 */
int LoadPNG24(unsigned char ** pixelBuffer, const char *filename, unsigned int *width, unsigned int *height) {
	png_byte header[8];
	
	//open file as binary
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		return TEXTURE_LOAD_ERROR;
	}
	
	//read the header
	fread(header, 1, 8, fp);
	
	//test if png
	int is_png = !png_sig_cmp(header, 0, 8);
	if (!is_png) {
		fclose(fp);
		return TEXTURE_LOAD_ERROR;
	}
	
	//create png struct
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fclose(fp);
		return (TEXTURE_LOAD_ERROR);
	}
	
	//create png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		fclose(fp);
		return (TEXTURE_LOAD_ERROR);
	}
	
	//create png info struct
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
		fclose(fp);
		return (TEXTURE_LOAD_ERROR);
	}
	
	//png error stuff, not sure libpng man suggests this.
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return (TEXTURE_LOAD_ERROR);
	}
	
	//init png reading
	png_init_io(png_ptr, fp);
	
	//let libpng know you already read the first 8 bytes
	png_set_sig_bytes(png_ptr, 8);
	
	// read all the info up to the image data
	png_read_info(png_ptr, info_ptr);
	
	//variables to pass to get info
	int bit_depth, color_type;
	png_uint_32 twidth, theight;
	
	// get info about png
	png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);
	
	// Update the png info struct.
	png_read_update_info(png_ptr, info_ptr);
	
	// Row size in bytes.
	//png_size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	png_size_t rowbytes = sizeof(png_byte) * 4 * twidth;
	
	// Allocate the image_data as a big block, to be given to opencl
	png_byte *image_data = (png_byte *)malloc(sizeof(png_byte) * 4 * twidth * theight);
	if (!image_data) {
		//clean up memory and close stuff
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return TEXTURE_LOAD_ERROR;
	}
	
	//row_pointers is for pointing to image_data for reading the png with libpng
	png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * theight);
	if (!row_pointers) {
		//clean up memory and close stuff
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		free(image_data);
		fclose(fp);
		return TEXTURE_LOAD_ERROR;
	}
	
	// set the individual row_pointers to point at the correct offsets of image_data
	for (int i = 0; i < theight; ++i) {
		row_pointers[i] = image_data + (i * rowbytes);
	}
	
	// PNG Transforms
	if (color_type == PNG_COLOR_TYPE_RGB) {
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
		//png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
	}
	
	png_read_update_info(png_ptr, info_ptr);
	
	//read the png into image_data through row_pointers
	png_read_image(png_ptr, row_pointers);
	
	//clean up memory and close stuff
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	free(row_pointers);
	fclose(fp);
	
	//update width and height based on png info
	(*pixelBuffer) = image_data;
	if (width) {*width = twidth;}
	if (height) {*height = theight;}
	
	return TEXTURE_LOAD_OK;
}

int ClosePNG24(unsigned char ** pixelBuffer) {
	free(*pixelBuffer);
	
	return TEXTURE_LOAD_OK;
}

int CreatePNG24(unsigned char ** pixelBuffer, unsigned int width, unsigned int height) {
	png_size_t rowbytes = width * 4 * sizeof(png_byte);
	if (((*pixelBuffer) = malloc(rowbytes * height)) == NULL) {return TEXTURE_LOAD_ERROR;}
	
	return TEXTURE_LOAD_OK;
}

int WritePNG24(unsigned char ** pixelBuffer, const char *filename, unsigned int width, unsigned int height) {
	png_size_t rowbytes = ceil((width * 4 * sizeof(png_byte)) / 4) * 4;
	png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
	for (int i = 0; i < height; ++i) {
		row_pointers[i] = (*pixelBuffer) + (i * rowbytes);
	}
	
	/* create file */
	FILE *fp;
	if (!(fp = fopen(filename, "wb"))) {
		//fprintf(stderr, "[write_png_file] File %s could not be opened for writing", filename);
		free(row_pointers);
		return TEXTURE_LOAD_ERROR;
	}
	
	/* initialize stuff */
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!png_ptr) {
		//fprintf(stderr, "[write_png_file] png_create_write_struct failed");
		free(row_pointers);
		fclose(fp);
		return TEXTURE_LOAD_ERROR;
	}
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		//fprintf(stderr, "[write_png_file] png_create_info_struct failed");
		free(row_pointers);
		fclose(fp);
		return TEXTURE_LOAD_ERROR;
	}
	
	if (setjmp(png_jmpbuf(png_ptr))) {
		//fprintf(stderr, "[write_png_file] Error during init_io");
		free(row_pointers);
		fclose(fp);
		return TEXTURE_LOAD_ERROR;
	}
	
	png_init_io(png_ptr, fp);
	
	
	/* write header */
	if (setjmp(png_jmpbuf(png_ptr))) {
		//fprintf(stderr, "[write_png_file] Error during writing header");
		free(row_pointers);
		fclose(fp);
		return TEXTURE_LOAD_ERROR;
	}
	
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	png_write_info(png_ptr, info_ptr);
	
	// Image Transforms
	// Get rid of filler (OR ALPHA) bytes, pack XRGB/RGBX/ARGB/RGBA into
	// RGB (4 channels -> 3 channels). The second parameter is not used.
	png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
	
	png_read_update_info(png_ptr, info_ptr);
	
	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr))) {
		//fprintf(stderr, "[write_png_file] Error during writing bytes");
		free(row_pointers);
		fclose(fp);
		return TEXTURE_LOAD_ERROR;
	}
	
	png_write_image(png_ptr, row_pointers);
	
	/* end write */
	if (setjmp(png_jmpbuf(png_ptr))) {
		//fprintf(stderr, "[write_png_file] Error during end of write");
		free(row_pointers);
		fclose(fp);
		return TEXTURE_LOAD_ERROR;
	}
	
	png_write_end(png_ptr, NULL);
	
	free(row_pointers);
	
	fclose(fp);
	
	return TEXTURE_LOAD_OK;
}
