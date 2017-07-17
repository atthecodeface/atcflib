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
open Bigarray
open Unix

type c_bunzip
type uint8_array = (int, int8_unsigned_elt, c_layout) Bigarray.Genarray.t

external bz_create    : unit -> c_bunzip   = "atcf_bunzip_create"
external bz_destroy   : c_bunzip -> unit   = "atcf_bunzip_destroy"
external bz_set_size  : c_bunzip -> int -> unit   = "atcf_bunzip_set_size"
external bz_block_data           : c_bunzip -> uint8_array -> int64 -> int64 -> int   = "atcf_bunzip_block_data"
external bz_block_read_header    : c_bunzip -> int                 = "atcf_bunzip_block_read_header"
external bz_block_huffman_decode : c_bunzip -> int                 = "atcf_bunzip_block_huffman_decode"
external bz_block_size           : c_bunzip -> int                 = "atcf_bunzip_block_size"
external bz_block_start_bit      : c_bunzip -> int64               = "atcf_bunzip_block_start_bit"
external bz_block_end_bit        : c_bunzip -> int64               = "atcf_bunzip_block_end_bit"
external bz_block_magic          : c_bunzip -> int64               = "atcf_bunzip_block_magic"
external bz_block_mtf            : c_bunzip -> int                 = "atcf_bunzip_block_mtf"
external bz_block_bwt_order      : c_bunzip -> int                 = "atcf_bunzip_block_bwt_order"
external bz_block_no_rle_size    : c_bunzip -> int                 = "atcf_bunzip_block_no_rle_size"
external bz_block_no_rle_decompress : c_bunzip -> uint8_array -> int = "atcf_bunzip_block_no_rle_decompress"
(*external bz_block_decompress     : c_bunzip -> int                 = "atcf_bunzip_block_decompress"*)

module Indexentry = struct
    type t = {
    mutable no_rle_start   : int64 ;
    mutable rle_start      : int64 ;
    mutable bz_start_bit   : int64 ;
    mutable bz_num_bits    : int32 ;
    mutable no_rle_length  : int32 ;
    mutable rle_length     : int32 ;
    mutable block_crc      : int32 ;
    mutable decomp_data    : int32 ;
    mutable user_data      : (int32 * int32 * int32 * int32 * int32)
    }
    let create _ = { no_rle_start = 0L ;
                     rle_start = 0L ;
                     bz_start_bit = 0L ;
                     bz_num_bits = 0l ;
                     no_rle_length = 0l ;
                     rle_length = 0l ;
                     block_crc = 0l ;
                     decomp_data = 0l ;
                     user_data = (0l, 0l, 0l, 0l, 0l);
                   }
    let bz_bit_pos i bz_sb bz_eb = 
        i.bz_start_bit <- bz_sb; 
        i.bz_num_bits  <- Int64.to_int32 (Int64.sub bz_eb bz_sb) ;
        ()
    let no_rle i pi l = 
        i.no_rle_start <- Int64.add pi.no_rle_start (Int64.of_int32 pi.no_rle_length) ;
        i.no_rle_length <- Int32.of_int l ;
        ()
    let ( >>= ) x f =
      match x with
        Ok v         -> f v
      | Error _ as e -> e
    let chk_error m x = if x=0 then Ok x else Error m
    let get_bzip_block_data bz ba start_bit =
      chk_error "data" (bz_block_data bz ba start_bit 100000L) >>= fun _ ->
      chk_error "hdr"  (bz_block_read_header bz)              >>= fun _ ->
      chk_error "huf"  (bz_block_huffman_decode bz)           >>= fun _ ->
      Ok ((bz_block_start_bit bz),(bz_block_end_bit bz),(bz_block_no_rle_size bz))

    let rec build_index_r bz ba start_bit verbose n prev =
      (*if (n>100) then [] else begin*)
      verbose n prev ;
      let r = get_bzip_block_data bz ba start_bit in
      match r with
        Error e -> Printf.printf "Error %s\n" e ; []
      | Ok d    -> let (i,e) = index_entry bz prev in (i::(build_index_r bz ba e verbose (n+1) i))
     (*end*)
    and index_entry bz prev =
        let i = create () in
        bz_bit_pos i (bz_block_start_bit bz) (bz_block_end_bit bz) ;
        no_rle i prev (bz_block_no_rle_size bz) ;
        (i,(bz_block_end_bit bz))

    let rec write_int_n i max_i f n =
        let b i = (Int64.to_int (Int64.shift_right n i)) land 255 in
        output_byte f (b i) ;
        if (i<max_i-8) then write_int_n (8+i) max_i f n 
    let write_int64 f n = write_int_n 0 64 f n
    let write_int32 f n = write_int_n 0 32 f (Int64.of_int32 n) 
    let write i f = 
        let ua,ub,uc,ud,ue = i.user_data in
        write_int64 f i.no_rle_start ;
        write_int64 f i.rle_start  ;
        write_int64 f i.bz_start_bit  ;
        write_int32 f i.bz_num_bits  ;
        write_int32 f i.no_rle_length  ;
        write_int32 f i.rle_length  ;
        write_int32 f i.block_crc  ;
        write_int32 f i.decomp_data ;
        write_int32 f ua ;
        write_int32 f ub ;
        write_int32 f uc ;
        write_int32 f ud ;
        write_int32 f ue 
    let rec read_int_n i max_i b n v =
        let vb = Int64.of_int(Char.code(Bytes.get b n)) in
        let nv = Int64.logor v (Int64.shift_left vb i) in
        if (i>=max_i-8) then nv else read_int_n (8+i) max_i b (n+1) nv
    let read_int64 b n = read_int_n 0 64 b n 0L
    let read_int32 b n = Int64.to_int32(read_int_n 0 32 b n 0L)
    let read f =
        let b = Bytes.create 64 in
        if ((input f b 0 64)<64) then
          None
        else
          Some { no_rle_start  = read_int64 b 0 ;
            rle_start     = read_int64 b 8 ;
            bz_start_bit  = read_int64 b 16 ;
            bz_num_bits   = read_int32 b 24 ;
            no_rle_length = read_int32 b 28 ;
            rle_length    = read_int32 b 32 ;
            block_crc     = read_int32 b 36 ;
            decomp_data   = read_int32 b 40 ;
            user_data     = (0l, 0l, 0l, 0l, 0l);
          }
    let str i =
      Printf.sprintf "%20Ld +%8ld : %20Ld +%8ld" i.bz_start_bit i.bz_num_bits i.no_rle_start i.no_rle_length
end
module Index = struct
    type t = {
    block_size : int ;
    entries    : Indexentry.t list ;
    }
    let verbose_progress n i = Printf.printf "%d:Bz at bit %Ld : %ld : %Ld\r%!" n i.Indexentry.bz_start_bit i.Indexentry.bz_num_bits i.Indexentry.no_rle_start
    let quiet_progress n i = ()
    let build_index bz ba verbose = { 
        block_size = (bz_block_size bz) ;
        entries = 
          let progress_fn verbose = if verbose then verbose_progress else quiet_progress in
          Indexentry.build_index_r bz ba 32L (progress_fn verbose) 0 (Indexentry.create ())
      }
    let show f i = 
       f (Printf.sprintf "Block size %d" i.block_size) ;
       let df n ie = f (Printf.sprintf "%8d: %s" n (Indexentry.str ie)) in
       List.iteri df i.entries
    let write f i =
       let wf n ie = Indexentry.write ie f in
       List.iteri wf i.entries
    let create filename verbose = 
        let open_read filename = Unix.openfile filename [O_RDONLY ;] 0 in
        let fd = open_read "../../device_analyzer_data/8926ff5477452ba9aea697f796e7d3570195576f.csv.bz2" in
        let ba = Bigarray.Genarray.map_file fd (*pos:(int64 0)*) Bigarray.Int8_unsigned c_layout false [|-1;|] in
        let bz = bz_create () in
        let ba0 = Bigarray.Genarray.get ba [|0;|] in
        let ba1 = Bigarray.Genarray.get ba [|1;|] in
        let ba2 = Bigarray.Genarray.get ba [|2;|] in
        let ba3 = Bigarray.Genarray.get ba [|3;|] in
        if ((ba0==0x42) &&
            (ba1==0x5a) &&
            (ba2==0x68) &&
            true
           ) then begin
          bz_set_size bz (ba3-48) ;
        Some (build_index bz ba verbose)
        end else
          None
    let read filename verbose =
      let f = open_in_bin "8926ff5477452ba9aea697f796e7d3570195576f.csv.bz2.index" in
      let rec read_entries f entries =
        match (Indexentry.read f) with
        Some e -> read_entries f (entries@[e])
        | None   -> entries
      in
      { block_size=9 ;
        entries = (read_entries f []) ;
      }
end

let old_create_and_show () = 
  let index = Index.create "../../device_analyzer_data/8926ff5477452ba9aea697f796e7d3570195576f.csv.bz2" true in
  let f2 = open_out_bin "8926ff5477452ba9aea697f796e7d3570195576f.csv.bz2.index" in
  match index with
    Some i ->  begin Index.write f2 i ;
                     let f s = Printf.printf "%s\n" s in
                     Index.show f i
               end
  | None  -> ()

let _ = 
    let index = Index.read "8926ff5477452ba9aea697f796e7d3570195576f.csv.bz2.index" true in
    let f s = Printf.printf "%s\n" s in
    Index.show f index


