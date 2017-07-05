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
 * This is part of a test suite for the ATCF library
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

(*a Test suite *)
(*b time vector functions *)
let time_execution tim (f: unit -> unit) n =
  Timer.init tim ;
  Timer.entry tim ;
  for i=0 to n do
    f ()
  done ;
  Timer.exit tim

module type Vector = sig
    type t ={ cv : Atcflib.c_vector }
    val create : Atcflib.c_vector -> t
    val get_cv : t -> Atcflib.c_vector
    val scale  : t -> float  -> unit
    end
module Vector : Vector = struct
    type t = {cv:Atcflib.c_vector}
    let create (cv_in:Atcflib.c_vector) = {cv = cv_in}
    let get_cv m = m.cv
    let scale m f = Atcflib.v_scale m.cv f
end

let test_suite_vector = 
    "vector" >::: [
      ("timeit" >::
         fun ctxt ->
         let tim = Timer.create () in
         let v = mkvector2 1.0 2.0 in
         let cv = v#get_cv in
         let mv = Vector.create cv in
         let f  = fun () -> ignore (v#scale 1.0) in
         let f2 = fun () -> ignore (Vector.scale mv 1.0) in
         let f3 = fun () -> ignore (Atcflib.v_scale cv 1.0) in
         time_execution tim f (1000*1000*10) ;
         Printf.printf "Time taken per scale as object method in ns:%f\n" ((Timer.value_us tim) /. 10000.) ;
         time_execution tim f2 (1000*1000*10) ;
         Printf.printf "Time taken per scale using module call in ns:%f\n" ((Timer.value_us tim) /. 10000.) ;
         time_execution tim f3 (1000*1000*10) ;
         Printf.printf "Time taken per scale direct C call in ns:%f\n" ((Timer.value_us tim) /. 10000.) ;
         ()
      ) ;
    ]


(*a All test suites, toplevel *)
let test_suites = "All tests" >::: [
     test_suite_vector ;
    ]


let _ = 
    at_exit Gc.full_major ;
    run_test_tt_main  test_suites ;

