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

let biggie = Bigarray.Genarray.create float64 c_layout [|1000;|]
let b_v0_3_0_1 = (Vector.make_slice_array biggie 3 0 1)
let b_v1_3_1_1 = (Vector.make_slice_array biggie 3 1 1)

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

(*a Quaternion test suite *)
(*b Quaternion creation tests *)
let test_suite_quaternion_create = 
    "create" >::: [
        ("zero" >::
           fun ctxt ->
           let q = Quaternion.make () in
           assert_equal_quat "zero" q [|0.0; 0.0; 0.0; 0.0|]
        ) ;
        ("1234" >::
           fun ctxt ->
           let q = Quaternion.make_rijk 1.0 2.0 3.0 4.0 in
           assert_equal_quat "1234" q [|1.0; 2.0; 3.0; 4.0|]
        ) ;
        ("1234r" >::
           fun ctxt ->
           let q = Quaternion.reciprocal(Quaternion.make_rijk 1.0 2.0 3.0 4.0) in
           let m = Quaternion.modulus_squared q in
            Printf.printf "Modulus %f\n" m ;
           assert_equal_quat "1234r" q [|1.0*.m; (-. 2.0)*.m; (-. 3.0)*.m; (-. 4.0)*.m|]
        ) ;
        ("1234c" >::
           fun ctxt ->
           let q = Quaternion.(conjugate (make_rijk 1.0 2.0 3.0 4.0)) in
           assert_equal_quat "1234c" q [|1.0; -.2.0; -.3.0; -.4.0;|]
        ) ;
        ("1234c2" >::
           fun ctxt ->
           let q =  Quaternion.(conjugate(make_rijk 1.0 2.0 3.0 4.0)) in
           let q2 = Quaternion.make () in
           assert_equal_quat "1234c" q2 [|0.0; 0.0; 0.0; 0.0|] ;
           ignore (Quaternion.assign q q2 ); (* q2 <= q *)
           assert_equal_quat "1234c" q2 [|1.0; -.2.0; -.3.0; -.4.0;|] 
        ) ;
        ("rotation" >::
           fun ctxt ->
           let q =  Quaternion.make () in
           let q2 = Quaternion.make () in
           let v = Vector.copy(x3) in
           let x = Vector.copy(x3) in
           let y = Vector.copy(y3) in
           let z = Vector.copy(z3) in
           let (c,s) = Vector.assign_q_as_rotation v (Quaternion.assign_lookat_graphics z y q) in
           Printf.printf "\nQ %s" (Quaternion.str q) ;
           let (c,s) = Vector.assign_q_as_rotation v (Quaternion.assign_lookat_graphics z x q) in
           Printf.printf "\nQ of X %s" (Vector.str (Vector.apply_q q (Vector.make3 1.0 0.0 0.0)));
           Printf.printf "\nQ of Y %s" (Vector.str (Vector.apply_q q (Vector.make3 0.0 1.0 0.0)));
           Printf.printf "\nQ of Z %s" (Vector.str (Vector.apply_q q (Vector.make3 0.0 0.0 1.0)));
           Printf.printf "\nV %s" (Vector.str v) ;
           Printf.printf "\nCos/sin %f %f\n" c s
        ) ;
    ]
(*b Quaternion modulus tests *)
let test_suite_quaternion_modulus = 
    "modulus" >::: [
        ("test0" >::
           fun ctxt -> let q = Quaternion.make_rijk 1.0 0.0 0.0 0.0 in
           let qc = Quaternion.conjugate (Quaternion.copy q) in
           let qcq = Quaternion.postmultiply (Quaternion.copy q) qc in
           let r = (Quaternion.get_rijk qcq).(0) in
           assert_equal_float "m=r" (Quaternion.modulus_squared q) r ;
           assert_equal_float "m"   (Quaternion.modulus q) 1.0 ;
           assert_equal_float "msq" (Quaternion.modulus_squared q) ((Quaternion.modulus q) ** 2.)
        ) ;
        ("test1" >::
           fun ctxt -> let q = Quaternion.make_rijk 0.0 1.0 0.0 0.0 in
           assert_equal_float "m"   (Quaternion.modulus q) 1.0 ;
           assert_equal_float "msq" (Quaternion.modulus_squared q) ((Quaternion.modulus q) ** 2.)
        ) ;
        ("test2" >::
           fun ctxt -> let q = Quaternion.make_rijk 0.0 0.0 1.0 0.0 in
           assert_equal_float "m"   (Quaternion.modulus q) 1.0 ;
           assert_equal_float "msq" (Quaternion.modulus_squared q) ((Quaternion.modulus q) ** 2.)
        ) ;
        ("test3" >::
           fun ctxt -> let q = Quaternion.make_rijk 0.0 0.0 0.0 1.0 in
           assert_equal_float "m"   (Quaternion.modulus q) 1.0 ;
           assert_equal_float "msq" (Quaternion.modulus_squared q) ((Quaternion.modulus q) ** 2.)
        ) ;
        ("test4" >::
           fun ctxt -> let q = Quaternion.make_rijk 2.0 0.0 0.0 0.0 in
           let qc = Quaternion.conjugate (Quaternion.copy q) in
           let qcq = Quaternion.postmultiply (Quaternion.copy q) qc in
           let r = (Quaternion.get_rijk qcq).(0) in
           assert_equal_float "m=r" (Quaternion.modulus_squared q) r ;
           assert_equal_float "m"   (Quaternion.modulus q) 2.0 ;
           assert_equal_float "msq" (Quaternion.modulus_squared q) ((Quaternion.modulus q) ** 2.)
           
        ) ;
        ("test5" >::
           fun ctxt -> let q = Quaternion.make_rijk 0.0 2.0 0.0 0.0 in
           assert_equal_float "m"   (Quaternion.modulus q) 2.0 ;
           assert_equal_float "msq" (Quaternion.modulus_squared q) ((Quaternion.modulus q) ** 2.)
        ) ;
        ("test6" >::
           fun ctxt -> let q = Quaternion.make_rijk 0.0 0.0 2.0 0.0 in
           assert_equal_float "m"   (Quaternion.modulus q) 2.0 ;
           assert_equal_float "msq" (Quaternion.modulus_squared q) ((Quaternion.modulus q) ** 2.)
        ) ;
        ("test7" >::
           fun ctxt -> let q = Quaternion.make_rijk 0.0 0.0 0.0 2.0 in
           assert_equal_float "m"   (Quaternion.modulus q) 2.0 ;
           assert_equal_float "msq" (Quaternion.modulus_squared q) ((Quaternion.modulus q) ** 2.)
        ) ;
        ("test8" >::
           fun ctxt -> let q = Quaternion.make_rijk 3.0 4.0 12.0 84.0 in
           assert_equal_float "m"   (Quaternion.modulus q) 85.0 ;
           assert_equal_float "msq" (Quaternion.modulus_squared q) ((Quaternion.modulus q) ** 2.)
        ) ;
    ]
(*b Quaternion operation tests *)
let test_suite_quaternion_operation = 
    "operation" >::: [
        ("add_scaled" >::
           fun ctxt ->
           let q1 = Quaternion.make_rijk 1.0 2.0 3.0 4.0 in
           let q2 = Quaternion.make_rijk 4.0 3.0 2.0 1.0 in
           assert_equal_quat "q2+1*q1" (Quaternion.add_scaled 2.0 q1 q2) [|6.0; 7.0; 8.0; 9.0|] ;
           assert_equal_quat "zero"    (Quaternion.add_scaled (-.1.0) q1 q1) [|0.0; 0.0; 0.0; 0.0|] ;
           assert_equal_quat "q2+1*q1" (Quaternion.add_scaled 2.0 q1 q2) [|6.0; 7.0; 8.0; 9.0|]
        ) ;
        ("multiply" >::
           fun ctxt ->
           let q = Quaternion.make () in
           let q1 = Quaternion.make_rijk 1.0 2.0 3.0 4.0 in
           let q2 = Quaternion.make_rijk 4.0 3.0 2.0 1.0 in
           assert_equal_quat "q2*q1" Quaternion.(premultiply q1 (copy q2)) [|-.12.0; 6.0; 24.0; 12.0|] ;
           assert_equal_quat "q2*q1" Quaternion.(premultiply q1 (copy q2)) Quaternion.(get_rijk (postmultiply q2 (copy q1)));
           assert_equal_quat "q2*q1" Quaternion.(premultiply q2 (copy q1)) Quaternion.(get_rijk (postmultiply q1 (copy q2)));
           assert_equal_quat "q1*q2" Quaternion.(premultiply q2 (copy q1)) [|-.12.0; 16.0; 4.0; 22.0|] ;
           assert_equal_quat "q1*q2" Quaternion.(assign_q_q  q1 q2 (copy q)) Quaternion.(get_rijk (postmultiply q2 (copy q1)));
        ) ;
    ]
(*b Quaternion lookat tests *)
let test_lookat_graphics at up =
  let q = Quaternion.make () in
  ignore (Quaternion.assign_lookat_graphics at up q) ;
  Printf.printf "\n Check lookat maps at correctly to -Z:" ;
  assert_coords (Vector.(apply_q q (copy at))) [| 0.0; 0.0  ; (-. 1.0) |] ;
  Printf.printf "\n Check lookat maps up correctly to +Y:" ;
  assert_coords (Vector.(apply_q q (copy up))) [| 0.0; 1.0 ; 0.0 |]

let test_lookat_aeronautic at up =
  let q = Quaternion.make () in
  ignore (Quaternion.assign_lookat_aeronautic at up q) ;
  Printf.printf "\n Check lookat maps at correctly to +Z:" ;
  assert_coords Vector.(apply_q q (copy at)) [| 0.0; 0.0 ; 1.0 |] ;
  Printf.printf "\n Check lookat maps up correctly to +X:" ;
  assert_coords Vector.(apply_q q (copy up)) [| 1.0; 0.0 ; 0.0 |]

let test_suite_quaternion_lookat = 
    "lookat" >::: [
        ("aeronautic_null" >::
           fun ctxt ->
           test_lookat_aeronautic z3 x3
        ) ;
        ("aeronautic_null" >::
           fun ctxt ->
           let zn = Vector.(scale (-. 1.0) (copy z3))  in
           test_lookat_aeronautic zn y3
        ) ;
        ("graphics_null" >::
           fun ctxt ->
           let zn = Vector.(scale (-. 1.0) (copy z3)) in
           test_lookat_graphics zn y3 ;
           ()
        ) ;
        ("graphics_1" >::
           fun ctxt ->
           let zn = Vector.(scale (-. 1.0) (copy z3)) in
           test_lookat_graphics zn x3 ;
           ()
        ) ;
    ]
(*b Quaternion test suite - combine individual suites *)
(* to do 
of rotation
lookat (direction, up)
to rotation (vector, sin, cos)

Vector-to-vector rotation as quaternion
 *)
let test_suite_quaternion =
  "Test quaternions" >:::
    [
      test_suite_quaternion_create ;
      test_suite_quaternion_operation ;
      test_suite_quaternion_lookat ;
      test_suite_quaternion_modulus ;
    ]

(*a All test suites, toplevel *)
let test_suites = "All tests" >::: [
     test_suite_quaternion ;
    ]


let _ = 
    at_exit Gc.full_major ;
    run_test_tt_main  test_suites ;
