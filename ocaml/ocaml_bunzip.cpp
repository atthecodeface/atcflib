/** Copyright (C) 2017,  Gavin J Stark.  All rights reserved.
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
 * @file          ocaml_bunzip.cpp
 * @brief         Bzip2 decompression methods
 *
 */

/*a Documentation
 * 
 * This is a wrapper around the ATCF library bunzip class
 *
 * It provides ocaml wrappers for the bunzip methods in 'bunzip.h'
 *
 * bunzip_create unit -> bz
 *
 * bunzip_reset bz -> unit - frees data, unmaps file
 *
 * bunzip_set_size (sets block_size) bz -> n -> unit
 *
 * bunzip_use_buffer bz -> array (of 100*1000*block_size*sizeof(int32)) -> unit
 *
 * [bunzip_block_map_file fd -> start_bit -> end_bit -> int]
 *
 * bunzip_block_data bz -> array -> start_bit -> end_bit -> int
 *
 * bunzip_block_read_header  bz -> start_bit -> end_bit -> int
 *
 * bunzip_block_huffman_decode  bz -> int/unit
 *
 * bunzip_block_mtf          bz -> int/unit
 *
 * bunzip_block_bwt_order    bz -> int/unit
 *
 * bunzip_block_no_rle_size  bz -> int (returns size of data without final RLE)
 *
 * bunzip_block_rle_size     bz -> int (returns size of data with final RLE)
 *
 * bunzip_block_decompress   bz -> array -> int -> int -> int (returns size of data - may be larger than array)
 *
 *
 * Usage:
 *
 * bz = bunzip_create
 * bunzip_reset bz
 * bunzip_set_size bz (1-9 - comes from file header)
 * [bunzip_use_buffer bz bigarray] - optional - will malloc if not
 * 
 * Then to decompres etc a block:
 * bunzip_block_data bz data_bigarray start_bit end_bit (if non-zero, then an error - abort)
 * bunzip_block_read_header bz    (parses header of block, symbol map, selectors, huffman tables)
 * bunzip_block_huffman_decode bz (huffman decodes from data to buffer, leaving data in mtf)
 * bunzip_block_mtf bz            (undoes mtf inside the buffer, after huffman_decode)
 * bunzip_block_bwt_order bz      (works out order of data in buffer, after mtf)
 * bunzip_block_no_rle_size bz    (at any point after huffman_decode )
 * bunzip_block_no_rle_decompress bz decomp_array   (after bwt_order, decompresses without RLE)
 * bunzip_block_rle_size bz       (only works after bwt_order, has to run through the data buffer)
 * bunzip_block_rle_decompress bz decomp_array   (after bwt_order, decompresses with RLE)
 */
/*a Includes
 */
#define CAML_NAME_SPACE 
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/intext.h>
#include <caml/threads.h>
#include <caml/bigarray.h>

#include <atcf/bunzip.h>

#include "ocaml_atcflib.h"

#include <stdio.h>

/*a Defines
 */
// Use -D__OCAML_BUNZIP_VERBOSE on compilation to be verbose,
// or uncomment the following
// #define __OCAML_BUNZIP_VERBOSE
#ifdef __OCAML_BUNZIP_VERBOSE
#define VERBOSE fprintf
#else
#define VERBOSE __discard
static void __discard(void *, ...) {}
#endif

/*a Statics
 */
static void finalize_bunzip(value v)
{
    delete bunzip_of_val(v);
}

static struct custom_operations custom_ops = {
    (char *)"atcf.bunzip",
    finalize_bunzip,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default
};

/*a Creation functions
 */
/*f caml_atcf_alloc_bunzip
 *
 * Creates a bunzip from a NEW c_bunzip
 *
 */
extern value
caml_atcf_alloc_bunzip(c_bunzip *cv)
{
    value v = caml_alloc_custom(&custom_ops, sizeof(c_bunzip *), 0, 1);
    bunzip_of_val(v) = cv;
    VERBOSE(stderr,"Allocked caml bunzip %p\n", cv);
    return v;
}

/*f atcf_bunzip_create : n:int -> NEW c_bunzip
 *
 * Creates a bunzip of length n
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_create(void)
{
    CAMLparam0();
    VERBOSE(stderr,"Create bunzip\n");
    CAMLreturn(caml_atcf_alloc_bunzip(new c_bunzip()));
}


/*f atcf_bunzip_destroy : c_bunzip -> unit
 *
 * Destroys a bunzip
 *
 */
extern "C"
CAMLprim void
atcf_bunzip_destroy(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Destroy bunzip %p\n", bunzip_of_val(v));
    delete bunzip_of_val(v);
    bunzip_of_val(v) = NULL;
    CAMLreturn0;
}

/*f atcf_bunzip_set_size : c_bunzip -> int -> unit
 *
 * Set the block size for the bunzip2 file
 *
 */
extern "C"
CAMLprim void
atcf_bunzip_set_size(value v, value n)
{
    CAMLparam2(v, n);
    bunzip_of_val(v) -> set_size(Long_val(n));
    CAMLreturn0;
}

/*a Block functions
 */
/*f atcf_bunzip_block_data : c_bunzip -> bigarray -> int -> int -> int
 *
 * Return an array containing the coordinates of the bunzip
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_block_data(value bz, value ba, value s, value e)
{
    CAMLparam4(bz,ba,s,e);
    c_bunzip *cb = bunzip_of_val(bz);
    int start_bit=Long_val(s);
    int end_bit=Long_val(e);
    const t_uint8 *data = (const t_uint8 *)(Caml_ba_array_val(ba)->data);
    if (((end_bit+7)/8) >= Caml_ba_array_val(ba)->dim[0]) {
        fprintf(stderr,"End bit too long\n");
    }
    CAMLreturn(Val_long(cb->use_data(data, start_bit, end_bit)));
}

/*f atcf_bunzip_block_read_header : c_bunzip -> int
 *
 * Read the header; 0 result means all good, others indicate type of failure
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_block_read_header(value v)
{
    CAMLparam1(v);
    c_bunzip *cb = bunzip_of_val(v);
    CAMLreturn(Val_long(cb->read_header()));
}

/*f atcf_bunzip_block_start_bit : c_bunzip -> int64
 *
 * Return the start bit (offset from start of data presented)
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_block_start_bit(value bz)
{
    CAMLparam1(bz);
    c_bunzip *cb = bunzip_of_val(bz);
    CAMLreturn(caml_copy_int64(cb->block_start_bit()));
}

/*f atcf_bunzip_block_end_bit : c_bunzip -> int64
 *
 * Return the end bit (offset from end of data presented)
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_block_end_bit(value bz)
{
    CAMLparam1(bz);
    c_bunzip *cb = bunzip_of_val(bz);
    CAMLreturn(caml_copy_int64(cb->block_end_bit()));
}

/*f atcf_bunzip_block_magic : c_bunzip -> int64
 *
 * Return the 48 bits of block magic
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_block_magic(value bz)
{
    CAMLparam1(bz);
    c_bunzip *cb = bunzip_of_val(bz);
    CAMLreturn(caml_copy_int64(cb->block_magic()));
}

/*f atcf_bunzip_block_huffman_decode : c_bunzip -> int
 *
 * Read the header; 0 result means all good, others indicate type of failure
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_block_huffman_decode(value v)
{
    CAMLparam1(v);
    c_bunzip *cb = bunzip_of_val(v);
    CAMLreturn(Val_long(cb->huffman_decode()));
}

/*f atcf_bunzip_block_mtf : c_bunzip -> int
 *
 * Read the header; 0 result means all good, others indicate type of failure
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_block_mtf(value v)
{
    CAMLparam1(v);
    c_bunzip *cb = bunzip_of_val(v);
    CAMLreturn(Val_long(cb->mtf()));
}

/*f atcf_bunzip_block_bwt_order : c_bunzip -> int
 *
 * Read the header; 0 result means all good, others indicate type of failure
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_block_bwt_order(value v)
{
    CAMLparam1(v);
    c_bunzip *cb = bunzip_of_val(v);
    CAMLreturn(Val_long(cb->bwt_order()));
}

/*f atcf_bunzip_block_no_rle_size : c_bunzip -> int
 *
 * Returns the size of the decompressed data prior to the final RLE
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_block_no_rle_size(value v)
{
    CAMLparam1(v);
    c_bunzip *cb = bunzip_of_val(v);
    CAMLreturn(Val_long(cb->data_buffer_used));
}

/*f atcf_bunzip_block_no_rle_decompress : c_bunzip -> bigarray -> int
 *
 * Decompress without the final RLE (so buffer size is known) - start at byte N, end at byte 
 *
 */
extern "C"
CAMLprim value
atcf_bunzip_block_no_rle_decompress(value bz, value ba)
{
    CAMLparam2(bz, ba);
    c_bunzip *cb = bunzip_of_val(bz);
    t_uint8 *data = (t_uint8 *)(Caml_ba_array_val(ba)->data);
    CAMLreturn(Val_long(cb->decompress_no_rle(data,Caml_ba_array_val(ba)->dim[0])));
}

