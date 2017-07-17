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

let old_create_and_show () = 
  let index = Atcflib.Bunzip.Index.create "../../device_analyzer_data/8926ff5477452ba9aea697f796e7d3570195576f.csv.bz2" true in
  let f2 = open_out_bin "8926ff5477452ba9aea697f796e7d3570195576f.csv.bz2.index" in
  match index with
    Some i ->  begin Atcflib.Bunzip.Index.write f2 i ;
                     let f s = Printf.printf "%s\n" s in
                     Atcflib.Bunzip.Index.show f i
               end
  | None  -> ()

let _ = 
    let index = Atcflib.Bunzip.Index.read "8926ff5477452ba9aea697f796e7d3570195576f.csv.bz2.index" true in
    let f s = Printf.printf "%s\n" s in
    Atcflib.Bunzip.Index.show f index


