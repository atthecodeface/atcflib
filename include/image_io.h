/** Copyright (C) 2016,  Gavin J Stark.  All rights reserved.
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
 * @file          image_io.h
 * @brief         Image reading/writing library, using libpng/libjpeg
 *
 */

/*a Wrapper
 */
#ifdef __INC_IMAGE_IO
#else
#define __INC_IMAGE_IO

/*a Defines
 */

/*a Types
 */
/*t c_image_io
 */
class c_image_io
{
    struct prvt_data *prvt;
    int png_write_init(FILE *f);
    int png_write_finalize(void);
    int png_read_init(FILE *f);
    int png_read_set_rgb8(void);
    void png_read_finalize(void);
    int png_read_alloc(void);
    int write_image_png(void);
    int read_image_png(void);
public:
    c_image_io(void);
    ~c_image_io(void);
    void free_image_data(void);
    int jpeg_read(FILE *f);
    int png_read(FILE *f);
    int png_write(FILE *f);
 
    unsigned char *image_data;
    unsigned int width;    
    unsigned int height;
    int byte_width;
    int bit_depth;
    int color_type;

    int free_image_data_on_destruction;
};

extern int image_io_write_rgba(const char *filename, const unsigned char *image_data, int width, int height);
extern unsigned char *image_io_read_rgba(const char *filename, int *width, int *height);

/*a Wrapper
 */
#endif

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/
