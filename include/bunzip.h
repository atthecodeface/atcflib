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
 * @file          bunzip.h
 * @brief         Bzip decompression methods
 *
 */

/*a Wrapper
 */
#ifdef __INC_BUNZIP
#else
#define __INC_BUNZIP

/*a Defines
 */
#define __BZ2__HUFFMAN_MAX_CODE_LENGTH 20
#define __BZ2__HUFFMAN_MAX_SYMBOLS     260
#define __BZ2__HUFFMAN_DIRECT_MAP_BITS 6
#define __BZ2__HUFFMAN_DIRECT_MAP_SIZE (1<<__BZ2__HUFFMAN_DIRECT_MAP_BITS)

/*a Types
 */
typedef unsigned char  t_uint8;
typedef unsigned short t_uint16;
typedef unsigned int   t_uint32;
typedef unsigned long long   t_uint64;

/*t t_huffman_table
 */
typedef struct {
    int num_symbols;
    unsigned char min_length;
    unsigned char max_length;
    unsigned char lengths[__BZ2__HUFFMAN_MAX_SYMBOLS];
    unsigned int  order_of_symbol[__BZ2__HUFFMAN_MAX_SYMBOLS];  // must be big enough to encode a symbol
    unsigned int  symbol_of_order[__BZ2__HUFFMAN_MAX_SYMBOLS];  // must be big enough to encode a symbol
    t_uint32 code_of_symbol[__BZ2__HUFFMAN_MAX_SYMBOLS];  // must be big enough to encode a code and length
    unsigned char count_of_lengths[__BZ2__HUFFMAN_MAX_CODE_LENGTH]; // possibly should be large enough to encode count of all symbols
    t_uint32 code_base[__BZ2__HUFFMAN_MAX_CODE_LENGTH];
    t_uint32 code_max[__BZ2__HUFFMAN_MAX_CODE_LENGTH];
    t_uint32 direct_map_decode[__BZ2__HUFFMAN_DIRECT_MAP_SIZE];
} t_huffman_table;

class c_bunzip
{
    const t_uint8 *_compressed_data;
    struct {
        int length; // 
        int fd;
        size_t start_byte; // where the start of compressed data maps to (0 if not a mmaped)
        int mapped_length; // 0 if not mmapped
        t_uint64 starting_bit;
        int bit_offset;
        t_uint64 pos; // within buffer
        int bit; // last comsumed bit within byte - 8 is left most bit (which is first in the file)
    } _cd;
    unsigned char *_decompressed_data;
public:
    int block_size;
    int bwt_orig_offset;
    int number_huffman_tables;
    int number_selectors;
    t_uint8  symbol_map[256];
    t_uint32 symbol_counts[256];
    int number_symbols;
    int number_huffman_symbols;
    t_uint8 *data_buffer;
    t_uint32 *order_buffer;
    int data_buffer_to_be_freed;
    int data_buffer_used;
    t_uint8 selectors[32768];
    t_huffman_table huffman_tables[6];
    t_uint32 block_magic1;
    t_uint32 block_magic2;
    t_uint32 block_crc;
    int block_randomized;

    void init(void);
    c_bunzip(void);
    ~c_bunzip(void);
    void reset(void);
    inline void set_size(int b) { block_size=b; }
    inline t_uint64 block_magic(void) { return (((t_uint64) block_magic1)<<16) | block_magic2; }
    inline t_uint64 block_start_bit(void) { return _cd.starting_bit; }
    inline t_uint64 block_end_bit(void)   { return (_cd.pos*8)+(8-_cd.bit); }
    int ensure_data_buffer(void);
    int map_file_block(int fd, size_t starting_bit, size_t ending_bit );
    int use_data(const t_uint8 *data, size_t starting_bit, size_t ending_bit );
    inline t_uint32 read_data_bits(int n);
    inline size_t in_data_pos(void);
    void read_symbol_map(void);
    void read_selectors(void);
    void read_huffman_tables(void);
    int read_header(void);
    int huffman_decode(void);
    int mtf(void);
    int bwt_order(void);
    int bwt_order_reverse(void);
    int bwt_decompress(t_uint8 *output_buffer, t_uint32 order_start, int length, int reverse);
    int decompress_no_rle(t_uint8 *output_buffer, int buffer_length);

};

/*a External functions
 */

/*a Wrapper
 */
#endif

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/
