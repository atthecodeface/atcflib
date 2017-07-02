open Atcflib
open OUnit

let str=Printf.sprintf

let epsilon = 0.00000001

let x3 = (vector3 1.0 0.0 0.0)

let y3 = (vector3 0.0 1.0 0.0)

let z3 = (vector3 0.0 0.0 1.0)

let xy3 = (vector3 1.0 1.0 0.0)#normalize
let xz3 = (vector3 1.0 0.0 1.0)#normalize
let yz3 = (vector3 0.0 1.0 1.0)#normalize

let rt2 = sqrt 2.0
let n_rt2 = -. rt2
let rrt2 = 1.0 /. rt2
let n_rrt2 = -. rrt2

let assert_equal_float msg v0 v1 =
  let diff = abs_float ( v0 -. v1 ) in
  let close_enough = (diff < epsilon) in
  assert_bool (str "%s:%f:%f" msg v0 v1) close_enough

let assert_equal_int msg v0 v1 =
  let close_enough = (v0 = v1) in
  assert_bool (str "%s:%d:%d" msg v0 v1) close_enough

let assert_coords v cs =
  let assert_coord = fun a b -> assert_equal_float "Coord" a b in
  Array.iter2 assert_coord v#coords cs


let test_suite_vector =
  "Test vectors" >:::
    [ ("create2" >::
        fun ctxt ->
        let v = vector2 1.0 2.0 in
        assert_coords v [| 1.0; 2.0 |] ;
        assert_equal_int "length" v#length 2
      ) ;
      ("create3" >::
         fun ctxt ->
         let v = vector3 1.0 2.0 3.0 in
         assert_coords v [| 1.0; 2.0; 3.0 |] ;
         assert_equal_int "length" v#length 3
      ) ;
      ("create4" >::
         fun ctxt ->
         let v = vector4 1.0 2.0 3.0 4.0 in
         assert_coords v [| 1.0; 2.0; 3.0 ; 4.0 |] ;
         assert_equal_int "length" v#length 4
      ) ;
      ("add2_0" >::
         fun ctxt ->
         let v1 = vector2 1.0 2.0 in
         let v2 = vector2 3.0 1.0 in
         assert_coords (v1#add v2) [| 4.0; 3.0 |]
      ) ;
      ("add2_1" >::
         fun ctxt ->
         let v1 = vector2 3.0 5.0 in
         let v2 = vector2 1.0 0.0 in
         assert_coords (v1#add v2) [| 4.0; 5.0 |]
      ) ;
      ("add2_scaled_0" >::
         fun ctxt ->
         let v1 = vector2 3.0 5.0 in
         let v2 = vector2 1.0 0.0 in
         assert_coords (v1#add_scaled v2 2.0) [| 5.0; 5.0 |]
      ) ;
      ("add2_scaled_1" >::
         fun ctxt ->
         let v1 = vector2 3.0 5.0 in
         let v2 = vector2 1.0 0.0 in
         assert_coords (v1#add_scaled v1 (-. 1.0)) [| 0.0; 0.0 |]
      ) ;
      ("modulus_0" >::
         fun ctxt ->
         let v1 = vector2 3.0 4.0 in
         assert_equal_float "Modulus" v1#modulus 5.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus 10.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus 20.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus 40.0;
      ) ;
      ("modulus_1" >::
         fun ctxt ->
         let v1 = vector2 3.0 4.0 in
         assert_equal_float "Modulus" v1#modulus_squared 25.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus_squared 100.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus_squared 400.0;
         assert_equal_float "Modulus" (v1#scale 2.0)#modulus_squared 1600.0;
      ) ;
      ("normalize2_0" >::
         fun ctxt ->
         let v1 = vector2 0.0 1.0 in
         (* ignore (v1#normalize) ; *)
         assert_coords (v1#normalize) [| 0.0; 1.0 |]
      ) ;
      ("normalize2_1" >::
         fun ctxt ->
         let v1 = vector2 1.0 0.0 in
         (* ignore (v1#normalize) ; *)
         assert_coords (v1#normalize) [| 1.0; 0.0 |]
      ) ;
      ("normalize2_2" >::
         fun ctxt ->
         let v1 = vector2 3.0 4.0 in
         (* ignore (v1#normalize) ; *)
         assert_coords (v1#normalize) [| 0.6; 0.8 |]
      ) ;
      ("assign_0" >::
         fun ctxt ->
         let v1 = vector2 3.0 4.0 in
         let v2 = vector2 1.0 2.0 in
         assert_coords (v1#assign v2) [| 1.0; 2.0 |];
         assert_coords (v2) [| 1.0; 2.0 |]
      ) ;
      ("assign_1" >::
         fun ctxt ->
         let v1 = vector2 3.0 4.0 in
         let v2 = vector2 1.0 2.0 in
         assert_coords (v2#assign v1) [| 3.0; 4.0 |];
         assert_coords (v1) [| 3.0; 4.0 |]
      ) ;
      ("copy_0" >::
         fun ctxt ->
         let v1 = vector2 3.0 4.0 in
         let v2 = v1#copy in
         assert_coords (v2) [| 3.0; 4.0 |];
         assert_coords (v1#normalize) [| 0.6; 0.8 |];
         assert_coords (v2) [| 3.0; 4.0 |]
      ) ;
      ("scale_0" >::
         fun ctxt ->
         let v1 = vector2 3.0 4.0 in
         assert_coords (v1#scale 2.0) [| 6.0; 8.0 |];
      ) ;
      ("set_0" >::
         fun ctxt ->
         let v1 = vector2 3.0 4.0 in
         assert_coords ((v1#set 0 1.0)#set 1 2.0) [| 1.0; 2.0 |];
      ) ;
      ("dot_product_0" >::
         fun ctxt ->
         let v1 = vector2 3.0 4.0 in
         let v2 = vector2 3.0 4.0 in
         assert_equal_float "Commutative" (v1#dot_product v2) (v2#dot_product v1);
         assert_equal_float "Result"      (v1#dot_product v2) 25.0;
      ) ;
      ("dot_product_1" >::
         fun ctxt ->
         let v1 = vector2 3.0 4.0 in
         let v2 = vector2 4.0 (-. 3.0) in
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
         assert_coords v#repr [|rrt2 ; n_rrt2 ; 0.0|]
      ) ;
    ]


let test_suites       = "All tests" >::: [ test_suite_vector ]
let _ = 
  run_test_tt_main  test_suites
