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

(*a Test stuff *)

(*a Helper functions *)
(*f sfmt <string format> <arg> * -> string
 *    formats a string with arguments
 *)
let sfmt = Printf.sprintf

(*a Global variables *)
(* epsilon is how close floats have to be to be 'the same' *)
let epsilon = 0.00000001

(* +-sqrt(2) and their reciprocals, useful for sin/cos(pi/4) *)
let rt2 = sqrt 2.0
let n_rt2 = -. rt2
let rrt2 = 1.0 /. rt2
let n_rrt2 = -. rrt2

(* sin/cos 30 *)
let c30 = (sqrt 3.0) /. 2.0
let s30 = 0.5

(* Some globally useful unit vectors *)
let x3 = (Vector.make3 1.0 0.0 0.0)
let y3 = (Vector.make3 0.0 1.0 0.0)
let z3 = (Vector.make3 0.0 0.0 1.0)
let xy3 = Vector.normalize (Vector.make3 1.0 1.0 0.0)
let xz3 = Vector.normalize(Vector.make3 1.0 0.0 1.0)
let yz3 = Vector.normalize(Vector.make3 0.0 1.0 1.0)
let zero3 = (Vector.make3 0.0 0.0 0.0)
let rot30_z = 
  Matrix.(identity (make 3 3) |>
            set 0 0 c30 |>
            set 0 1 (-. s30) |>
            set 1 0 s30 |>
            set 1 1 c30 )

let mat_1234 = 
  Matrix.(identity (make 2 2) |>
            set 0 0 1.0 |>
            set 0 1 2.0 |>
            set 1 0 3.0 |>
            set 1 1 4.0)

(*a Useful functions *)
let rec fori (i:int) l f r =
    if (i<l) then (fori (i+1) l f (f i r)) else r

(*a Assertion functions, to simplify OUnit test code *)
(*f assert_equal_float : string -> float -> float -> unit
 *
 * Assert that the two floats are equal (or near enough),
 * with a message msg and the two float values
 *
 *)
let assert_equal_float msg v0 v1 =
  let diff = abs_float ( v0 -. v1 ) in
  let close_enough = (diff < epsilon) in
  assert_bool (sfmt "%s:%f:%f" msg v0 v1) close_enough

(*f assert_equal_int : string -> int -> int -> unit
 *
 * Assert that the two ints are equal with a message msg and the two
 * float values
 * 
 *)
let assert_equal_int msg v0 v1 =
  let close_enough = (v0 = v1) in
  assert_bool (sfmt "%s:%d:%d" msg v0 v1) close_enough

(*f assert_equal_quat : string -> quaternion -> float array -> unit
 *
 * Assert that a quaternion has certain r,i,j,k (or near enough),
 * with a message msg
 *
 *)
let assert_equal_quat msg q rijk =
    let qrijk = Quaternion.get_rijk q in
    assert_equal_float (sfmt "%s:r:" msg) qrijk.(0) rijk.(0) ;
    assert_equal_float (sfmt "%s:i:" msg) qrijk.(1) rijk.(1) ;
    assert_equal_float (sfmt "%s:j:" msg) qrijk.(2) rijk.(2) ;
    assert_equal_float (sfmt "%s:k:" msg) qrijk.(3) rijk.(3)

(*f assert_coords : vector -> float array -> unit
 *
 * Assert that coordinates of the vector are close enough to the float array
 * 
 *)
let assert_coords v cs =
  let assert_coord = fun i a -> assert_equal_float (sfmt "Coord %d" i) a cs.(i) in
  Array.iteri assert_coord (Vector.coords v)

(*f assert_vector : vector -> vector -> unit
 *
 * Assert that coordinates of the vector are close enough to the other vector
 * 
 *)
let assert_vector v cs =
  assert_coords v (Vector.coords cs)

(*a All test suites, toplevel *)
let test_suites = "All tests" >::: [
     Test_vector.test_suite_vector ;
     Test_matrix.test_suite_matrix ;
     (*test_suite_quaternion ;
     test_suite_matrix ;*)
    ]


let _ = 
    at_exit Gc.full_major ;
    run_test_tt_main  test_suites ;
