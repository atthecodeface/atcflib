/** Copyright (C) 2016-2017,  Gavin J Stark.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file          image_io.cpp
 * @brief         Image input and output functions
 *
 */

/*a Includes
 */
#include <stdlib.h>
#include <stdio.h>
// jpeglib needs ssize_t, FILE before it is included...
#include <jpeglib.h>
#include <png.h>
#include <setjmp.h>
#include <string.h>
#include "image_io.h"

/*a Types
 */
/*t prvt_data */
struct prvt_data {
    png_structp png_ptr;
    png_infop   info_ptr;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jpeg_err;
    jmp_buf jpeg_setjmp_buffer;
};

/*a Constructor and destructors, free etc */
/*f c_image_io::c_image_io */
c_image_io::c_image_io(void)
{
    prvt = (struct prvt_data *)malloc(sizeof(prvt_data));
    prvt->info_ptr = NULL;
    prvt->png_ptr = NULL;
    image_data = NULL;
    free_image_data_on_destruction = 1;
}

/*f c_image_io::~c_image_io */
c_image_io::~c_image_io(void)
{
    if (free_image_data_on_destruction) {
        free_image_data();
    }
    if (prvt) {
        free(prvt);
        prvt = NULL;
    }
}

/*f c_image_io::free_image_data */
void
c_image_io::free_image_data(void)
{
    if (image_data) {
        free(image_data);
    }
    image_data = NULL;
}

/*f PNG methods */
/*f c_image_io::png_read_init - initialize for reading a PNG */
int
c_image_io::png_read_init(FILE *f)
{
    int ret=1;
    unsigned char png_sig[8];

    prvt->info_ptr = NULL;
    prvt->png_ptr = NULL;

    if (!f)
        return 1;

    fread(png_sig, 1, 8, f);
    if (png_check_sig(png_sig, 8)) {
        prvt->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    }

    if (prvt->png_ptr) {
        prvt->info_ptr = png_create_info_struct(prvt->png_ptr);
    }

    if (prvt->info_ptr) {
        int err;
        err = setjmp(png_jmpbuf(prvt->png_ptr));
        if (err==0) {
            png_init_io(prvt->png_ptr, f);
            png_set_sig_bytes(prvt->png_ptr, 8);
            png_read_info(prvt->png_ptr, prvt->info_ptr);
            png_get_IHDR(prvt->png_ptr, prvt->info_ptr,
                         &width, &height,
                         &bit_depth, &color_type,
                         NULL, NULL, NULL);
            ret = 0;
        }
    }
    return ret;
}

/*f c_image_io::png_read_set_rgb8 - */
int
c_image_io::png_read_set_rgb8(void)
{
    int ret;
    int err;

    ret = 1;
    err = setjmp(png_jmpbuf(prvt->png_ptr));

    if (err==0) {
        switch (color_type) {
        case PNG_COLOR_TYPE_PALETTE:
            png_set_expand(prvt->png_ptr);
            break;
        case PNG_COLOR_TYPE_GRAY:
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            if (bit_depth<8) png_set_expand(prvt->png_ptr);
            png_set_gray_to_rgb(prvt->png_ptr);
            break;
        }
        if (bit_depth == 16) png_set_strip_16(prvt->png_ptr);

        png_read_update_info(prvt->png_ptr, prvt->info_ptr);

        byte_width = png_get_rowbytes(prvt->png_ptr, prvt->info_ptr);

        ret = 0;
    }
    return ret;
}

/*f c_image_io::png_read_alloc - Allocate data for an image */
int
c_image_io::png_read_alloc(void)
{
    image_data = (unsigned char *)malloc(height*byte_width);
    return (image_data==NULL);
}

/*f c_image_io::read_image_png */
int
c_image_io::read_image_png(void)
{
    int ret;
    int err;

    ret = 1;
    err = setjmp(png_jmpbuf(prvt->png_ptr));

    if (err==0) {
        png_bytepp row_pointers;

        row_pointers = NULL;
        if (image_data) {
            row_pointers = (png_bytepp) malloc(height*sizeof(png_bytep));
        }
        if (row_pointers) {
            for (int i=0; i<height; i++) {
                row_pointers[i] = image_data + i*byte_width;
            }
            png_read_image(prvt->png_ptr, row_pointers);
        }

        if (row_pointers) {
            free(row_pointers);
            row_pointers = NULL;
        }

        png_read_end(prvt->png_ptr, NULL);
        ret = 0;
    }
    return ret;
}

/*f c_image_io::png_read_finalize - finalize reading a PNG image
 * 
 * Returns zero on success
 *
 */
void c_image_io::png_read_finalize(void)
{
    if (prvt->png_ptr) {
        if (prvt->info_ptr) {
            png_destroy_read_struct(&prvt->png_ptr, &prvt->info_ptr, NULL);
        } else {
            png_destroy_read_struct(&prvt->png_ptr, NULL, NULL);
        }
        prvt->info_ptr = NULL;
        prvt->png_ptr = NULL;
    }
}

/*f c_image_io::png_read - read a PNG image from a file
 * 
 * Returns zero on success
 *
 */
int
c_image_io::png_read(FILE *f)
{
    int ret;

    ret = png_read_init(f);

    if (!ret) ret=png_read_set_rgb8();
    if (!ret) ret=png_read_alloc();
    if (!ret) ret=read_image_png();

    png_read_finalize();

    return ret;
}

/*f c_image_io::png_write_init
 * 
 * Returns zero on success
 *
 */
int
c_image_io::png_write_init(FILE *f)
{
    int ret=1;

    prvt->info_ptr = NULL;
    prvt->png_ptr = NULL;

    if (f) {
        prvt->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL, //(png_voidp)user_error_ptr,
                                                NULL, //user_error_fn,
                                                NULL ); //user_warning_fn/);
    }
    if (prvt->png_ptr) {
        prvt->info_ptr = png_create_info_struct(prvt->png_ptr);
    }
    if (prvt->info_ptr)
    {
        int err;
        err = setjmp(png_jmpbuf(prvt->png_ptr));
        if (err==0) {
            png_init_io(prvt->png_ptr, f);
            png_set_IHDR(prvt->png_ptr, prvt->info_ptr,
                         width, height,
                         8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                         PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
            png_write_info(prvt->png_ptr, prvt->info_ptr);
            ret = 0;
        }
    }
    return ret;
}

/*f c_image_io::write_image
 * 
 * Returns zero on success
 *
 */
int
c_image_io::write_image_png(void)
{
    png_byte **row_pointers;
    row_pointers = (png_byte **)malloc(sizeof(png_byte *)*height);
    for (int i=0; i<height; i++) {
        row_pointers[i] = image_data+i*byte_width;
    }
    png_write_image(prvt->png_ptr, row_pointers);
    free(row_pointers);
    return 0;
}

/*f c_image_io::png_write_finalize
 * 
 * Returns zero on success
 *
 */
int
c_image_io::png_write_finalize(void)
{
    png_write_end(prvt->png_ptr, prvt->info_ptr);
    if (prvt->png_ptr) {
        if (prvt->info_ptr==NULL) {
            png_destroy_write_struct(&prvt->png_ptr, (png_infopp) NULL);
        } else {
            png_destroy_write_struct(&prvt->png_ptr, &prvt->info_ptr);
        }
    }
    return 0;
}


/*f c_image_io::png_write - write an image as a PNG to a file
 * 
 * Returns zero on success
 *
 */
int
c_image_io::png_write(FILE *f)
{
    int ret;

    ret = png_write_init(f);

    if (!ret) ret=write_image_png();

    png_write_finalize();

    return ret;
}

/*a Jpeg functions
 */
static void jpeg_error_exit(j_common_ptr cinfo)
{
    struct prvt_data *prvt = (struct prvt_data *) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(prvt->jpeg_setjmp_buffer, 1);
}

int c_image_io::jpeg_read(FILE *f)
{
    int err;
    int ret;

    ret = 1;
    prvt->cinfo.err = jpeg_std_error(&prvt->jpeg_err);
    prvt->jpeg_err.error_exit = jpeg_error_exit;
    
    err = setjmp(prvt->jpeg_setjmp_buffer);
    if (err==0) {
        JSAMPARRAY buffer; // pointer to one row of data and its data
        int bytes_per_jpeg_row;

        jpeg_create_decompress(&prvt->cinfo);
        jpeg_stdio_src(&prvt->cinfo, f);
        jpeg_read_header(&prvt->cinfo, TRUE); // return value?
        jpeg_calc_output_dimensions(&prvt->cinfo);  //See libjpeg.txt for more about this.
        bytes_per_jpeg_row = prvt->cinfo.output_components * prvt->cinfo.output_width;
        buffer = (*(prvt->cinfo.mem->alloc_sarray))((j_common_ptr) &prvt->cinfo,
                                                    JPOOL_IMAGE,
                                                    bytes_per_jpeg_row,
                                                    1);

        jpeg_start_decompress(&prvt->cinfo); // return value?
        height = prvt->cinfo.output_height;
        width  = prvt->cinfo.output_width;
        bit_depth = 8;
        byte_width = width*4;
        image_data = (unsigned char *)malloc(height*byte_width);

        while (1) {
            jpeg_read_scanlines(&prvt->cinfo, buffer, 1);
            int y = prvt->cinfo.output_scanline;
            if (y>=prvt->cinfo.output_height)
                break;
            for (int x=0; x<prvt->cinfo.output_width; x++) {
                image_data[y*byte_width+x*4+0] = 0;
                image_data[y*byte_width+x*4+1] = 0;
                image_data[y*byte_width+x*4+2] = 0;
                image_data[y*byte_width+x*4+3] = 0;
                for (int c=0; c<prvt->cinfo.output_components; c++) {
                    image_data[y*byte_width+x*4+c] = buffer[0][x*prvt->cinfo.output_components+(2-c)];
                }
                image_data[y*byte_width+x*4+0] = buffer[0][x*prvt->cinfo.output_components+0];
                image_data[y*byte_width+x*4+1] = buffer[0][x*prvt->cinfo.output_components+1];
                image_data[y*byte_width+x*4+2] = buffer[0][x*prvt->cinfo.output_components+2];
                image_data[y*byte_width+x*4+3] = buffer[0][x*prvt->cinfo.output_components+3];
            }
        }

        jpeg_finish_decompress(&prvt->cinfo); // return value?
        ret = 0;
    }
    jpeg_destroy_decompress(&prvt->cinfo);
    return ret;
}


/*a External image read/write functions
 */
/*f image_io_write_rgba - write a 32bpp (8-bit RGBA) image as a PNG to a filename given data, width and height
 */
extern int
image_io_write_rgba(const char *filename, const unsigned char *image_data, int width, int height)
{
    c_image_io image_io;
    int err;

    image_io.free_image_data_on_destruction = 0;
    image_io.width = width;
    image_io.height = height;
    image_io.byte_width = 4*width;
    image_io.bit_depth = 8;
    image_io.color_type = 0;
    image_io.image_data = (unsigned char *)image_data;
    FILE *fp = fopen(filename, "wb");

    if (!fp)
        return 1;

    err = image_io.png_write(fp);
    fclose(fp);

    return err;
}

/*f image_io_read_rgba - read an image from a filename, returning mallocked image data and setting width and height
 */
extern unsigned char *
image_io_read_rgba(const char *filename, int *width, int *height)
{
    c_image_io image_io;
    int err;
    FILE *fp = fopen(filename, "rb");

    if (!fp)
        return NULL;

    if ( (strlen(filename)>3) &&
         ((!strcmp(filename+strlen(filename)-3,"jpg")) ||
          ((!strcmp(filename+strlen(filename)-3,"JPG"))) ) ) {
        err = image_io.jpeg_read(fp);
    } else {
        err = image_io.png_read(fp);
    }
    fclose(fp);
    if (err) {
        return NULL;
    }
    *width = image_io.width;
    *height = image_io.height;
    image_io.free_image_data_on_destruction = 0;
    return image_io.image_data;
}

