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
    let m = Matrix.identity (Matrix.make 3 3) in
    Matrix.set m 0 0 c30 ;
    Matrix.set m 0 1 (-. s30) ;
    Matrix.set m 1 0 s30 ;
    Matrix.set m 1 1 c30

let mat_1234 = 
    let m = Matrix.identity (Matrix.make 2 2) in
    Matrix.set m 0 0 1.0 ;
    Matrix.set m 0 1 2.0 ;
    Matrix.set m 1 0 3.0 ;
    Matrix.set m 1 1 4.0

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
           let v = (Vector.make 2) in
           (Vector.set v ~n:0 ~f:1.0) |>
           Vector.set ~n:1 ~f:2.0 ;
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
         assert_coords (Vector.assign v1 v2) [| 1.0; 2.0 |];
         assert_coords (v2) [| 1.0; 2.0 |]
      ) ;
      ("assign_1" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         let v2 = Vector.make2 1.0 2.0 in
         assert_coords (Vector.assign v2 v1) [| 3.0; 4.0 |];
         assert_coords (v1) [| 3.0; 4.0 |]
      ) ;
      ("set_0" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         assert_coords (Vector.set (Vector.set v1 0 1.0) 1 2.0) [| 1.0; 2.0 |];
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
         assert_coords (Vector.scale v1 2.0) [| 6.0; 8.0 |];
      ) ;
     ("add2_0" >::
         fun ctxt ->
         let v1 = Vector.make2 1.0 2.0 in
         let v2 = Vector.make2 3.0 1.0 in
         assert_coords (Vector.add v1 v2) [| 4.0; 3.0 |]
      ) ;
      ("add2_1" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 5.0 in
         let v2 = Vector.make2 1.0 0.0 in
         assert_coords (Vector.add v1 v2) [| 4.0; 5.0 |]
      ) ;
      ("add2_scaled_0" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 5.0 in
         let v2 = Vector.make2 1.0 0.0 in
         assert_coords (Vector.add_scaled v1 v2 2.0) [| 5.0; 5.0 |]
      ) ;
      ("add2_scaled_1" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 5.0 in
         assert_coords (Vector.add_scaled v1 v1 (-. 1.0)) [| 0.0; 0.0 |]
      ) ;
    ]

(*b Vector interrogation (no-side-effect) tests *)
let test_suite_vector_interrogation = 
    "interrogation" >::: [
      ("modulus_0" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         assert_equal_float "Modulus" (Vector.modulus v1) 5.0;
         assert_equal_float "Modulus" (Vector.modulus (Vector.scale v1 2.0)) 10.0;
         assert_equal_float "Modulus" (Vector.modulus (Vector.scale v1 2.0)) 20.0;
         assert_equal_float "Modulus" (Vector.modulus (Vector.scale v1 2.0)) 40.0;
      ) ;
      ("modulus_1" >::
         fun ctxt ->
         let v1 = Vector.make2 3.0 4.0 in
         assert_equal_float "Modulus" (Vector.modulus_squared v1) 25.0;
         assert_equal_float "Modulus" (Vector.modulus_squared (Vector.scale v1 2.0)) 100.0;
         assert_equal_float "Modulus" (Vector.modulus_squared (Vector.scale v1 2.0)) 400.0;
         assert_equal_float "Modulus" (Vector.modulus_squared (Vector.scale v1 2.0)) 1600.0;
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
      ("cross_product3_0" >::
         fun ctxt ->
         assert_vector (Vector.cross_product3 x3 y3) z3 ;
         assert_vector (Vector.cross_product3 y3 z3) x3 ;
         assert_vector (Vector.cross_product3 z3 x3) y3 ;
      ) ;
      ("cross_product3_1" >::
         fun ctxt ->
         assert_vector (Vector.cross_product3 x3 (Vector.scale (Vector.copy z3) (-. 1.0))) y3 ;
         assert_vector (Vector.cross_product3 y3 (Vector.scale (Vector.copy x3) (-. 1.0))) z3 ;
         assert_vector (Vector.cross_product3 z3 (Vector.scale (Vector.copy y3) (-. 1.0))) x3 ;
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
         assert_vector v (Vector.scale (Vector.copy y3) (-. 1.0)) ;
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
    ]

(*a Matrix test suite *)
(*b Matrix creation tests *)
let test_suite_matrix_create = 
    "create" >::: [
        ("2x2" >::
           fun ctxt ->
           let m = Matrix.make 2 2 in
           assert_equal_int "nrows" (Matrix.nrows m) 2 ;
           assert_equal_int "ncols" (Matrix.ncols m) 2
        ) ;
        ("3x3" >::
           fun ctxt ->
           let m = Matrix.make 3 3 in
           assert_equal_int "nrows" (Matrix.nrows m) 3 ;
           assert_equal_int "ncols" (Matrix.ncols m) 3
        ) ;
        ("4x2" >::
           fun ctxt ->
           let m = Matrix.make 4 2 in
           assert_equal_int "nrows" (Matrix.nrows m) 4 ;
           assert_equal_int "ncols" (Matrix.ncols m) 2
        ) ;
    ]
(*b Matrix assignment tests *)
let assert_matrices m1 m2 =
  let match_m_row r _ = assert_coords ((Matrix.row_vector m1) r) (Vector.coords ((Matrix.row_vector m2) r)) in
  fori 0 (Matrix.nrows m1) match_m_row ()

let test_transpose ctxt m = 
  let mt = Matrix.transpose (Matrix.copy m) in
  assert_equal_int "nrows" (Matrix.nrows m) (Matrix.ncols mt) ;
  assert_equal_int "nrows" (Matrix.nrows mt) (Matrix.ncols m) ;
  let match_m_col c _ = assert_coords ((Matrix.col_vector m) c) (Vector.coords ((Matrix.row_vector mt) c)) in
  let match_m_row r _ = assert_coords ((Matrix.row_vector m) r) (Vector.coords ((Matrix.col_vector mt) r)) in
  fori 0 (Matrix.nrows m) match_m_row () ;
  fori 0 (Matrix.ncols m) match_m_col ()
    
let test_invert ctxt m = 
  let mt = Matrix.lup_invert (Matrix.copy m) in
  let i  = Matrix.identity   (Matrix.copy m) in
  let z  = Matrix.scale (Matrix.copy m) 0.0 in
  let r0 = Matrix.add_scaled (Matrix.assign_m_m (Matrix.copy mt) m mt) i (-. 1.0) in
  let r1 = Matrix.add_scaled (Matrix.assign_m_m (Matrix.copy mt) mt m) i (-. 1.0) in
  assert_matrices r0 z ;
  assert_matrices r1 z
    
let test_suite_matrix_assign = 
    "assign" >::: [
        ("2x2" >::
           fun ctxt ->
           let m = Matrix.make 2 2 in
           ignore (Matrix.set (Matrix.set m 0 0 3.0) 0 1 2.0) ;
           assert_equal_int "nrows" (Matrix.nrows m) 2 ;
           assert_equal_int "ncols" (Matrix.ncols m) 2 ;
           assert_coords ((Matrix.row_vector m) 0) [|3.0; 2.0|];
           assert_coords ((Matrix.row_vector m) 1) [|0.0; 0.0|];
           assert_coords ((Matrix.col_vector m) 0) [|3.0; 0.0|];
           assert_coords ((Matrix.col_vector m) 1) [|2.0; 0.0|];
        ) ;
        ("2x2" >::
           fun ctxt ->
           let m = Matrix.make 2 2 in
           ignore (Matrix.set (Matrix.set m 0 0 1.0) 0 1 2.0) ;
           ignore (Matrix.set (Matrix.set m 1 0 3.0) 1 1 4.0) ;
           assert_equal_int "nrows" (Matrix.nrows m) 2 ;
           assert_equal_int "ncols" (Matrix.ncols m) 2 ;
           assert_coords ((Matrix.row_vector m) 0) [|1.0; 2.0|];
           assert_coords ((Matrix.row_vector m) 1) [|3.0; 4.0|];
           assert_coords ((Matrix.col_vector m) 0) [|1.0; 3.0|];
           assert_coords ((Matrix.col_vector m) 1) [|2.0; 4.0|];
        ) ;
        ("transpose_2x2" >::
           fun ctxt ->
           let m = Matrix.make 2 2 in
           ignore (Matrix.set (Matrix.set m 0 0 1.0) 0 1 2.0) ;
           ignore (Matrix.set (Matrix.set m 1 0 3.0) 1 1 4.0) ;
           test_transpose ctxt m
        ) ;
        ("transpose_4x2" >::
           fun ctxt ->
           let m = Matrix.make 4 2 in
           ignore (Matrix.set (Matrix.set m 0 0 1.0) 0 1 2.0) ;
           ignore (Matrix.set (Matrix.set m 1 0 3.0) 1 1 4.0) ;
           test_transpose ctxt m
        ) ;
        ("transpose_1x2" >::
           fun ctxt ->
           let m = Matrix.make 1 2 in
           ignore (Matrix.set (Matrix.set m 0 0 1.0) 0 1 2.0) ;
           test_transpose ctxt m
        ) ;
    ]
(*b Matrix interrogation tests *)
let test_suite_matrix_interrogation = 
    "interrogate" >::: [
        ("3x3" >::
           fun ctxt ->
           let m = Matrix.make 3 3 in
           assert_equal_int "nrows" (Matrix.nrows m) 3 ;
           assert_equal_int "ncols" (Matrix.ncols m) 3 ;
           assert_vector ((Matrix.row_vector m) 0) zero3 ;
           assert_vector ((Matrix.row_vector m) 1) zero3 ;
           assert_vector ((Matrix.row_vector m) 2) zero3 ;
           assert_vector ((Matrix.col_vector m) 0) zero3 ;
           assert_vector ((Matrix.col_vector m) 1) zero3 ;
           assert_vector ((Matrix.col_vector m) 2) zero3
        ) ;
        ("2x1" >::
           fun ctxt ->
           let m = Matrix.make 2 1 in
           ignore (Matrix.set (Matrix.set m 0 0 1.0) 0 1 2.0) ;
           assert_coords ((Matrix.row_vector m) 0) [|1.0; |] ;
           assert_coords ((Matrix.row_vector m) 1) [|2.0; |] ;
           assert_coords ((Matrix.col_vector m) 0) [|1.0; 2.0|]
        ) ;
        ("1x2" >::
           fun ctxt ->
           let m = Matrix.make 1 2 in
           ignore (Matrix.set (Matrix.set m 0 0 1.0) 0 1 2.0) ;
           assert_coords ((Matrix.col_vector m) 0) [|1.0; |] ;
           assert_coords ((Matrix.col_vector m) 1) [|2.0; |] ;
           assert_coords ((Matrix.row_vector m) 0) [|1.0; 2.0|]
        ) ;
        ("3x3_identity" >::
           fun ctxt ->
           let m = Matrix.make 3 3 in
           ignore (Matrix.identity m);
           assert_equal_int "nrows" (Matrix.nrows m) 3 ;
           assert_equal_int "ncols" (Matrix.ncols m) 3 ;
           assert_vector ((Matrix.row_vector m) 0) x3 ;
           assert_vector ((Matrix.row_vector m) 1) y3 ;
           assert_vector ((Matrix.row_vector m) 2) z3 ;
           assert_vector ((Matrix.col_vector m) 0) x3 ;
           assert_vector ((Matrix.col_vector m) 1) y3 ;
           assert_vector ((Matrix.col_vector m) 2) z3
        ) ;
    ]

(*b Matrix operation tests *)
let test_suite_matrix_operation = 
    "operation" >::: [
        ("scale2x2" >::
           fun ctxt ->
           let m = Matrix.make 2 2 in
           ignore (Matrix.scale (Matrix.identity m) 2.0) ;
           assert_coords ((Matrix.row_vector m) 0) [|2.0; 0.0|];
           assert_coords ((Matrix.row_vector m) 1) [|0.0; 2.0|];
        ) ;
        ("add3x3" >::
           fun ctxt ->
           let m = Matrix.scale (Matrix.add_scaled (Matrix.copy rot30_z) rot30_z 1.0) 0.50 in
           assert_coords (Matrix.row_vector m 0) (Vector.coords (Matrix.row_vector rot30_z 0));
           assert_coords (Matrix.row_vector m 1) (Vector.coords (Matrix.row_vector rot30_z 1));
           assert_coords (Matrix.row_vector m 2) (Vector.coords (Matrix.row_vector rot30_z 2));
        ) ;
        ("add3x3" >::
           fun ctxt ->
           let m = Matrix.scale (Matrix.add_scaled (Matrix.copy rot30_z) rot30_z 3.0) 0.25 in
           assert_coords (Matrix.row_vector m 0) (Vector.coords (Matrix.row_vector rot30_z 0));
           assert_coords (Matrix.row_vector m 1) (Vector.coords (Matrix.row_vector rot30_z 1));
           assert_coords (Matrix.row_vector m 2) (Vector.coords (Matrix.row_vector rot30_z 2));
        ) ;
       ("invert3x3" >::
           fun ctxt ->
           let m = Matrix.lup_invert (Matrix.copy rot30_z) in
           assert_coords (Matrix.row_vector m 0) [|c30; s30; 0.0|];
           assert_coords (Matrix.row_vector m 1) [|(-. s30); c30; 0.0|];
           assert_coords (Matrix.row_vector m 2) (Vector.coords (Matrix.row_vector rot30_z 2));
        ) ;
        ("invert2x2" >::
           fun ctxt ->
           let m = mat_1234 in
           test_invert ctxt m
        ) ;
        ("invert3x3" >::
           fun ctxt ->
           let m = rot30_z in
           test_invert ctxt m
        ) ;

    ]
(*b Matrix LUP tests *)
let test_suite_matrix_lup = 
    "lup" >::: [
        ("lup2x2" >::
           fun ctxt ->
           let m = Matrix.copy mat_1234 in
           let p = Matrix.lup_decompose m in
           let l = Matrix.lup_get_l (Matrix.copy m) in
           let u = Matrix.lup_get_u (Matrix.copy m)  in
           let r = Matrix.assign_m_m (Matrix.copy u) l u in
           assert_coords p [|1.0; 0.0|];
           assert_coords (Matrix.row_vector m 0) [|3.0; 4.0|] ;
           assert_coords (Matrix.row_vector m 1) [|(1.0 /. 3.0); (2.0 /. 3.0)|] ;
           assert_coords (Matrix.row_vector l 0) [|1.0; 0.0|] ;
           assert_coords (Matrix.row_vector l 1) [|(1.0 /. 3.0); 1.0|] ;
           assert_coords (Matrix.row_vector u 0) [|3.0; 4.0|] ;
           assert_coords (Matrix.row_vector u 1) [|0.0; (2.0 /. 3.0)|] ;
           assert_coords (Matrix.row_vector r 0) [|3.0; 4.0|] ;
           assert_coords (Matrix.row_vector r 1) [|1.0; 2.0|] ;
        ) ;
        ("decomp3x3" >::
           fun ctxt ->
           let m = Matrix.copy rot30_z in
           let p = Matrix.lup_decompose m in
           assert_coords p [|0.0 ; 1.0 ; 2.0|];
           assert_coords ((Matrix.row_vector m) 0) [|c30; (-.s30); 0.0|];
           assert_coords ((Matrix.row_vector m) 1) [|0.5/.c30; 1.0/.c30; 0.0|];
           assert_coords ((Matrix.row_vector m) 2) [|0.0; 0.0; 1.0|]
        ) ;
    ]
(*b Matrix test suite - combine individual suites *)
(* to do 
    set_row <vector>
    set_col <vector>
 *)
let test_suite_matrix =
  "Test matrices" >:::
    [
      test_suite_matrix_create ;
      test_suite_matrix_assign ;
      test_suite_matrix_interrogation ;
      test_suite_matrix_operation ;
      test_suite_matrix_lup ;
    ]

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
           let q = Quaternion.conjugate(Quaternion.make_rijk 1.0 2.0 3.0 4.0) in
           assert_equal_quat "1234c" q [|1.0; -.2.0; -.3.0; -.4.0;|]
        ) ;
        ("1234c2" >::
           fun ctxt ->
           let q =  Quaternion.conjugate(Quaternion.make_rijk 1.0 2.0 3.0 4.0) in
           let q2 = Quaternion.make () in
           assert_equal_quat "1234c" q2 [|0.0; 0.0; 0.0; 0.0|] ;
           ignore (Quaternion.assign q2 q ); (* q2 <= q *)
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
           let (c,s) = Vector.assign_q_as_rotation v (Quaternion.assign_lookat_graphics q z y) in
           Printf.printf "\nQ %s" (Quaternion.repr q) ;
           let (c,s) = Vector.assign_q_as_rotation v (Quaternion.assign_lookat_graphics q z x) in
           Printf.printf "\nQ of X %s" (Vector.repr (Vector.apply_q (Vector.make3 1.0 0.0 0.0) q));
           Printf.printf "\nQ of Y %s" (Vector.repr (Vector.apply_q (Vector.make3 0.0 1.0 0.0) q));
           Printf.printf "\nQ of Z %s" (Vector.repr (Vector.apply_q (Vector.make3 0.0 0.0 1.0) q));
           Printf.printf "\nV %s" (Vector.repr v) ;
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
           assert_equal_quat "q2+1*q1" (Quaternion.add_scaled q2 q1 2.0) [|6.0; 7.0; 8.0; 9.0|] ;
           assert_equal_quat "zero"    (Quaternion.add_scaled q1 q1 (-.1.0)) [|0.0; 0.0; 0.0; 0.0|] ;
           assert_equal_quat "q2+1*q1" (Quaternion.add_scaled q2 q1 2.0) [|6.0; 7.0; 8.0; 9.0|]
        ) ;
        ("multiply" >::
           fun ctxt ->
           let q = Quaternion.make () in
           let q1 = Quaternion.make_rijk 1.0 2.0 3.0 4.0 in
           let q2 = Quaternion.make_rijk 4.0 3.0 2.0 1.0 in
           assert_equal_quat "q2*q1" (Quaternion.premultiply (Quaternion.copy q2) q1) [|-.12.0; 6.0; 24.0; 12.0|] ;
           assert_equal_quat "q2*q1" (Quaternion.premultiply (Quaternion.copy q2) q1) (Quaternion.get_rijk (Quaternion.postmultiply (Quaternion.copy q1) q2));
           assert_equal_quat "q2*q1" (Quaternion.premultiply (Quaternion.copy q1) q2) (Quaternion.get_rijk (Quaternion.postmultiply (Quaternion.copy q2) q1));
           assert_equal_quat "q1*q2" (Quaternion.premultiply (Quaternion.copy q1) q2) [|-.12.0; 16.0; 4.0; 22.0|] ;
           assert_equal_quat "q1*q2" (Quaternion.assign_q_q (Quaternion.copy q) q1 q2) (Quaternion.get_rijk (Quaternion.postmultiply (Quaternion.copy q1) q2));
        ) ;
    ]
(*b Quaternion lookat tests *)
let test_lookat_graphics at up =
  let q = Quaternion.make () in
  ignore (Quaternion.assign_lookat_graphics q at up) ;
  Printf.printf "\n Check lookat maps at correctly to -Z:" ;
  assert_coords (Vector.apply_q (Vector.copy at) q) [| 0.0; 0.0  ; (-. 1.0) |] ;
  Printf.printf "\n Check lookat maps up correctly to +Y:" ;
  assert_coords (Vector.apply_q (Vector.copy up) q) [| 0.0; 1.0 ; 0.0 |]

let test_lookat_aeronautic at up =
  let q = Quaternion.make () in
  ignore (Quaternion.assign_lookat_aeronautic q at up) ;
  Printf.printf "\n Check lookat maps at correctly to +Z:" ;
  assert_coords (Vector.apply_q (Vector.copy at) q) [| 0.0; 0.0 ; 1.0 |] ;
  Printf.printf "\n Check lookat maps up correctly to +X:" ;
  assert_coords (Vector.apply_q (Vector.copy up) q) [| 1.0; 0.0 ; 0.0 |]

let test_suite_quaternion_lookat = 
    "lookat" >::: [
        ("aeronautic_null" >::
           fun ctxt ->
           test_lookat_aeronautic z3 x3
        ) ;
        ("aeronautic_null" >::
           fun ctxt ->
           let zn = Vector.scale (Vector.copy z3) (-. 1.0) in
           test_lookat_aeronautic zn y3
        ) ;
        ("graphics_null" >::
           fun ctxt ->
           let zn = Vector.scale (Vector.copy z3) (-. 1.0) in
           test_lookat_graphics zn y3 ;
           ()
        ) ;
        ("graphics_1" >::
           fun ctxt ->
           let zn = Vector.scale (Vector.copy z3) (-. 1.0) in
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
     test_suite_vector ;
     test_suite_quaternion ;
     test_suite_matrix ;
    ]


let _ = 
    at_exit Gc.full_major ;
    run_test_tt_main  test_suites ;
