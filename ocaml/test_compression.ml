(** Copyright (C) 2017,  Gavin J Stark.  All rights reserved.
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
 * @file          test_atcf.ml
 * @brief         Test suite for the Atcflib libraries
 *
 *)

(*a Documentation
 * 
 * This is a test suite for the ATCF library
 *
 *)
(*a Libraries used
 *
 * Atcflib is used as that is being tested
 *
 * OUnit is required as the test framework
 *
 *)
open Atcflib
open OUnit
open Bigarray
open Unix

type c_bunzip
type uint8_array = (char, int8_unsigned_elt, c_layout) Bigarray.Genarray.t

external bz_create    : unit -> c_bunzip   = "atcf_bunzip_create"
external bz_destroy   : c_bunzip -> unit   = "atcf_bunzip_destroy"
external bz_set_size  : c_bunzip -> int -> unit   = "atcf_bunzip_set_size"
external bz_block_data           : c_bunzip -> uint8_array -> int -> int -> int   = "atcf_bunzip_block_data"
external bz_block_read_header    : c_bunzip -> int                 = "atcf_bunzip_block_read_header"
external bz_block_huffman_decode : c_bunzip -> int                 = "atcf_bunzip_block_huffman_decode"
external bz_block_start_bit      : c_bunzip -> int64               = "atcf_bunzip_block_start_bit"
external bz_block_end_bit        : c_bunzip -> int64               = "atcf_bunzip_block_end_bit"
external bz_block_magic          : c_bunzip -> int64               = "atcf_bunzip_block_magic"
external bz_block_mtf            : c_bunzip -> int                 = "atcf_bunzip_block_mtf"
external bz_block_bwt_order      : c_bunzip -> int                 = "atcf_bunzip_block_bwt_order"
external bz_block_no_rle_size    : c_bunzip -> int                 = "atcf_bunzip_block_no_rle_size"
external bz_block_no_rle_decompress : c_bunzip -> uint8_array -> int = "atcf_bunzip_block_no_rle_decompress"
(*external bz_block_decompress     : c_bunzip -> int                 = "atcf_bunzip_block_decompress"*)

let open_read filename = Unix.openfile filename [O_RDONLY ;] 0

let fd = open_read "../../device_analyzer_data/8926ff5477452ba9aea697f796e7d3570195576f.csv.bz2"
let ba = Bigarray.Genarray.map_file fd (*pos:(int64 0)*) char c_layout false [|-1;|]
let bz = bz_create ()
let _ =  bz_set_size bz 9 ;
         ignore (bz_block_data bz ba 32 100000) ;
         ignore (bz_block_read_header bz) ;
          Printf.printf "Magic %Lx\n" (bz_block_magic bz);
         ignore (bz_block_huffman_decode bz) ;
         ignore (bz_block_mtf bz) ;
         ignore (bz_block_bwt_order bz)
let _ = Printf.printf "no-rle size %d\n" (bz_block_no_rle_size bz)
let decomp = Bigarray.Genarray.create char c_layout [|(bz_block_no_rle_size bz);|]
let decomp_c = Bigarray.array1_of_genarray decomp
let _ = bz_block_no_rle_decompress bz decomp ;
    for i=0 to 500 do
      Printf.printf "%c" (Bigarray.Array1.get decomp_c i)
    done ;
    Printf.printf "\n" ;
    Printf.printf "start %Ld end %Ld\n" (bz_block_start_bit bz) (bz_block_end_bit bz)


let _ = Printf.printf "%d\n" (bz_block_data bz ba 613165 10000000) ;
    Printf.printf "start %Ld end %Ld\n" (bz_block_start_bit bz) (bz_block_end_bit bz) ;
          Printf.printf "%d\n" (bz_block_read_header bz) ;
          Printf.printf "%Lx\n" (bz_block_magic bz);
         ignore (bz_block_huffman_decode bz) ;
         ignore (bz_block_mtf bz) ;
         ignore (bz_block_bwt_order bz)

let _ = bz_block_no_rle_decompress bz decomp ;
    for i=0 to 500 do
      Printf.printf "%c" (Bigarray.Array1.get decomp_c i)
    done ;
    Printf.printf "\n" ;
    Printf.printf "start %Ld end %Ld\n" (bz_block_start_bit bz) (bz_block_end_bit bz)

