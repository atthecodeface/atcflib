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
let b_v0_3_0_1 = (Vector.of_bigarray ~length:3 ~offset:0 ~stride:1 biggie)
let b_v1_3_1_1 = (Vector.of_bigarray ~length:3 ~offset:1 ~stride:1 biggie)

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

(*a Vector test suite *)
(*b Vector creation tests *)
let test_suite_vector_create = 
    "create" >::: [
        ("module" >::
           fun ctxt ->
           let v = Vector.(make 2 |> set 0 1.0 |> set 1 2.0) in
           assert_equal_int "length" (Vector.length v) 2
        ) ;
         ("create2" >::
           fun ctxt ->
           let v = Vector.make2 1.0 2.0 in
           assert_coords v [| 1.0; 2.0 |] ;
           assert_equal_int "length" (Vector.length v) 2
        ) ;
        ("create3" >::
           fun ctxt ->
           let v = Vector.make3 1.0 2.0 3.0 in
           assert_coords v [| 1.0; 2.0; 3.0 |] ;
           assert_equal_int "length" (Vector.length v) 3
        ) ;
        ("create4" >::
           fun ctxt ->
           let v = Vector.make4 1.0 2.0 3.0 4.0 in
           assert_coords v [| 1.0; 2.0; 3.0 ; 4.0 |] ;
           assert_equal_int "length" (Vector.length v) 4
        ) ;
      ("copy_0" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         let v2 = Vector.copy v1 in
         assert_coords (v2) [| 3.0; 4.0 |];
         assert_coords (Vector.normalize v1) [| 0.6; 0.8 |];
         assert_coords (v2) [| 3.0; 4.0 |]
      ) ;
    ]

(*b Vector assignment tests *)
let test_suite_vector_assign = 
    "assign" >::: [
      ("assign_0" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         let v2 = Vector.make2 1.0 2.0 in
         assert_coords (Vector.assign v2 v1) [| 1.0; 2.0 |];
         assert_coords (v2) [| 1.0; 2.0 |]
      ) ;
      ("assign_1" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         let v2 = Vector.make2 1.0 2.0 in
         assert_coords (Vector.assign v1 v2) [| 3.0; 4.0 |];
         assert_coords (v1) [| 3.0; 4.0 |]
      ) ;
      ("set_0" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         assert_coords (Vector.(set 1 2.0 (set 0 1.0 v1))) [| 1.0; 2.0 |];
      ) ;
(*       ("matrix_mult" >::
         fun ctxt ->
         let v1 = (Vector.assign_m_v (Vector.copy xy3) rot30_z x3) in
         let v2 = (Vector.assign_m_v (Vector.copy v1)  rot30_z v1) in
         let v3 = (Vector.assign_m_v (Vector.copy xy3) rot30_z v2) in
         assert_vector v3 y3;
      ) ;*)
    ]

(*b Vector in-place operation tests *)
let test_suite_vector_in_place = 
    "in_place" >::: [
      ("normalize2_0" >::
         fun ctxt ->
         let v1 = Vector.make2 0.0 1.0 in
         (* ignore (v1#normalize) ; *)
         assert_coords (Vector.normalize v1) [| 0.0; 1.0 |]
      ) ;
      ("normalize2_1" >::
         fun ctxt ->
         let v1 = Vector.make2 1.0 0.0 in
         (* ignore (v1#normalize) ; *)
         assert_coords (Vector.normalize v1) [| 1.0; 0.0 |]
      ) ;
      ("normalize2_2" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         (* ignore (v1#normalize) ; *)
         assert_coords (Vector.normalize v1) [| 0.6; 0.8 |]
      ) ;
      ("scale_0" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         assert_coords (Vector.scale 2.0 v1) [| 6.0; 8.0 |];
      ) ;
     ("add2_0" >::
         fun ctxt ->
         let v1 = Vector.make2 1.0 2.0 in
         let v2 = Vector.make2 3.0 1.0 in
         assert_coords (Vector.add v2 v1) [| 4.0; 3.0 |]
      ) ;
      ("add2_1" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 5.0 in
         let v2 = Vector.make2 1.0 0.0 in
         assert_coords (Vector.add v2 v1) [| 4.0; 5.0 |]
      ) ;
      ("add2_scaled_0" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 5.0 in
         let v2 = Vector.make2 1.0 0.0 in
         assert_coords (Vector.add_scaled 2.0 v2 v1) [| 5.0; 5.0 |]
      ) ;
      ("add2_scaled_1" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 5.0 in
         assert_coords (Vector.add_scaled (-. 1.0) v1 v1) [| 0.0; 0.0 |]
      ) ;
    ]

(*b Vector interrogation (no-side-effect) tests *)
let test_suite_vector_interrogation = 
    "interrogation" >::: [
      ("modulus_0" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         assert_equal_float "Modulus" (Vector.modulus v1) 5.0;
         assert_equal_float "Modulus" (Vector.modulus (Vector.scale 2.0 v1)) 10.0;
         assert_equal_float "Modulus" (Vector.modulus (Vector.scale 2.0 v1)) 20.0;
         assert_equal_float "Modulus" (Vector.modulus (Vector.scale 2.0 v1)) 40.0;
      ) ;
      ("modulus_1" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         assert_equal_float "Modulus" (Vector.modulus_squared v1) 25.0;
         assert_equal_float "Modulus" (Vector.modulus_squared (Vector.scale 2.0 v1)) 100.0;
         assert_equal_float "Modulus" (Vector.modulus_squared (Vector.scale 2.0 v1)) 400.0;
         assert_equal_float "Modulus" (Vector.modulus_squared (Vector.scale 2.0 v1)) 1600.0;
      ) ;
  ]
    
(*b Vector-3 operations *)
let test_suite_vector_vector3 = 
  "vector3" >::: [
      ("dot_product_0" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         let v2 = Vector.make2 3.0 4.0 in
         assert_equal_float "Commutative" (Vector.dot_product v1 v2) (Vector.dot_product v2 v1);
         assert_equal_float "Result"      (Vector.dot_product v1 v2) 25.0;
      ) ;
      ("dot_product_1" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         let v2 = Vector.make2 4.0 (-. 3.0) in
         assert_equal_float "Commutative" (Vector.dot_product v1 v2) (Vector.dot_product v2 v1);
         assert_equal_float "Result"      (Vector.dot_product v1 v2) 0.0;
      ) ;
(*      ("cross_product3_0" >::
         fun ctxt ->
         assert_vector (Vector.cross_product3 x3 y3) z3 ;
         assert_vector (Vector.cross_product3 y3 z3) x3 ;
         assert_vector (Vector.cross_product3 z3 x3) y3 ;
      ) ;
      ("cross_product3_1" >::
         fun ctxt ->
         assert_vector Vector.(cross_product3 x3 (scale (-. 1.0) (copy z3))) y3 ;
         assert_vector Vector.(cross_product3 y3 (scale (-. 1.0) (copy x3))) z3 ;
         assert_vector Vector.(cross_product3 z3 (scale (-. 1.0) (copy y3))) x3 ;
      ) ;
      ("angle_axis3_0" >::
         fun ctxt ->
         let (v,c,s) = Vector.angle_axis_to3 x3 y3 in
         assert_vector v z3;
         assert_equal_float "cos" c 0.0;
         assert_equal_float "sin" s 1.0;
      ) ;
      ("angle_axis3_1" >::
         fun ctxt ->
         let (v,c,s) = Vector.angle_axis_to3 x3 z3 in
         assert_vector v Vector.(scale (-. 1.0) (copy y3)) ;
         assert_equal_float "cos" c 0.0;
         assert_equal_float "sin" s 1.0;
      ) ;
      ("angle_axis3_2" >::
         fun ctxt ->
         let (v,c,s) = Vector.angle_axis_to3 x3 xy3 in
         assert_vector v z3 ;
         assert_equal_float "cos" c rrt2;
         assert_equal_float "sin" s rrt2;
      ) ;
      ("angle_axis3_3" >::
         fun ctxt ->
         let (v,c,s) = Vector.angle_axis_to3 xy3 y3 in
         assert_vector v z3;
         assert_equal_float "cos" c rrt2;
         assert_equal_float "sin" s rrt2;
      ) ;
      ("angle_axis3_4" >::
         fun ctxt ->
         let (v,c,s) = Vector.angle_axis_to3 xy3 z3 in
         assert_equal_float "cos" c 0.0;
         assert_equal_float "sin" s 1.0;
         assert_coords v [|rrt2 ; n_rrt2 ; 0.0|]
      ) ;
 *)    ]

(*b Vector bigaray slice tests *)
let test_suite_vector_bigarray_slice = 
    "bigarray" >::: [
      ("slice" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         ignore (Vector.(assign x3 b_v0_3_0_1));
         ignore (Vector.(assign x3 b_v1_3_1_1));
         assert_equal_float "Biggie contains slice vector 0" (Genarray.get biggie [|0;|]) 1.0 ;
         assert_equal_float "Biggie contains slice vector 1" (Genarray.get biggie [|1;|]) 1.0 ;
         assert_equal_float "Biggie contains slice vector 2" (Genarray.get biggie [|2;|]) 0.0 ;
         assert_equal_float "Biggie contains slice vector 3" (Genarray.get biggie [|3;|]) 0.0 ;
         assert_vector b_v1_3_1_1 x3 ;
         ()
      ) ;
  ]
    
(*b Vector test suite - combine individual suites *)
let test_suite_vector =
  "Test vectors" >:::
    [
      test_suite_vector_create ;
      test_suite_vector_assign ;
      test_suite_vector_in_place ;
      test_suite_vector_interrogation ;
      test_suite_vector_vector3;
      test_suite_vector_bigarray_slice;
    ]

(*a All test suites, toplevel *)
let test_suites = "All tests" >::: [
     test_suite_vector ;
    ]


let _ = 
    at_exit Gc.full_major ;
    run_test_tt_main  test_suites ;
