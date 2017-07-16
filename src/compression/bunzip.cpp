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
 * @file          bunzip.cpp
 * @brief         Bzip decompression methods
 *
 */

/*a Includes
 */
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include "bunzip.h"

/*a Defines
 */
#define BZIP2_GROUP_SIZE 50
#define BZIP2_SYMBOL_RUNA 0
#define BZIP2_SYMBOL_RUNB 1

/*a Huffman table functions
 */
/*f huffman_init - initialize Huffman table
 */
static void
huffman_init(t_huffman_table *ht)
{
    memset(ht, 0, sizeof(*ht));
    ht->num_symbols = 0;
    ht->min_length = 255;
    ht->max_length = 0;
}

/*f huffman_next_symbol_length - inform table of length in bits of next encoded symbol
 */
static void
huffman_next_symbol_length(t_huffman_table *ht, int l)
{
    ht->lengths[ht->num_symbols++] = l;
    if (l<ht->min_length) ht->min_length=l;
    if (l>ht->max_length) ht->max_length=l;
}

/*f huffman_decode_bits - decode top-most bits and return symbol and number of bits used
 */
static t_uint32
huffman_decode_bits(t_huffman_table *ht, t_uint32 bits)
{
    t_uint32 ordered_code;
    ordered_code = 0;
    for (unsigned int l=ht->min_length; l<=ht->max_length; l++) {
        unsigned int code;
        code = bits>>(32-l);
        if (code<ht->code_max[l]) {
            ordered_code += code-ht->code_base[l];
            return (ht->symbol_of_order[ordered_code]<<8) | l;
        }
        ordered_code += ht->count_of_lengths[l];
    }
    return -1;
}

/*f huffman_finalize - all symbols added, build decode table
 */
static void 
huffman_finalize(t_huffman_table *ht)
{
    int n=0;
    int base_code;

    for (unsigned int l=ht->min_length; l<=ht->max_length; l++) {
        for (unsigned int j=0;j<ht->num_symbols;j++) {
            if (ht->lengths[j]==l) {
                ht->order_of_symbol[j]=n;
                ht->symbol_of_order[n]=j;
                ht->count_of_lengths[l]++;
                n++;
            }
        }
    }

    base_code = 0;
    for (unsigned int l=ht->min_length; l<=ht->max_length; l++) {
        base_code <<= 1; // add a bit to the bottom of the base code
        ht->code_base[l] = base_code;
        ht->code_max[l]  = base_code + ht->count_of_lengths[l];
        base_code += ht->count_of_lengths[l]; // move past the last code
    }

    base_code = 0;
    n=0;
    for (unsigned int l=ht->min_length; l<=ht->max_length; l++) {
        for (base_code = ht->code_base[l]; base_code<ht->code_max[l]; base_code++) {
            ht->code_of_symbol[ht->symbol_of_order[n]] = (base_code<<8) | l;
            n+=1;
        }
    }

    for (int i=0; i<__BZ2__HUFFMAN_DIRECT_MAP_SIZE; i++) {
        t_uint32 cl=huffman_decode_bits(ht,i<<(32-__BZ2__HUFFMAN_DIRECT_MAP_BITS));
        if ((cl&0xff)<=__BZ2__HUFFMAN_DIRECT_MAP_BITS) {
            ht->direct_map_decode[i] = cl;
        } else {
            ht->direct_map_decode[i] = 0x80000000UL | (cl&0xff) | 0;
        }
    }

}

/*f huffman_encode - not used (since this is for decompression) but returns symbol of code
static unsigned int 
huffman_encode(t_huffman_table *ht, unsigned char symbol)
{
    return ht->code_of_symbol[symbol];
}
 */

/*f huffman_decode - decode top-most bits and return symbol and number of bits used
 */
static t_uint32
huffman_decode_data(t_huffman_table *ht, t_uint32 bits)
{
    t_uint32 cl;
    cl = ht->direct_map_decode[bits>>(32-__BZ2__HUFFMAN_DIRECT_MAP_BITS)];
    if (!(cl&0x80000000UL)) {
        return cl;
    }
    return huffman_decode_bits(ht, bits);
}

/*a Bunzip constructors
 */
/*f c_bunzip::c_bunzip
 */
c_bunzip::c_bunzip(void)
{
    _cd.mapped_length = 0;
    data_buffer = NULL;
    order_buffer = NULL;
    data_buffer_to_be_freed = 0;
}

/*f c_bunzip::~c_bunzip
 */
c_bunzip::~c_bunzip(void)
{
    reset();
}

/*f c_bunzip::reset
 */
void
c_bunzip::reset(void)
{
    block_size = 0;
    if (_cd.mapped_length>0) {
        munmap((void *)_compressed_data, _cd.mapped_length);
        _cd.mapped_length = 0;
    }
    if (data_buffer_to_be_freed) {
        if (data_buffer)
            free(data_buffer);
        data_buffer = NULL;
        data_buffer_to_be_freed = 0;
    }
    data_buffer_used = 0;
}

/*f c_bunzip::ensure_data_buffer
 * Return 1 if data buffer is set up
 */
int
c_bunzip::ensure_data_buffer(void)
{
    if (data_buffer==NULL) {
        data_buffer = (t_uint8 *)malloc(block_size*100*1000*sizeof(t_uint8));
        order_buffer = (t_uint32 *)malloc(block_size*100*1000*sizeof(t_uint32));
        data_buffer_to_be_freed = 1;
    }
    return (data_buffer!=NULL);
}

/*f c_bunzip::map_file_block
 * Return 0 on success
 */
int
c_bunzip::map_file_block(int fd, size_t starting_bit, size_t ending_bit )
{
    int pagesize = getpagesize();

    _cd.fd = fd;
    _cd.mapped_length = ((ending_bit+7)-(starting_bit&~7))/8;
    _cd.start_byte = (starting_bit/8) & ~(size_t)(pagesize-1);
    _cd.pos = (starting_bit/8) - _cd.start_byte;
    _cd.bit = 8-(starting_bit%8);
    _compressed_data = (const t_uint8 *)mmap( NULL,
                                              _cd.mapped_length,
                                              PROT_READ, MAP_FILE|MAP_PRIVATE,
                                              _cd.fd,
                                              _cd.start_byte );
    if (_compressed_data==MAP_FAILED) {
        _compressed_data = NULL;
        _cd.mapped_length = 0;
        return -1;
    }
    return use_data(_compressed_data, starting_bit, ending_bit);
}

/*f c_bunzip::use_data
 * Return 0 on success
 */
int
c_bunzip::use_data(const t_uint8 *data, size_t starting_bit, size_t ending_bit )
{
    _cd.starting_bit = starting_bit;
    _cd.pos = starting_bit/8;
    _cd.bit = 8-(starting_bit%8);
    _cd.length = ((ending_bit+7)-(starting_bit&~7))/8;
    _compressed_data = data;
    return 0;
}

/*f c_bunzip::read_data_bits
 */
inline t_uint32
c_bunzip::read_data_bits(int n)
{
    t_uint32 r;
    r = 0;
    for (int i=0; i<n; i++) {
        _cd.bit--;
        if (_cd.bit<0) {
            _cd.bit=7;
            _cd.pos++;
        }
        r = (r<<1) | ((_compressed_data[_cd.pos] >> _cd.bit)&1);
    }
    return r;
}

/*f c_bunzip::in_data_pos - local function for reporting where in the data one is
 */
inline size_t
c_bunzip::in_data_pos(void)
{
    return (_cd.start_byte + _cd.pos)*8 + (8-_cd.bit);
}

/*f c_bunzip::read_symbol_map */
void
c_bunzip::read_symbol_map(void)
{
    // Unpack the bitmap of used symbols (in Huffman stream, so because of the MTF step 0, 1 etc are used)
    t_uint32 ranges_used   = read_data_bits(16);
    int n = 0;
    for (int i=15; i>=0; i--) {
        if (ranges_used & (1<<i)) {
            t_uint32 bitmap = read_data_bits(16);
            for (int j=15; j>=0; j--) {
                if (bitmap & (1<<j)) {
                    symbol_map[n++] = 255-j-16*i;
                }
            }
        }
    }
    number_symbols = n;
}

/*f c_bunzip::read_selectors
 */
void
c_bunzip::read_selectors(void)
{
    t_uint8 mtf[8];
    for (int i=0; i<number_huffman_tables; i++) {
        mtf[i] = i;
    }

    for (int i=0; i<number_selectors; i++) {
        int j=0, table;
        while (read_data_bits(1)==1) { j++; }
        if (j>number_huffman_tables) {
            //raise Exception("MTF selector for huffman table exceeds number of huffman tables")
        }
        table = mtf[j];
        memmove(mtf+1, mtf, j*1); // j*1 as mtf is t_uint8
        mtf[0] = table;
        selectors[i] = table;
    }
}

/*f c_bunzip::read_huffman_tables
 */
void
c_bunzip::read_huffman_tables(void)
{
    for (int i=0; i<number_huffman_tables; i++) {
        t_huffman_table *ht = &(huffman_tables[i]);
        huffman_init(ht);
        t_uint32 symbol_length = read_data_bits(5);
        for (int j=0; j<number_huffman_symbols; j++) {
            while (1) {
                if ((symbol_length<1) || (symbol_length>20)) {
                    // abort
                }
                if (read_data_bits(1)==0) break;
                symbol_length += read_data_bits(1)?-1:1;
            }
            huffman_next_symbol_length(ht, symbol_length);
        }
        huffman_finalize(ht);
    }
}

/*f c_bunzip::read_header
 * Call after bunzip::read_block
 * Should probably check magic1/2, randomized==0, and orig_ptr is bounded properly (0...100000*block_size-1)
 * Should check number of huffman tables is 2 to 6
 */
int
c_bunzip::read_header(void)
{
    block_magic1        = read_data_bits(32); // should be 0x31415926
    block_magic2        = read_data_bits(16); // should be 0x5359
    block_crc           = read_data_bits(32);
	block_randomized    = read_data_bits(1);  // Not supported if 1
    bwt_orig_offset     = read_data_bits(24); // after inverse BWT, this is where the data starts
    fprintf(stderr,"Here %x %x %p\n",block_magic1, block_magic2,_compressed_data);
    if (block_magic1 != 0x31415926) return 1;
    if (block_magic2 != 0x5359) return 1;
    fprintf(stderr,"Got through\n");

    read_symbol_map();
    number_huffman_symbols = number_symbols+2; // as Huffman has to encode RUNA and RUNB and the end, but not 0
                 
	number_huffman_tables = read_data_bits(3); // Spec says 2 to 6
    number_selectors      = read_data_bits(15); // Number of Huffman table selectors, 50 symbols per choice

    read_selectors();

    read_huffman_tables();

    return 0;
}

/*f c_bunzip::huffman_decode
*/
int
c_bunzip::huffman_decode(void)
{
    if (!ensure_data_buffer()) return -1;
    
    int current_selector=0;
    int run_length = 0;
    int run_length_exp = 1;
    t_uint8 *data_ptr = data_buffer;
    int data_bits_valid = 0;
    int data_bits = 0;
    const t_uint8 *huffman_data = _compressed_data + _cd.pos;
    data_bits_valid = _cd.bit;
    data_bits = (huffman_data[0] << (8-data_bits_valid))<<24;
    huffman_data++;
    int finished = 0;
    while (!finished) {
        t_huffman_table *ht = huffman_tables + selectors[current_selector];
        //fprintf(stderr,"Sel %d\n",selectors[current_selector]);
        current_selector++;
        for (int i=BZIP2_GROUP_SIZE; (i>0) && (!finished); i--) {
            t_uint32 symbol;
            int number_bits_used;
            while (data_bits_valid<24) {
                data_bits |= huffman_data[0]<<(24-data_bits_valid);
                data_bits_valid += 8;
                huffman_data++;
            }
            t_uint32 sl = huffman_decode_data(ht, data_bits);
            number_bits_used = sl&0xff;
            symbol = sl>>8;
            data_bits_valid -= number_bits_used;
            data_bits       <<= number_bits_used;
            if (symbol<=BZIP2_SYMBOL_RUNB) {
                run_length += ((symbol==BZIP2_SYMBOL_RUNB)?2:1)*run_length_exp;
                run_length_exp = run_length_exp<<1;
            } else {
                if (run_length) {
                    for (int j=0; j<run_length; j++) {
                        *data_ptr++=0;
                    }
                    run_length = 0;
                    run_length_exp = 1;
                }
                if (symbol==number_huffman_symbols-1) {
                    data_buffer_used = data_ptr - data_buffer;
                    finished = 1;
                    break;
                }
                *data_ptr++ = symbol-1;
            }
        }
    }
    _cd.pos = huffman_data - _compressed_data;
    _cd.bit = data_bits_valid+8;
    while (_cd.bit>8) {
        _cd.pos --;
        _cd.bit -= 8;
    }
    return 0;
}

/*f c_bunzip::mtf
*/
int
c_bunzip::mtf(void)
{
    t_uint8 mtf[256];
    t_uint8 *data_ptr = data_buffer;
    for (int i=0; i<256; i++) {
        mtf[i]=i;
        symbol_counts[i] = 0;
    }
    for (int i=0; i<data_buffer_used; i++) {
        t_uint8 k = *data_ptr;
        t_uint8 m = mtf[k];
        t_uint8 s = symbol_map[m];
        *data_ptr = s;
        data_ptr++;
        symbol_counts[s] += 1;
        if (k!=0) {
            memmove(mtf+1, mtf, k*1);
            mtf[0] = m;
        }
    }
    return 0;
}

/*f c_bunzip::bwt_order
 *
 * Derive the order of the characters in the data buffer
 *
 * Don't actually do the decompression, which requires a lot of
 * dependent reads
 */
int
c_bunzip::bwt_order(void)
{
    if (order_buffer==NULL) {
        order_buffer = (t_uint32 *)malloc(block_size*100*1000*sizeof(t_uint32));
    }
    if (order_buffer==NULL) {
        return -1;
    }

    int n=0;
    t_uint32 symbol_numbering[256];
    for (int i=0; i<256; i++) {
        symbol_numbering[i] = n;
        n += symbol_counts[i];
    }

    for (int i=0; i<data_buffer_used; i++) {
        t_uint8 s = data_buffer[i];
        order_buffer[symbol_numbering[s]] = i; // forward links in the order buffer
        symbol_numbering[s]++;
    }
    return 0;
}

/*f bzip2_bwt_order_reverse
 *
 * Generate an ordering of the symbols in the buffer in reverse
 *
 */
int
c_bunzip::bwt_order_reverse(void)
{
    if (order_buffer==NULL) {
        order_buffer = (t_uint32 *)malloc(block_size*100*1000*sizeof(t_uint32));
    }
    if (order_buffer==NULL) {
        return -1;
    }

    int n=0;
    t_uint32 symbol_numbering[256];
    for (int i=0; i<256; i++) {
        symbol_numbering[i] = n;
        n += symbol_counts[i];
    }

    for (int i=0; i<data_buffer_used; i++) {
        t_uint8 s = data_buffer[i];
        order_buffer[i] = symbol_numbering[s]; // backward links in the order buffer
        symbol_numbering[s]++;
    }
    return 0;
}

/*f c_bunzip::bwt_decompress
 *
 * Decompress in forward order or reverse order
 *
 */
int
c_bunzip::bwt_decompress(t_uint8 *output_buffer, t_uint32 order_start, int length, int reverse)
{
    t_uint32 p;
    p = order_start;
    if (reverse) {
        for (int i=length-1; i>=0; i--) {
            output_buffer[i] = data_buffer[p];
            p = order_buffer[p];
        }
    } else {
        for (int i=0; i<length; i++) {
            output_buffer[i] = data_buffer[p];
            p = order_buffer[p];
        }
    }
    return 0;
}

/*f c_bunzip::decompress_no_rle
 *
 * Decompress in forward order
 *
 */
int
c_bunzip::decompress_no_rle(t_uint8 *output_buffer, int buffer_length)
{
    t_uint32 p = order_buffer[bwt_orig_offset];
    if (buffer_length>data_buffer_used) buffer_length=data_buffer_used;
    for (int i=0; i<buffer_length; i++) {
        output_buffer[i] = data_buffer[p];
        p = order_buffer[p];
    }
    return 0;
}
