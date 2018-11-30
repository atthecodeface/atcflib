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
let zero3x3 = Matrix.(scale 0. (make 3 3))
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

(*f assert_matrix_rc : matrixfn -> n -> float array -> unit
 *
 * Assert that matrixfn n are close enough to the float array
 * 
 *)
let assert_matrix_rc matrixfn n cs =
  let tmp = Vector.make (Array.length cs) in
  let tmp = matrixfn n tmp in
  assert_coords tmp cs

(*f assert_matrix_rc : int -> matrixfn -> n -> matrixfn -> n -> unit
 *
 * Assert that matrixfn n are close enough to the float array
 * 
 *)
let assert_matrix_rc_rc sz mfn0 n0 mfn1 n1 =
  let tmp0 = mfn0 n0 (Vector.make sz) in
  let tmp1 = mfn1 n1 (Vector.make sz) in
  assert_vector tmp0 tmp1

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
  let match_m_row r _ = assert_matrix_rc_rc (Matrix.ncols m2) (Matrix.row_vector m1) r (Matrix.row_vector m2) r in
  fori 0 (Matrix.nrows m1) match_m_row ()

let test_transpose ctxt m = 
  let mt = Matrix.transpose (Matrix.copy m) in
  assert_equal_int "nrows" (Matrix.nrows m) (Matrix.ncols mt) ;
  assert_equal_int "nrows" (Matrix.nrows mt) (Matrix.ncols m) ;
  let tmp1 = Vector.make (Matrix.ncols m) in
  let tmp2 = Vector.make (Matrix.ncols m) in
  let match_m_col c _ = assert_coords (Matrix.col_vector m c tmp1) (Vector.coords (Matrix.row_vector mt c tmp2)) in
  let match_m_row r _ = assert_coords (Matrix.row_vector m r tmp1) (Vector.coords (Matrix.col_vector mt r tmp2)) in
  fori 0 (Matrix.nrows m) match_m_row () ;
  fori 0 (Matrix.ncols m) match_m_col ()
    
let test_invert ctxt m = 
  let mt = Matrix.(lup_invert (copy m)) in
  let i  = Matrix.(identity   (copy m)) in
  let z  = Matrix.(scale 0.0 (copy m)) in
  let r0 = Matrix.(add_scaled (-. 1.0) i (assign_m_m m mt (copy mt))) in
  let r1 = Matrix.(add_scaled (-. 1.0) i (assign_m_m mt m (copy mt))) in
  assert_matrices r0 z ;
  assert_matrices r1 z
    
let test_suite_matrix_assign = 
    "assign" >::: [
        ("2x2a" >::
           fun ctxt ->
           let m = Matrix.make 2 2 in
           ignore Matrix.(set 0 1 2.0 (set 0 0 3.0 m) ) ;
           ignore Matrix.(set 1 1 0.0 (set 1 0 0.0 m) ) ;
           assert_matrix_rc (Matrix.row_vector m) 0 [|3.0; 2.0|];
           assert_matrix_rc (Matrix.row_vector m) 1 [|0.0; 0.0|];
           assert_matrix_rc (Matrix.col_vector m) 0 [|3.0; 0.0|];
           assert_matrix_rc (Matrix.col_vector m) 1 [|2.0; 0.0|];
        ) ;
       ("2x2b" >::
           fun ctxt ->
           let m = Matrix.make 2 2 in
           ignore Matrix.(set 0 1 2.0 (set 0 0 1.0 m)) ;
           ignore Matrix.(set 1 1 4.0 (set 1 0 3.0 m)) ;
           assert_equal_int "nrows" (Matrix.nrows m) 2 ;
           assert_equal_int "ncols" (Matrix.ncols m) 2 ;
           assert_matrix_rc (Matrix.row_vector m) 0 [|1.0; 2.0|];
           assert_matrix_rc (Matrix.row_vector m) 1 [|3.0; 4.0|];
           assert_matrix_rc (Matrix.col_vector m) 0 [|1.0; 3.0|];
           assert_matrix_rc (Matrix.col_vector m) 1 [|2.0; 4.0|];
        ) ;
        ("transpose_2x2" >::
           fun ctxt ->
           let m = Matrix.make 2 2 in
           ignore Matrix.(set 0 1 2.0 (set 0 0 1.0 m)) ;
           ignore Matrix.(set 1 1 4.0 (set 1 0 3.0 m)) ;
           test_transpose ctxt m
        ) ;
        ("transpose_4x2" >::
           fun ctxt ->
           let m = Matrix.make 4 2 in
           ignore Matrix.(set 0 1 2.0 (set 0 0 1.0 m)) ;
           ignore Matrix.(set 1 1 4.0 (set 1 0 3.0 m)) ;
           test_transpose ctxt m
        ) ;
        ("transpose_1x2" >::
           fun ctxt ->
           let m = Matrix.make 1 2 in
           ignore Matrix.(set 0 1 2.0 (set 0 0 1.0 m)) ;
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
           assert_matrices m zero3x3;
           assert_matrix_rc (Matrix.row_vector m) 0 [|0.;0.;0.;|];
           assert_matrix_rc (Matrix.row_vector m) 1 [|0.;0.;0.;|];
           assert_matrix_rc (Matrix.row_vector m) 2 [|0.;0.;0.;|];
           assert_matrix_rc (Matrix.col_vector m) 0 [|0.;0.;0.;|];
           assert_matrix_rc (Matrix.col_vector m) 1 [|0.;0.;0.;|];
           assert_matrix_rc (Matrix.col_vector m) 2 [|0.;0.;0.;|];
        ) ;
        ("2x1" >::
           fun ctxt ->
           let m = Matrix.make 2 1 in
           ignore Matrix.(set 0 1 2.0 (set 0 0 1.0 m)) ;
           assert_matrix_rc (Matrix.row_vector m) 0 [|1.0; |] ;
           assert_matrix_rc (Matrix.row_vector m) 1 [|2.0; |] ;
           assert_matrix_rc (Matrix.col_vector m) 0 [|1.0; 2.0|]
        ) ;
        ("1x2" >::
           fun ctxt ->
           let m = Matrix.make 1 2 in
           ignore Matrix.(set  0 1 2.0 (set 0 0 1.0 m)) ;
           assert_matrix_rc (Matrix.col_vector m) 0 [|1.0; |] ;
           assert_matrix_rc (Matrix.col_vector m) 1 [|2.0; |] ;
           assert_matrix_rc (Matrix.row_vector m) 0 [|1.0; 2.0|]
        ) ;
        ("3x3_identity" >::
           fun ctxt ->
           let m = Matrix.make 3 3 in
           let tmp = Vector.make 3 in
           ignore (Matrix.identity m);
           assert_equal_int "nrows" (Matrix.nrows m) 3 ;
           assert_equal_int "ncols" (Matrix.ncols m) 3 ;
           assert_vector ((Matrix.row_vector m) 0 tmp) x3 ;
           assert_vector ((Matrix.row_vector m) 1 tmp) y3 ;
           assert_vector ((Matrix.row_vector m) 2 tmp) z3 ;
           assert_vector ((Matrix.col_vector m) 0 tmp) x3 ;
           assert_vector ((Matrix.col_vector m) 1 tmp) y3 ;
           assert_vector ((Matrix.col_vector m) 2 tmp) z3
        ) ;
    ]

(*b Matrix operation tests *)
let test_suite_matrix_operation = 
    "operation" >::: [
        ("scale2x2" >::
           fun ctxt ->
           let m = Matrix.make 2 2 in
           ignore Matrix.(scale 2.0 (identity m)) ;
           assert_matrix_rc (Matrix.row_vector m) 0 [|2.0; 0.0|];
           assert_matrix_rc (Matrix.row_vector m) 1 [|0.0; 2.0|];
        ) ;
        ("add3x3" >::
           fun ctxt ->
           let m = Matrix.(scale 0.50 (add_scaled 1.0 rot30_z (copy rot30_z))) in
           assert_matrices m rot30_z;
        ) ;
        ("add3x3" >::
           fun ctxt ->
           let m = Matrix.(scale 0.25 (add_scaled 3.0 rot30_z (copy rot30_z))) in
           assert_matrices m rot30_z;
        ) ;
       ("invert3x3" >::
           fun ctxt ->
           let m = Matrix.lup_invert (Matrix.copy rot30_z) in
           assert_matrix_rc (Matrix.row_vector m) 0 [|c30; s30; 0.0|];
           assert_matrix_rc (Matrix.row_vector m) 1 [|(-. s30); c30; 0.0|];
           assert_matrix_rc_rc 3 (Matrix.row_vector m) 2 (Matrix.row_vector rot30_z) 2;
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
           let v = Vector.make 2 in
           let m = Matrix.copy mat_1234 in
           let p = Matrix.lup_decompose m v in
           let l = Matrix.(lup_get_l (copy m)) in
           let u = Matrix.(lup_get_u (copy m)) in
           let r = Matrix.(assign_m_m l u (copy u)) in
           assert_coords p [|1.0; 0.0|];
           assert_matrix_rc (Matrix.row_vector m) 0 [|3.0; 4.0|] ;
           assert_matrix_rc (Matrix.row_vector m) 1 [|(1.0 /. 3.0); (2.0 /. 3.0)|] ;
           assert_matrix_rc (Matrix.row_vector l) 0 [|1.0; 0.0|] ;
           assert_matrix_rc (Matrix.row_vector l) 1 [|(1.0 /. 3.0); 1.0|] ;
           assert_matrix_rc (Matrix.row_vector u) 0 [|3.0; 4.0|] ;
           assert_matrix_rc (Matrix.row_vector u) 1 [|0.0; (2.0 /. 3.0)|] ;
           assert_matrix_rc (Matrix.row_vector r) 0 [|3.0; 4.0|] ;
           assert_matrix_rc (Matrix.row_vector r) 1 [|1.0; 2.0|] ;
        ) ;
        ("decomp3x3" >::
           fun ctxt ->
           let v = Vector.make 3 in
           let m = Matrix.copy rot30_z in
           let p = Matrix.lup_decompose m v in
           assert_coords p [|0.0 ; 1.0 ; 2.0|];
           assert_matrix_rc (Matrix.row_vector m) 0 [|c30; (-.s30); 0.0|];
           assert_matrix_rc (Matrix.row_vector m) 1 [|0.5/.c30; 1.0/.c30; 0.0|];
           assert_matrix_rc (Matrix.row_vector m) 2 [|0.0; 0.0; 1.0|]
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

(*a All test suites, toplevel *)
let test_suites = "All tests" >::: [
     test_suite_matrix ;
    ]


let _ = 
    at_exit Gc.full_major ;
    run_test_tt_main  test_suites ;
