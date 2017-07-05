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
let ncv = Atcflib.no_class_v
let v  = ncv(v_create 3)
let v2 = (ncv (v_create 3)) #assign v
(*
let ncm = Atcflib.no_class_m
let m  = ncm(m_create 3 3)
let v3 = (v#assign_m_v m v2)
let m2 = m #copy #assign_m_m m m
let v4 = m2 #apply v2
 *)

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
let x3 = (mkvector3 1.0 0.0 0.0)
let y3 = (mkvector3 0.0 1.0 0.0)
let z3 = (mkvector3 0.0 0.0 1.0)
let xy3 = (mkvector3 1.0 1.0 0.0)#normalize
let xz3 = (mkvector3 1.0 0.0 1.0)#normalize
let yz3 = (mkvector3 0.0 1.0 1.0)#normalize
let zero3 = (mkvector3 0.0 0.0 0.0)
let rot30_z = ((((((mkmatrix 3 3)#identity)#set 0 0 c30)#set 0 1 (-. s30))#set 1 0 s30)#set 1 1 c30)
let mat_1234 = ((((((mkmatrix 2 2)#identity)#set 0 0 1.0)#set 0 1 2.0)#set 1 0 3.0)#set 1 1 4.0)

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
    let qrijk = q#get_rijk in
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
  Array.iteri assert_coord v#coords

(*a Vector test suite *)
(*b Vector creation tests *)
let test_suite_vector_create = 
    "create" >::: [
        ("create2" >::
           fun ctxt ->
           let v = mkvector2 1.0 2.0 in
           assert_coords v [| 1.0; 2.0 |] ;
           assert_equal_int "length" v#length 2
        ) ;
        ("create3" >::
           fun ctxt ->
           let v = mkvector3 1.0 2.0 3.0 in
           assert_coords v [| 1.0; 2.0; 3.0 |] ;
           assert_equal_int "length" v#length 3
        ) ;
        ("create4" >::
           fun ctxt ->
           let v = mkvector4 1.0 2.0 3.0 4.0 in
           assert_coords v [| 1.0; 2.0; 3.0 ; 4.0 |] ;
           assert_equal_int "length" v#length 4
        ) ;
      ("copy_0" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 4.0 in
         let v2 = v1#copy in
         assert_coords (v2) [| 3.0; 4.0 |];
         assert_coords (v1#normalize) [| 0.6; 0.8 |];
         assert_coords (v2) [| 3.0; 4.0 |]
      ) ;
    ]

(*b Vector assignment tests *)
let test_suite_vector_assign = 
    "assign" >::: [
      ("assign_0" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 4.0 in
         let v2 = mkvector2 1.0 2.0 in
         assert_coords (v1#assign v2) [| 1.0; 2.0 |];
         assert_coords (v2) [| 1.0; 2.0 |]
      ) ;
      ("assign_1" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 4.0 in
         let v2 = mkvector2 1.0 2.0 in
         assert_coords (v2#assign v1) [| 3.0; 4.0 |];
         assert_coords (v1) [| 3.0; 4.0 |]
      ) ;
      ("set_0" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 4.0 in
         assert_coords ((v1#set 0 1.0)#set 1 2.0) [| 1.0; 2.0 |];
      ) ;
      ("matrix_mult" >::
         fun ctxt ->
         let v1 = ((xy3#copy)#assign_m_v rot30_z x3) in
         let v2 = ((v1#copy)#assign_m_v rot30_z v1) in
         let v3 = ((xy3#copy)#assign_m_v rot30_z v2) in
         assert_coords v3 y3#coords;
      ) ;
    ]

(*b Vector in-place operation tests *)
let test_suite_vector_in_place = 
    "in_place" >::: [
      ("normalize2_0" >::
         fun ctxt ->
         let v1 = mkvector2 0.0 1.0 in
         (* ignore (v1#normalize) ; *)
         assert_coords (v1#normalize) [| 0.0; 1.0 |]
      ) ;
      ("normalize2_1" >::
         fun ctxt ->
         let v1 = mkvector2 1.0 0.0 in
         (* ignore (v1#normalize) ; *)
         assert_coords (v1#normalize) [| 1.0; 0.0 |]
      ) ;
      ("normalize2_2" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 4.0 in
         (* ignore (v1#normalize) ; *)
         assert_coords (v1#normalize) [| 0.6; 0.8 |]
      ) ;
      ("scale_0" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 4.0 in
         assert_coords (v1#scale 2.0) [| 6.0; 8.0 |];
      ) ;
     ("add2_0" >::
         fun ctxt ->
         let v1 = mkvector2 1.0 2.0 in
         let v2 = mkvector2 3.0 1.0 in
         assert_coords (v1#add v2) [| 4.0; 3.0 |]
      ) ;
      ("add2_1" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 5.0 in
         let v2 = mkvector2 1.0 0.0 in
         assert_coords (v1#add v2) [| 4.0; 5.0 |]
      ) ;
      ("add2_scaled_0" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 5.0 in
         let v2 = mkvector2 1.0 0.0 in
         assert_coords (v1#add_scaled v2 2.0) [| 5.0; 5.0 |]
      ) ;
      ("add2_scaled_1" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 5.0 in
         assert_coords (v1#add_scaled v1 (-. 1.0)) [| 0.0; 0.0 |]
      ) ;
    ]

(*b Vector interrogation (no-side-effect) tests *)
let test_suite_vector_interrogation = 
    "interrogation" >::: [
      ("modulus_0" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 4.0 in
         assert_equal_float "Modulus" v1#modulus 5.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus 10.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus 20.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus 40.0;
      ) ;
      ("modulus_1" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 4.0 in
         assert_equal_float "Modulus" v1#modulus_squared 25.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus_squared 100.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus_squared 400.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus_squared 1600.0;
      ) ;
  ]
    
(*b Vector-3 operations *)
let test_suite_vector_vector3 = 
  "vector3" >::: [
      ("dot_product_0" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 4.0 in
         let v2 = mkvector2 3.0 4.0 in
         assert_equal_float "Commutative" (v1#dot_product v2) (v2#dot_product v1);
         assert_equal_float "Result"      (v1#dot_product v2) 25.0;
      ) ;
      ("dot_product_1" >::
         fun ctxt ->
         let v1 = mkvector2 3.0 4.0 in
         let v2 = mkvector2 4.0 (-. 3.0) in
         assert_equal_float "Commutative" (v1#dot_product v2) (v2#dot_product v1);
         assert_equal_float "Result"      (v1#dot_product v2) 0.0;
      ) ;
      ("cross_product3_0" >::
         fun ctxt ->
         assert_coords (x3#cross_product3 y3) z3#coords;
         assert_coords (y3#cross_product3 z3) x3#coords;
         assert_coords (z3#cross_product3 x3) y3#coords;
      ) ;
      ("cross_product3_1" >::
         fun ctxt ->
         assert_coords (x3#cross_product3 ((z3#copy)#scale (-. 1.0))) y3#coords;
         assert_coords (y3#cross_product3 ((x3#copy)#scale (-. 1.0))) z3#coords;
         assert_coords (z3#cross_product3 ((y3#copy)#scale (-. 1.0))) x3#coords;
      ) ;
      ("angle_axis3_0" >::
         fun ctxt ->
         let (v,c,s) = x3#angle_axis_to3 y3 in
         assert_coords v z3#coords;
         assert_equal_float "cos" c 0.0;
         assert_equal_float "sin" s 1.0;
      ) ;
      ("angle_axis3_1" >::
         fun ctxt ->
         let (v,c,s) = x3#angle_axis_to3 z3 in
         assert_coords v ((y3#copy)#scale (-. 1.0))#coords;
         assert_equal_float "cos" c 0.0;
         assert_equal_float "sin" s 1.0;
      ) ;
      ("angle_axis3_2" >::
         fun ctxt ->
         let (v,c,s) = x3#angle_axis_to3 xy3 in
         assert_coords v z3#coords;
         assert_equal_float "cos" c rrt2;
         assert_equal_float "sin" s rrt2;
      ) ;
      ("angle_axis3_3" >::
         fun ctxt ->
         let (v,c,s) = xy3#angle_axis_to3 y3 in
         assert_coords v z3#coords;
         assert_equal_float "cos" c rrt2;
         assert_equal_float "sin" s rrt2;
      ) ;
      ("angle_axis3_4" >::
         fun ctxt ->
         let (v,c,s) = xy3#angle_axis_to3 z3 in
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
           let m = mkmatrix 2 2 in
           assert_equal_int "nrows" m#nrows 2 ;
           assert_equal_int "ncols" m#ncols 2
        ) ;
        ("3x3" >::
           fun ctxt ->
           let m = mkmatrix 3 3 in
           assert_equal_int "nrows" m#nrows 3 ;
           assert_equal_int "ncols" m#ncols 3
        ) ;
        ("4x2" >::
           fun ctxt ->
           let m = mkmatrix 4 2 in
           assert_equal_int "nrows" m#nrows 4 ;
           assert_equal_int "ncols" m#ncols 2
        ) ;
    ]
(*b Matrix assignment tests *)
let assert_matrices m1 m2 =
  let match_m_row r _ = assert_coords (m1#row_vector r) (m2#row_vector r)#coords in
  fori 0 m1#nrows match_m_row ()

let test_transpose ctxt m = 
  let mt = m#copy#transpose in
  assert_equal_int "nrows" m#nrows mt#ncols ;
  assert_equal_int "nrows" mt#nrows m#ncols ;
  let match_m_col c _ = assert_coords (m#col_vector c) (mt#row_vector c)#coords in
  let match_m_row r _ = assert_coords (m#row_vector r) (mt#col_vector r)#coords in
  fori 0 m#nrows match_m_row () ;
  fori 0 m#ncols match_m_col ()
    
let test_invert ctxt m = 
  let mt = m#copy#lup_invert in
  let i = m#copy#identity in
  let z = m#copy#scale 0.0 in
  let r0 = (mt#copy#assign_m_m m mt)#add_scaled i (-. 1.0) in
  let r1 = (mt#copy#assign_m_m mt m)#add_scaled i (-. 1.0) in
  assert_matrices r0 z ;
  assert_matrices r1 z
    
let test_suite_matrix_assign = 
    "assign" >::: [
        ("2x2" >::
           fun ctxt ->
           let m = mkmatrix 2 2 in
           ignore ((m#set 0 0 3.0)#set 0 1 2.0) ;
           assert_equal_int "nrows" m#nrows 2 ;
           assert_equal_int "ncols" m#ncols 2 ;
           assert_coords (m#row_vector 0) [|3.0; 2.0|];
           assert_coords (m#row_vector 1) [|0.0; 0.0|];
           assert_coords (m#col_vector 0) [|3.0; 0.0|];
           assert_coords (m#col_vector 1) [|2.0; 0.0|];
        ) ;
        ("2x2" >::
           fun ctxt ->
           let m = mkmatrix 2 2 in
           ignore ((m#set 0 0 1.0)#set 0 1 2.0) ;
           ignore ((m#set 1 0 3.0)#set 1 1 4.0) ;
           assert_equal_int "nrows" m#nrows 2 ;
           assert_equal_int "ncols" m#ncols 2 ;
           assert_coords (m#row_vector 0) [|1.0; 2.0|];
           assert_coords (m#row_vector 1) [|3.0; 4.0|];
           assert_coords (m#col_vector 0) [|1.0; 3.0|];
           assert_coords (m#col_vector 1) [|2.0; 4.0|];
        ) ;
        ("transpose_2x2" >::
           fun ctxt ->
           let m = mkmatrix 2 2 in
           ignore ((m#set 0 0 1.0)#set 0 1 2.0) ;
           ignore ((m#set 1 0 3.0)#set 1 1 4.0) ;
           test_transpose ctxt m
        ) ;
        ("transpose_4x2" >::
           fun ctxt ->
           let m = mkmatrix 4 2 in
           ignore ((m#set 0 0 1.0)#set 0 1 2.0) ;
           ignore ((m#set 1 0 3.0)#set 1 1 4.0) ;
           test_transpose ctxt m
        ) ;
        ("transpose_1x2" >::
           fun ctxt ->
           let m = mkmatrix 1 2 in
           ignore ((m#set 0 0 1.0)#set 0 1 2.0) ;
           test_transpose ctxt m
        ) ;
    ]
(*b Matrix interrogation tests *)
let test_suite_matrix_interrogation = 
    "interrogate" >::: [
        ("3x3" >::
           fun ctxt ->
           let m = mkmatrix 3 3 in
           assert_equal_int "nrows" m#nrows 3 ;
           assert_equal_int "ncols" m#ncols 3 ;
           assert_coords (m#row_vector 0) zero3#coords ;
           assert_coords (m#row_vector 1) zero3#coords ;
           assert_coords (m#row_vector 2) zero3#coords ;
           assert_coords (m#col_vector 0) zero3#coords ;
           assert_coords (m#col_vector 1) zero3#coords ;
           assert_coords (m#col_vector 2) zero3#coords
        ) ;
        ("2x1" >::
           fun ctxt ->
           let m = mkmatrix 2 1 in
           ignore ((m#set 0 0 1.0)#set 1 0 2.0) ;
           assert_coords (m#row_vector 0) [|1.0; |] ;
           assert_coords (m#row_vector 1) [|2.0; |] ;
           assert_coords (m#col_vector 0) [|1.0; 2.0|]
        ) ;
        ("1x2" >::
           fun ctxt ->
           let m = mkmatrix 1 2 in
           ignore ((m#set 0 0 1.0)#set 0 1 2.0) ;
           assert_coords (m#col_vector 0) [|1.0; |] ;
           assert_coords (m#col_vector 1) [|2.0; |] ;
           assert_coords (m#row_vector 0) [|1.0; 2.0|]
        ) ;
        ("3x3_identity" >::
           fun ctxt ->
           let m = mkmatrix 3 3 in
           ignore m#identity;
           assert_equal_int "nrows" m#nrows 3 ;
           assert_equal_int "ncols" m#ncols 3 ;
           assert_coords (m#row_vector 0) x3#coords ;
           assert_coords (m#row_vector 1) y3#coords ;
           assert_coords (m#row_vector 2) z3#coords ;
           assert_coords (m#col_vector 0) x3#coords ;
           assert_coords (m#col_vector 1) y3#coords ;
           assert_coords (m#col_vector 2) z3#coords
        ) ;
    ]

(*b Matrix operation tests *)
let test_suite_matrix_operation = 
    "operation" >::: [
        ("scale2x2" >::
           fun ctxt ->
           let m = mkmatrix 2 2 in
           ignore ((m#identity)#scale 2.0) ;
           assert_coords (m#row_vector 0) [|2.0; 0.0|];
           assert_coords (m#row_vector 1) [|0.0; 2.0|];
        ) ;
        ("add3x3" >::
           fun ctxt ->
           let m = ((rot30_z#copy)#add_scaled rot30_z 1.0)#scale 0.5 in
           assert_coords (m#row_vector 0) (rot30_z#row_vector 0)#coords;
           assert_coords (m#row_vector 1) (rot30_z#row_vector 1)#coords;
           assert_coords (m#row_vector 2) (rot30_z#row_vector 2)#coords;
        ) ;
        ("add3x3" >::
           fun ctxt ->
           let m = ((rot30_z#copy)#add_scaled rot30_z 3.0)#scale 0.25 in
           assert_coords (m#row_vector 0) (rot30_z#row_vector 0)#coords;
           assert_coords (m#row_vector 1) (rot30_z#row_vector 1)#coords;
           assert_coords (m#row_vector 2) (rot30_z#row_vector 2)#coords;
        ) ;
        ("invert3x3" >::
           fun ctxt ->
           let m = ((rot30_z#copy)#lup_invert) in
           assert_coords (m#row_vector 0) [|c30; s30; 0.0|];
           assert_coords (m#row_vector 1) [|(-. s30); c30; 0.0|];
           assert_coords (m#row_vector 2) (rot30_z#row_vector 2)#coords;
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
           let m = mat_1234#copy in
           let p = m#lup_decompose in
           let l = m#copy#lup_get_l in
           let u = m#copy#lup_get_u in
           let r = (u#copy)#assign_m_m l u in
           assert_coords p [|1.0; 0.0|];
           assert_coords (m#row_vector 0) [|3.0; 4.0|] ;
           assert_coords (m#row_vector 1) [|(1.0 /. 3.0); (2.0 /. 3.0)|] ;
           assert_coords (l#row_vector 0) [|1.0; 0.0|] ;
           assert_coords (l#row_vector 1) [|(1.0 /. 3.0); 1.0|] ;
           assert_coords (u#row_vector 0) [|3.0; 4.0|] ;
           assert_coords (u#row_vector 1) [|0.0; (2.0 /. 3.0)|] ;
           assert_coords (r#row_vector 0) [|3.0; 4.0|] ;
           assert_coords (r#row_vector 1) [|1.0; 2.0|] ;
        ) ;
        ("decomp3x3" >::
           fun ctxt ->
           let m = (rot30_z#copy) in
           let p = (m#lup_decompose) in
           assert_coords p [|0.0 ; 1.0 ; 2.0|];
           assert_coords (m#row_vector 0) [|c30; (-.s30); 0.0|];
           assert_coords (m#row_vector 1) [|0.5/.c30; 1.0/.c30; 0.0|];
           assert_coords (m#row_vector 2) [|0.0; 0.0; 1.0|]
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
           let q = mkquaternion in
           assert_equal_quat "zero" q [|0.0; 0.0; 0.0; 0.0|]
        ) ;
        ("1234" >::
           fun ctxt ->
           let q = mkquaternion_rijk 1.0 2.0 3.0 4.0 in
           assert_equal_quat "1234" q [|1.0; 2.0; 3.0; 4.0|]
        ) ;
        ("1234r" >::
           fun ctxt ->
           let q = (mkquaternion_rijk 1.0 2.0 3.0 4.0)#reciprocal in
           let m = q#modulus_squared in
            Printf.printf "Modulus %f\n" m ;
           assert_equal_quat "1234r" q [|1.0*.m; (-. 2.0)*.m; (-. 3.0)*.m; (-. 4.0)*.m|]
        ) ;
        ("1234c" >::
           fun ctxt ->
           let q = (mkquaternion_rijk 1.0 2.0 3.0 4.0)#conjugate in
           assert_equal_quat "1234c" q [|1.0; -.2.0; -.3.0; -.4.0;|]
        ) ;
        ("1234c2" >::
           fun ctxt ->
           let q =  (mkquaternion_rijk 1.0 2.0 3.0 4.0)#conjugate in
           let q2 = mkquaternion in
           assert_equal_quat "1234c" q2 [|0.0; 0.0; 0.0; 0.0|] ;
           q2#assign q ;
           assert_equal_quat "1234c" q2 [|1.0; -.2.0; -.3.0; -.4.0;|] 
        ) ;
        ("rotation" >::
           fun ctxt ->
           let q =  mkquaternion in
           let q2 = mkquaternion in
           let v = x3#copy in
           let (c,s) = v#assign_q_as_rotation (q#repr#assign_lookat z3 y3) in
           Printf.printf "\nQ" ;
           q#repr ;
           let (c,s) = v#assign_q_as_rotation (q#repr#assign_lookat z3 x3)#repr in
           Printf.printf "\nQ of X";
           ((mkvector3 1.0 0.0 0.0) #apply_q q)#repr ;
           Printf.printf "\nQ of Y" ;
           ((mkvector3 0.0 1.0 0.0) #apply_q q)#repr ;
           Printf.printf "\nQ of Z" ;
           ((mkvector3 0.0 0.0 1.0) #apply_q q)#repr ;
           Printf.printf "\nV" ;
           v#repr ;
           Printf.printf "\nCos/sin %f %f\n" c s
        ) ;
    ]
(*b Quaternion modulus tests *)
let test_suite_quaternion_modulus = 
    "modulus" >::: [
        ("test0" >::
           fun ctxt -> let q = mkquaternion_rijk 1.0 0.0 0.0 0.0 in
           let qc = q #copy #conjugate in
           let qcq = q #copy #postmultiply qc in
           let r = qcq #get_rijk .(0) in
           assert_equal_float "m=r" q#modulus_squared r ;
           assert_equal_float "m"   q#modulus 1.0 ;
           assert_equal_float "msq" q#modulus_squared (q#modulus ** 2.)
        ) ;
        ("test1" >::
           fun ctxt -> let q = mkquaternion_rijk 0.0 1.0 0.0 0.0 in
           assert_equal_float "m"   q#modulus 1.0 ;
           assert_equal_float "msq" q#modulus_squared (q#modulus ** 2.)
        ) ;
        ("test2" >::
           fun ctxt -> let q = mkquaternion_rijk 0.0 0.0 1.0 0.0 in
           assert_equal_float "m"   q#modulus 1.0 ;
           assert_equal_float "msq" q#modulus_squared (q#modulus ** 2.)
        ) ;
        ("test3" >::
           fun ctxt -> let q = mkquaternion_rijk 0.0 0.0 0.0 1.0 in
           assert_equal_float "m"   q#modulus 1.0 ;
           assert_equal_float "msq" q#modulus_squared (q#modulus ** 2.)
        ) ;
        ("test0" >::
           fun ctxt -> let q = mkquaternion_rijk 2.0 0.0 0.0 0.0 in
           let qc = q#copy#conjugate in
           let qcq = q #copy #postmultiply qc in
           let r = qcq #get_rijk .(0) in
           assert_equal_float "m=r" q#modulus_squared r ;
           assert_equal_float "m"   q#modulus 2.0 ;
           assert_equal_float "msq" q#modulus_squared (q#modulus ** 2.)
           
        ) ;
        ("test1" >::
           fun ctxt -> let q = mkquaternion_rijk 0.0 2.0 0.0 0.0 in
           assert_equal_float "m"   q#modulus 2.0 ;
           assert_equal_float "msq" q#modulus_squared (q#modulus ** 2.)
        ) ;
        ("test2" >::
           fun ctxt -> let q = mkquaternion_rijk 0.0 0.0 2.0 0.0 in
           assert_equal_float "m"   q#modulus 2.0 ;
           assert_equal_float "msq" q#modulus_squared (q#modulus ** 2.)
        ) ;
        ("test3" >::
           fun ctxt -> let q = mkquaternion_rijk 0.0 0.0 0.0 2.0 in
           assert_equal_float "m"   q#modulus 2.0 ;
           assert_equal_float "msq" q#modulus_squared (q#modulus ** 2.)
        ) ;
        ("test4" >::
           fun ctxt -> let q = mkquaternion_rijk 3.0 4.0 12.0 84.0 in
           assert_equal_float "m"   q#modulus 85.0 ;
           assert_equal_float "msq" q#modulus_squared (q#modulus ** 2.)
        ) ;
    ]
(*b Quaternion operation tests *)
let test_suite_quaternion_operation = 
    "operation" >::: [
        ("add_scaled" >::
           fun ctxt ->
           let q1 = mkquaternion_rijk 1.0 2.0 3.0 4.0 in
           let q2 = mkquaternion_rijk 4.0 3.0 2.0 1.0 in
           assert_equal_quat "q2+1*q1" (q2 #add_scaled q1 2.0) [|6.0; 7.0; 8.0; 9.0|] ;
           assert_equal_quat "zero" (q1 #add_scaled q1 (-.1.0)) [|0.0; 0.0; 0.0; 0.0|] ;
           assert_equal_quat "q2+1*q1" (q2 #add_scaled q1 2.0) [|6.0; 7.0; 8.0; 9.0|]
        ) ;
        ("multiply" >::
           fun ctxt ->
           let q = mkquaternion in
           let q1 = mkquaternion_rijk 1.0 2.0 3.0 4.0 in
           let q2 = mkquaternion_rijk 4.0 3.0 2.0 1.0 in
           assert_equal_quat "q2*q1" (q2#copy #premultiply q1) [|-.12.0; 6.0; 24.0; 12.0|] ;
           assert_equal_quat "q2*q1" (q2#copy #premultiply q1) (q1#copy #postmultiply q2)#get_rijk ;
           assert_equal_quat "q2*q1" (q1#copy #premultiply q2) (q2#copy #postmultiply q1)#get_rijk ;
           assert_equal_quat "q1*q2" (q1#copy #premultiply q2) [|-.12.0; 16.0; 4.0; 22.0|] ;
           assert_equal_quat "q1*q2" (q#copy #assign_q_q q1 q2) (q1#copy #postmultiply q2)#get_rijk ;
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
      test_suite_quaternion_modulus ;
    ]

(*a All test suites, toplevel *)
let test_suites = "All tests" >::: [
     test_suite_vector ;
     test_suite_matrix ;
     test_suite_quaternion ;
    ]


let _ = 
    at_exit Gc.full_major ;
    run_test_tt_main  test_suites ;
